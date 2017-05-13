#include "uuid/uuid.h" // Keep it at the top (bug #35692 on Windows)

#include "CoralBase/MessageStream.h"
#include "CoralCommon/IDevConnection.h"
#include "CoralCommon/Utilities.h"
#include "CoralBase/boost_thread_headers.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/SessionException.h"
#include "ConnectionHandle.h"
#include "ConnectionServiceConfiguration.h"

/// default constructor
coral::ConnectionService::ConnectionHandle::ConnectionHandle( const std::string& connectionServiceName )
  : m_connection()
  , m_info( new ConnectionSharedInfo( connectionServiceName ) )
{
}

/// constructor
coral::ConnectionService::ConnectionHandle::ConnectionHandle( IConnection* connection,
                                                              const ConnectionParams& connectionParams,
                                                              const ConnectionServiceConfiguration& configuration)
  : m_connection( connection )
  , m_info( new ConnectionSharedInfo(connectionParams,configuration) )
{
  coral_uuidt guid; // Redefine a 'coral' uuid_t (bug #35692 on Windows)
  ::uuid_generate_time( guid );
  char strGuid[37];
  ::uuid_unparse( guid,strGuid );
  m_info->m_connectionId = std::string(strGuid);
}

/// destructor
coral::ConnectionService::ConnectionHandle::~ConnectionHandle()
{
  coral::MessageStream log( m_info->m_configuration.serviceName() );
  long count = m_connection.use_count();
  if ( count == 1 ) log << coral::Info << "Connection to service \"" << m_info->m_serviceName << "\" with connectionID=" << m_info->showConnectionId() << " will be disconnected" << coral::MessageStream::endmsg;
  //else log << coral::Info << "Connection to service \"" << m_info->m_serviceName << "\" with connectionID="<< m_info->showConnectionId() << " will NOT YET be disconnected (count=" << count << ")" << coral::MessageStream::endmsg;
}

/// copy constructor
coral::ConnectionService::ConnectionHandle::ConnectionHandle( const ConnectionHandle& rhs )
  : m_connection(rhs.m_connection)
  , m_info(rhs.m_info)
{
}

/// assignment operator
coral::ConnectionService::ConnectionHandle&
coral::ConnectionService::ConnectionHandle::operator=( const ConnectionHandle& rhs ){
  if(this!=&rhs) {
    m_connection = rhs.m_connection;
    m_info = rhs.m_info;
  }
  return *this;
}

coral::ConnectionService::ConnectionHandle::operator bool() const
{
  return m_connection.get()!=0;
}

bool
coral::ConnectionService::ConnectionHandle::open()
{
  if(m_connection) {
    //     coral::MessageStream log( m_info->m_configuration.serviceContext(), m_info->m_configuration.serviceName(), coral::Verbose );
    coral::MessageStream log( m_info->m_configuration.serviceName() );
    if( ! this->isOpen() ) {
      m_info->m_open = false;
      coral::TimeStamp startConnectionAttempt = coral::TimeStamp::now(true);
      log << coral::Debug << "Connection parameters are: Retry-Period="<<m_info->m_configuration.connectionRetrialPeriod()<<"sec, Retry-Timeout="<<m_info->m_configuration.connectionRetrialTimeOut()<<"sec, Connection-Timeout="<<m_info->m_configuration.connectionTimeOut()<<"sec."<<coral::MessageStream::endmsg;
      // retrial block (only for ServerExceptions!)
      unsigned int iRetry = 0;
      while(!m_info->m_open) {
        try {
          m_connection->connect();
          m_info->m_open = true;
          log << coral::Info << "New connection to service \"" << m_info->m_serviceName << "\" with connectionID=" << m_info->showConnectionId() << " has been connected" << coral::MessageStream::endmsg; // Matches printout for endUserSession in SessionHandle.cpp
        } catch (const coral::ConnectionException& exc) {
          log << coral::Warning << "Failed to connect to service " << m_info->m_serviceName << " (coral::ConnectionException): '"<< exc.what() << "' - retry unless timed out" << coral::MessageStream::endmsg;
        } catch (const coral::Exception& exc) {
          log << coral::Warning << "Failed to connect to service " << m_info->m_serviceName << " (coral::Exception): '"<< exc.what() << "' - do NOT retry" << coral::MessageStream::endmsg;
          throw;
        } catch (const std::exception& exc) {
          log << coral::Warning << "Failed to connect to service " << m_info->m_serviceName << " (std::exception): '"<< exc.what() << "' - retry unless timed out" << coral::MessageStream::endmsg;
        } catch (...) {
          log << coral::Warning << "Failed to connect to service " << m_info->m_serviceName << " (unknown exception) - retry unless timed out" << coral::MessageStream::endmsg;
        }
        if(!m_info->m_open) {
          if ( (coral::TimeStamp::now(true).total_nanoseconds()-startConnectionAttempt.total_nanoseconds())/1000000000LL < m_info->m_configuration.connectionRetrialTimeOut() ) {
            log << coral::Warning << "Retry (" << ++iRetry << ") after "<< m_info->m_configuration.connectionRetrialPeriod() << " seconds" << coral::MessageStream::endmsg;
            coral::sleepSeconds(m_info->m_configuration.connectionRetrialPeriod());
          } else {
            break;
          }
        }
      }
    }
  }
  return m_info->m_open;
}

/*
/// THIS IS NEVER CALLED! (AV 22.07.2010)
/// finalize (and invalidate) the connection.
bool
coral::ConnectionService::ConnectionHandle::close()
{
  bool ret = false;
  if(m_connection){
    m_connection->disconnect();
  }
  return ret;
}
*/

 /// returns true if the connection is valid
bool
coral::ConnectionService::ConnectionHandle::isOpen() const
{
  if(m_connection==0) {
    return false;
  } else {
    return m_info->m_open;
  }
}

/// returns true if the connection is valid
bool
coral::ConnectionService::ConnectionHandle::isValid() const
{
  if(m_connection==0) {
    return false;
  } else {
    return m_connection->isConnected(true);
  }
}

/// initialize the session
coral::ISession*
coral::ConnectionService::ConnectionHandle::newSession( const std::string& schemaName,
                                                        const std::string& userName,
                                                        const std::string& password,
                                                        coral::AccessMode accessMode,
                                                        const std::string& sessionId )
{
  coral::ISession* physicalSession = 0;
  bool started = false;
  if( isOpen() ) {
    //    coral::MessageStream log( m_info->m_configuration.serviceContext(), m_info->m_configuration.serviceName(), coral::Verbose );
    coral::MessageStream log( m_info->m_configuration.serviceName() );
    coral::TimeStamp startSessionAttempt = coral::TimeStamp::now(true);
    physicalSession = m_connection->newSession( schemaName, accessMode);
    if(physicalSession) {
      try{
        // retrial block (only for SessionExceptions!)
        while(!started) {
          try {
            physicalSession->startUserSession( userName, password );
            if(m_info->m_configuration.isMonitoringEnabled()) {
              physicalSession->monitoring().start( m_info->m_configuration.monitoringLevel() );
            }
            m_info->m_idle = false;
            started = true;
            log << coral::Info << "New user session with sessionID=" << sessionId << " started on connectionID="<< m_info->showConnectionId() << " to service \"" << m_info->m_serviceName << "\" for user \"" << userName << "\" in "<< ( accessMode==coral::ReadOnly ? "read-only" : "update" ) << " mode" << coral::MessageStream::endmsg;
          } catch (const coral::StartSessionException& exc) {
            log << coral::Warning << "Failure while attempting to start a session on connection to service " << m_info->m_serviceName << ": "<< exc.what() << coral::MessageStream::endmsg;
          }
          if( !started ) {
            if ( (coral::TimeStamp::now(true).time()-startSessionAttempt.time()).seconds() < m_info->m_configuration.connectionRetrialTimeOut() ) {
              log << coral::Warning << "Failed to connect to service " << m_info->m_serviceName << ": retry after "<< m_info->m_configuration.connectionRetrialPeriod() << " seconds" << coral::MessageStream::endmsg;
              coral::sleepSeconds(m_info->m_configuration.connectionRetrialPeriod());
            } else {
              delete physicalSession;
              physicalSession = 0;
              break;
            }
          }
        }
      } catch (const coral::AuthenticationFailureException&) {
        delete physicalSession;
        physicalSession = 0;
        throw;
      }
    }
  }
  return physicalSession;
}

///returns the session id
coral::ConnectionService::Guid
coral::ConnectionService::ConnectionHandle::connectionId() const
{
  return m_info->m_connectionId;
}

std::string
coral::ConnectionService::ConnectionHandle::connectionString() const
{
  return m_info->m_connString;
}

/// returns the service name associated to the connection
const std::string&
coral::ConnectionService::ConnectionHandle::serviceName() const
{
  return m_info->m_serviceName;
}

/// returns the technology name
const std::string&
coral::ConnectionService::ConnectionHandle::technologyName() const
{
  return m_info->m_technologyName;
}

/// returns the number of sessions
size_t
coral::ConnectionService::ConnectionHandle::numberOfSessions() const
{
  size_t nsess = 0;
  if(m_connection) {
    nsess = dynamic_cast<coral::IDevConnection*>(m_connection.get())->size();
  }
  return nsess;
}

/// returns the server version
std::string
coral::ConnectionService::ConnectionHandle::serverVersion() const
{
  std::string sVer("");
  if( m_connection ) sVer = m_connection->serverVersion();
  return sVer;
}

coral::ITypeConverter&
coral::ConnectionService::ConnectionHandle::typeConverter() const
{
  if(!m_connection) throw coral::Exception("Session pointer is null","ConnectionService::ConnectionHandle::typeConverter","ConnectionService");
  return m_connection->typeConverter();
}


/// sets the start of idle time
void
coral::ConnectionService::ConnectionHandle::setIdle()
{
  m_info->m_startIdle = coral::TimeStamp::now(true);
  m_info->m_idle = true;
}

/// retrieves the idle flag
bool
coral::ConnectionService::ConnectionHandle::isIdle() const
{
  return m_info->m_idle;
}

/// returns true if timeout is reached
bool
coral::ConnectionService::ConnectionHandle::isExpired() const
{
  return ( coral::TimeStamp::now(true).time()-m_info->m_startIdle.time()).seconds() >= m_info->m_configuration.connectionTimeOut();
}

/// returns the specific time-out of the connection
int
coral::ConnectionService::ConnectionHandle::specificTimeOut()
{
  return m_info->m_configuration.connectionTimeOut();
}

const std::string
coral::ConnectionService::ConnectionHandle::showConnectionId() const
{
  return m_info->showConnectionId();
}

const std::string
coral::ConnectionService::ConnectionSharedInfo::showConnectionId( bool last ) const
{
  static char* simpleIDs = ::getenv( "CORAL_CONNECTIONSVC_SHOWSIMPLEIDS" );
  if ( simpleIDs && m_connectionId != "" )
  {
    static boost::mutex mutexForMap;
    boost::mutex::scoped_lock lock( mutexForMap );
    static std::map<Guid,std::string> idMap;
    static unsigned long lastId = 0;
    std::map<Guid,std::string>::iterator idIt = idMap.find( m_connectionId );
    if ( idIt != idMap.end() )
    {
      // GUID is already mapped
      std::string showId = idIt->second;
      if ( last ) idMap.erase( m_connectionId );
      return showId;
    }
    else if ( !last )
    {
      // GUID is not mapped and will be reused: map it
      std::stringstream msg;
      msg << "'Connection#" << ++lastId << "'";
      idMap[m_connectionId] = msg.str();
      coral::MessageStream log( m_connectionServiceName );
      log << coral::Debug << "Connection GUID=" << m_connectionId << " mapped to connectionID=" << msg.str() << coral::MessageStream::endmsg;
      return msg.str();
    }
    else
    {
      // GUID is not mapped and will not be reused: do not map it
      return m_connectionId;
    }
  }
  return m_connectionId;
}
