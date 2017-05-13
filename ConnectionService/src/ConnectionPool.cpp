#include "ConnectionPool.h"
#include "ConnectionPoolTimer.h"
#include "DataSource.h"
#include "ReplicaCatalogue.h"
#include "ConnectionServiceConfiguration.h"

#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/RelationalServiceException.h"

#include "CoralBase/MessageStream.h"

#include "CoralCommon/MonitoringEventDescription.h"

#include "CoralKernel/Context.h"

#include <vector>

//-----------------------------------------------------------------------------

coral::ConnectionService::ConnectionPool::ConnectionPool( const ConnectionServiceConfiguration& configuration )
  : m_idleConnections()
  , m_activeConnections()
  , m_connectionServiceConfiguration( configuration )
  , m_replicaCatalogue( new ReplicaCatalogue( configuration ) )
  , m_timerThread(0)
  , m_finalized()
  , m_mutexForPool()
  , m_mutexForThread()
  , m_poolStarted(false)
{
  coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );
  log << coral::Info << "Creating a ConnectionPool" << coral::MessageStream::endmsg;
}

//-----------------------------------------------------------------------------

coral::ConnectionService::ConnectionPool::~ConnectionPool()
{
  coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );
  log << coral::Info << "Deleting the ConnectionPool" << coral::MessageStream::endmsg;
  delete m_replicaCatalogue;
  {
    boost::mutex::scoped_lock lock(m_mutexForThread);
    // Lock the mutex if not locked (this avoids unlocking on unlocked mutex)
    m_finalized.try_lock();
    if ( m_poolStarted ) log << coral::Info << "Stopping the connection pool automatic cleanup thread" << coral::MessageStream::endmsg;
    // Release the lock
    m_finalized.unlock();
  }
  if(m_timerThread)
  {
    m_timerThread->join();
    delete m_timerThread;
    m_timerThread = 0;
  }
  clearConnectionPool();
}

//-----------------------------------------------------------------------------

coral::ConnectionService::SessionHandle
coral::ConnectionService::ConnectionPool::getSessionFromNewConnection(const ConnectionParams& connectionParameter,
                                                                      AccessMode accessMode)
{

  coral::IRelationalService& relationalService = m_connectionServiceConfiguration.relationalService();

  if( m_connectionServiceConfiguration.isMonitoringEnabled() ) m_connectionServiceConfiguration.monitoringService();

  SessionHandle sessionHandle( m_connectionServiceConfiguration.serviceName() );

  coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );
  log << coral::Debug << "Trying to get a new "
      << ( (accessMode==coral::ReadOnly)
           ? std::string("read-only")
           : std::string("update") )
      << " session on \"" << connectionParameter.connectionString() << "\""
      << coral::MessageStream::endmsg;

  try {
    coral::IRelationalDomain& domain = relationalService.domainForConnection( connectionParameter.connectionString() );
    coral::IConnection* connection = domain.newConnection( connectionParameter.serviceName() ) ;
    if( connection ) {

      ConnectionHandle connH( connection,
                              connectionParameter,
                              m_connectionServiceConfiguration );
      if(connH.open()) {
        SessionHandle sessH( m_connectionServiceConfiguration.serviceName(), connH );
        sessH.open( connectionParameter.schemaName(),
                    connectionParameter.userName(),
                    connectionParameter.password(),
                    accessMode );
        sessionHandle = sessH;
      } else {
        m_replicaCatalogue->setConnectionExcluded( connectionParameter.technologyName(),
                                                   connectionParameter.serviceName() );
      }
    } else {
      log << coral::Debug << "Could not get a connection to service \"" << connectionParameter.serviceName() <<
        "\" from domain. "<< coral::MessageStream::endmsg;

      throw coral::ConnectionServiceException("Could not get a connection from domain.",
                                              "ConnectionPool::getSessionFromNewConnection");
    }
  } catch ( const coral::NonExistingDomainException& exc) {
    /// not clear what to do in this case? continue?
    log << coral::Warning << "Connection to \"" << connectionParameter.serviceName() << "\" cannot be established: "<< exc.what()<< coral::MessageStream::endmsg;
  } catch (const coral::AuthenticationFailureException& exc) {
    log << coral::Warning << "Authentication failure while attempting to start session for user=\""<<connectionParameter.userName()<<"\" on service \"" << connectionParameter.serviceName() << "\": "<< exc.what() << coral::MessageStream::endmsg;
  } catch (const coral::NoAuthenticationServiceException& exc) {
    log << coral::Warning << "Authentication service missing - Required to start a session on connection \"" << connectionParameter.connectionString() << "\": "<< exc.what() << coral::MessageStream::endmsg;
  } catch (coral::MaximumNumberOfSessionsException& exc) {
    log << coral::Warning << "Could not get a session on new connection to service \"" << connectionParameter.serviceName() <<"\": "<< exc.what() << coral::MessageStream::endmsg;
  } catch (const coral::ServerException& exc) {
    log << coral::Warning << "Failure while attempting to connect to \"" << connectionParameter.connectionString() << "\": "<< exc.what() << coral::MessageStream::endmsg;
  }
  if( sessionHandle.isOpen() ) {
    boost::mutex::scoped_lock lock(m_mutexForPool);
    m_activeConnections.add( sessionHandle.connection() );
    log << coral::Debug << "A new connection to service \""<<sessionHandle.connection().serviceName()<<"\" has been added in the active list. Id="<<sessionHandle.connection().connectionId()<< coral::MessageStream::endmsg;
  }

  // Record the beginning of the connection using the Monitoring service
  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = coral::Context::instance().query<coral::monitor::IMonitoringService>();

  if ( monitoringService.isValid() )
  {
    monitoringService->setLevel( connectionParameter.connectionString(), coral::monitor::Default );
    monitoringService->enable( connectionParameter.connectionString() );
    monitoringService->record( connectionParameter.connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.connectionBegin() );
  }

  return sessionHandle;
}

//-----------------------------------------------------------------------------

coral::ConnectionService::SessionHandle
coral::ConnectionService::ConnectionPool::getSessionFromConnectionPool( const ConnectionParams& connectionParameter,
                                                                        AccessMode accessMode )
{
  SessionHandle sessionHandle( m_connectionServiceConfiguration.serviceName() );
  coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );

  boost::mutex::scoped_lock lock(m_mutexForPool);

  // if idle pool is not empty, first look up on it...
  if(m_idleConnections.size()>0) {

    ConnectionMap::Iterator iConn =
      m_idleConnections.lookup( connectionParameter.technologyName(), connectionParameter.serviceName() );

    if( iConn!=m_idleConnections.end() ) {
      try {
        if(iConn->isValid()) {
          log << coral::Debug << "Try to open a new session on idle connection Id="<<iConn->connectionId()<< coral::MessageStream::endmsg;
          SessionHandle sessH( m_connectionServiceConfiguration.serviceName(), *iConn );
          sessH.open( connectionParameter.schemaName(),
                      connectionParameter.userName(),
                      connectionParameter.password(),
                      accessMode );
          if( sessH.isOpen() ) {
            sessionHandle = sessH;
            m_idleConnections.remove( iConn );
            m_activeConnections.add( sessionHandle.connection() );
            log << coral::Debug << "The idle connection Id="<<sessionHandle.connection().connectionId()<<" to service \"" << sessionHandle.connection().serviceName() << "\" has been moved to the active list."<< coral::MessageStream::endmsg;
          }
        } else {
          log << coral::Debug << "Removing closed connection Id="<<iConn->connectionId()<<" on service=\""<<iConn->serviceName()<<"\" from the idle list."<< coral::MessageStream::endmsg;
          m_idleConnections.remove(iConn);
        }
      } catch (const coral::AuthenticationFailureException& exc) {
        log << coral::Warning << "Authentication failure while attempting to start session for user=\""<<connectionParameter.userName()<<"\" on service \"" << connectionParameter.serviceName() << "\": "<< exc.what() << coral::MessageStream::endmsg;
      } catch (const coral::NoAuthenticationServiceException& exc) {
        log << coral::Warning << "Authentication service missing - Required to start a session on connection \"" << connectionParameter.connectionString() << "\": "<< exc.what() << coral::MessageStream::endmsg;
      } catch (coral::MaximumNumberOfSessionsException& exc) {
        log << coral::Warning << "Could not get a new session on connection to service \"" << iConn->serviceName() << "\" Id="<<iConn->connectionId()<<": "<< exc.what() << coral::MessageStream::endmsg;
      } catch (const coral::ServerException& exc) {
        log << coral::Warning << "Failure while attempting to start a session on connection \"" << connectionParameter.connectionString() << "\": "<< exc.what() << coral::MessageStream::endmsg;
      }
    }
  }

  if( !sessionHandle && m_connectionServiceConfiguration.isConnectionSharingEnabled()) {

    ConnectionMap::Iterator iConn = m_activeConnections.lookup( connectionParameter.technologyName(),
                                                                connectionParameter.serviceName());
    if( iConn!=m_activeConnections.end() ) {
      try {
        if(iConn->isValid()) {
          log << coral::Debug << "Try to open a new session on shared active connection Id="<<iConn->connectionId()<< coral::MessageStream::endmsg;
          SessionHandle sessH( m_connectionServiceConfiguration.serviceName(), *iConn );
          sessH.open( connectionParameter.schemaName(),
                      connectionParameter.userName(),
                      connectionParameter.password(),
                      accessMode );
          if( sessH.isOpen() ) {
            sessionHandle = sessH;
            log << coral::Debug << "New session open on active connection Id="<<iConn->connectionId()<<" to service \"" << iConn->serviceName() <<"\""<< coral::MessageStream::endmsg;
          }
        } else {
          log << coral::Debug << "Removing closed connection Id="<<iConn->connectionId()<<" on service \""<<iConn->serviceName()<<"\" from the active list."<< coral::MessageStream::endmsg;
          m_activeConnections.remove(iConn);
        }
      } catch (const coral::AuthenticationFailureException& exc) {
        log << coral::Warning << "Authentication failure while attempting to start session for user \""<<connectionParameter.userName()<<"\" on service \"" << connectionParameter.serviceName() << "\": "<< exc.what() << coral::MessageStream::endmsg;
      } catch (const coral::NoAuthenticationServiceException& exc) {
        log << coral::Warning << "Authentication service missing - Required to start a session on connection \"" << connectionParameter.connectionString() << "\": "<< exc.what() << coral::MessageStream::endmsg;
      } catch (coral::MaximumNumberOfSessionsException& exc) {
        log << coral::Warning << "Could not get a new session on connection to service \"" << iConn->serviceName() << "\" Id="<<iConn->connectionId()<<": "<< exc.what() << coral::MessageStream::endmsg;
      } catch (const coral::ServerException& exc) {
        log << coral::Warning << "Failure while attempting to start a session on connection \"" << connectionParameter.connectionString() << "\": "<< exc.what() << coral::MessageStream::endmsg;
      }
    }

  }
  return sessionHandle;
}

//-----------------------------------------------------------------------------

coral::ConnectionService::SessionHandle
coral::ConnectionService::ConnectionPool::getValidSession( const std::string& connectionString,
                                                           const std::string& role,
                                                           AccessMode accessMode,
                                                           const ICertificateData *cert )
{
  // initialize pool if required
  startPool();

  coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );

  SessionHandle sessionHandle( m_connectionServiceConfiguration.serviceName() );

  std::vector<ConnectionParams>* replicas = m_replicaCatalogue->getReplicas(connectionString, accessMode);

  unsigned int iReplica = 0;
  bool sessionOk = false;
  while( iReplica < replicas->size() && !sessionOk ) {
    ConnectionParams& params = (*replicas)[iReplica];
    DataSource dataSource(m_connectionServiceConfiguration);
    if(dataSource.setupForConnection( params, role, cert )) {
      sessionHandle = getSessionFromConnectionPool( params, accessMode );
      sessionOk = sessionHandle.isOpen();
      if( !sessionOk ) {
        log << coral::Debug << "No valid connection for string \""<< params.connectionString() << "\" found in the pool. Creating a new connection." << coral::MessageStream::endmsg;
        if(!m_replicaCatalogue->isConnectionExcluded(params.technologyName(),params.serviceName())) {
          sessionHandle = getSessionFromNewConnection( params, accessMode );
          sessionOk = sessionHandle.isOpen();
        }
      }
    } else {
      log << coral::Info << "Replica with connection \""<<params.connectionString() <<"\" is invalid and will be skipped."<< coral::MessageStream::endmsg;
    }
    iReplica++;
  }
  delete replicas;
  if(!sessionOk) {
    throw coral::ConnectionNotAvailableException( connectionString,"ConnectionPool::getSessionFromNewConnection" );
  }

  return sessionHandle;
}

//-----------------------------------------------------------------------------

bool
coral::ConnectionService::ConnectionPool::releaseConnection( ConnectionHandle& connection )
{
  if(!connection) return false;

  boost::mutex::scoped_lock lock(m_mutexForPool);
  bool connected = connection.isValid();
  unsigned nsess = connection.numberOfSessions();
  coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );

  log << coral::Debug << "Connection id=" << connection.connectionId()
      << " to service \"" << connection.serviceName()
      << "\" has " << nsess << " session(s) opened."
      << coral::MessageStream::endmsg;
  if ( !connected || nsess==0 )
  {
    log << coral::Verbose // TEMPORARY... debug bug #61090
        << "The active connection id=" << connection.connectionId()
        << " to service \"" << connection.serviceName()
        << "\" will be removed from the active list."
        << coral::MessageStream::endmsg;
    m_activeConnections.remove( connection );
    log << coral::Debug
        << "The active connection id=" << connection.connectionId()
        << " to service \"" << connection.serviceName()
        << "\" has been removed from the active list."
        << coral::MessageStream::endmsg;
    if ( connected )
    {
      connection.setIdle();
      // if time out is set>0, move the idle connection to idle pool
      if ( connection.specificTimeOut()>0 )
      {
        m_idleConnections.add( connection );
        log << coral::Debug
            << "The valid active connection id=" << connection.connectionId()
            << " to service \"" << connection.serviceName()
            << "\" has been moved to the idle list."
            << coral::MessageStream::endmsg;
      }
    }
    else
    {
      log << coral::Debug
          << "The invalid connection id=" << connection.connectionId()
          << " to service \"" << connection.serviceName()
          << "\" has been removed from the pool."
          << coral::MessageStream::endmsg;
    }
  }
  else
  {
    log << coral::Verbose // TEMPORARY... debug bug #61090
        << "The active connection id=" << connection.connectionId()
        << " to service \"" << connection.serviceName()
        << "\" will NOT be removed from the active list."
        << coral::MessageStream::endmsg;
  }
  return true;
}

//-----------------------------------------------------------------------------

bool
coral::ConnectionService::ConnectionPool::removeConnection( ConnectionHandle& connection )
{
  if( !connection ) return false;
  boost::mutex::scoped_lock lock(m_mutexForPool);
  m_activeConnections.remove( connection );
  coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );

  log << coral::Debug << "The invalid connection id="<< connection.connectionId()<<" to service \""
      <<connection.serviceName() << "\" has been removed from the pool." << coral::MessageStream::endmsg;

  // Record the ending of the connection
  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = coral::Context::instance().query<coral::monitor::IMonitoringService>();

  if ( monitoringService.isValid() )
  {
    monitoringService->record( connection.connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.connectionEnd() );
    monitoringService->disable( connection.connectionString() );
  }

  return true;
}

//-----------------------------------------------------------------------------

void
coral::ConnectionService::ConnectionPool::clearConnectionPool()
{
  boost::mutex::scoped_lock lock(m_mutexForPool);
  m_idleConnections.clear();
  m_activeConnections.clear();
}

//-----------------------------------------------------------------------------

void
coral::ConnectionService::ConnectionPool::cleanUpTimedOutConnections()
{
  boost::mutex::scoped_lock lockForDestruction(m_mutexForThread);
  boost::mutex::scoped_lock lockForCleanUp(m_mutexForPool);
  if( m_idleConnections.size() > 0 )
  {
    std::vector< ConnectionMap::Iterator > timedOutConnections;

    for( ConnectionMap::Iterator iter = m_idleConnections.begin(); iter!= m_idleConnections.end(); ++iter ) {
      if( iter->isExpired() ) {
        timedOutConnections.push_back( iter );
      }
    }

    coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );

    log << coral::Debug << "Checking idle pool for timed-out connections - Total="<<m_idleConnections.size()<<", Expired="<<timedOutConnections.size()  << coral::MessageStream::endmsg;

    for( std::vector< ConnectionMap::Iterator >::iterator vecIter = timedOutConnections.begin();
         vecIter!= timedOutConnections.end();
         vecIter++ )
    {
      log << coral::Debug << "The timed-out idle connection id="<<(*vecIter)->connectionId()<<" to service \""<<(*vecIter)->serviceName() << "\" has been removed from the pool." << coral::MessageStream::endmsg;
      m_idleConnections.remove( *vecIter );
    }
  }
}

//-----------------------------------------------------------------------------

size_t
coral::ConnectionService::ConnectionPool::numberOfIdleConnections()
{
  boost::mutex::scoped_lock lock(m_mutexForPool);
  return m_idleConnections.size();
}

//-----------------------------------------------------------------------------

size_t
coral::ConnectionService::ConnectionPool::numberOfActiveConnections()
{
  boost::mutex::scoped_lock lock(m_mutexForPool);
  return m_activeConnections.size();
}

//-----------------------------------------------------------------------------

bool
coral::ConnectionService::ConnectionPool::waitForTimeout()
{
  //static std::string svcName = ""; // Also: is this the 1st call?
  //if ( svcName == "" )
  //{
  //  svcName = m_connectionServiceConfiguration.serviceName();
  //  std::cout << svcName << " ... poolCleanup initialized (period: "
  //            << m_connectionServiceConfiguration.poolCleanUpPeriod()
  //            << " seconds)" << std::endl;
  //}
  boost::xtime xt;
  boost::xtime_get(&xt, boost::TIME_UTC);
  if( m_connectionServiceConfiguration.poolCleanUpPeriod() > 0 )
    xt.sec += m_connectionServiceConfiguration.poolCleanUpPeriod();
  else
    xt.sec += 5;  // Default time period of 5 seconds
  //std::cout << svcName << " ... poolCleanup - sleep "
  //          << m_connectionServiceConfiguration.poolCleanUpPeriod()
  //          << " seconds" << std::endl;

  // Try to lock, until timeout reached or unlocked by main thread
  if( m_finalized.timed_lock(xt) )
  {
    // Was able to lock: this means the main thread called unlock
    m_finalized.unlock();
    //std::cout << svcName << " ... poolCleanup finalized" << std::endl;
    return false;
  }
  else
  {
    // Was unable to lock: timeout was reached
    //std::cout << svcName << " ... poolCleanup - slept" << std::endl;
    return true;
  }
}

//-----------------------------------------------------------------------------

void
coral::ConnectionService::ConnectionPool::startPool()
{
  boost::mutex::scoped_lock lock(m_mutexForPool);
  static bool first = true;
  if ( !m_poolStarted )
  {
    if( m_connectionServiceConfiguration.connectionTimeOut() > 0 &&
        m_connectionServiceConfiguration.isPoolAutomaticCleanUpEnabled() )
    {
      coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );
      if ( !first ) log << coral::Info << "The connection pool automatic cleanup thread has been re-enabled" << coral::MessageStream::endmsg;
      log << coral::Info << "Starting the connection pool automatic cleanup thread" << coral::MessageStream::endmsg;
      // Mark the pool as started
      m_poolStarted = true;
      // Instantiate cleaner timer
      ConnectionPoolTimer timer(*this);
      // Lock the mutex for timed locking
      m_finalized.lock();
      // Create new instance of thread
      m_timerThread = new boost::thread(timer);
    }
    else if ( first )
    {
      coral::MessageStream log( m_connectionServiceConfiguration.serviceName() );
      if( !m_connectionServiceConfiguration.isPoolAutomaticCleanUpEnabled() )
        log << coral::Info << "The connection pool automatic cleanup thread is currently explicitly disabled" << coral::MessageStream::endmsg;
      else
        log << coral::Info << "The connection pool automatic cleanup thread is currently disabled (connection timeout is 0)" << coral::MessageStream::endmsg;
    }
    first = false;
  }
}

//-----------------------------------------------------------------------------
