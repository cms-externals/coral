#include "CoralBase/Exception.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"

#include "ConnectionService.h"
#include "ConnectionServiceConfiguration.h"
#include "ConnectionPool.h"
#include "SessionProxy.h"
#include "WebCacheControl.h"

coral::ConnectionService::ConnectionService::ConnectionService( const std::string& componentName )
  : coral::Service( componentName )
  , m_connectionPool( 0 )
  , m_configuration( 0 )
  , m_webCacheControl( 0 )
  , m_sessions()
  , m_mutexLock()
{
  //std::cout << "Create ConnectionService" << std::endl;
  m_configuration = new ConnectionServiceConfiguration( coral::Service::name() );
  m_connectionPool = new ConnectionPool( *m_configuration );
  m_webCacheControl =  new WebCacheControl( this->m_configuration->serviceContext(), this->name() );
}

coral::ConnectionService::ConnectionService::~ConnectionService()
{
  //std::cout << "Delete ConnectionService " << this << "..." << std::endl;
  for( std::set<SessionProxy*>::const_iterator iSess = m_sessions.begin(); iSess!=m_sessions.end(); iSess++)
  {
    (*iSess)->invalidate();
  }
  delete m_connectionPool;
  delete m_configuration;
  delete m_webCacheControl;
  //std::cout << "Delete ConnectionService " << this << "... done" << std::endl;
}


coral::ISessionProxy*
coral::ConnectionService::ConnectionService::connect( const std::string& connectionString,
                                                      AccessMode accessMode,
                                                      const ICertificateData* cert )
{
  SessionProxy* theSession = 0;
  try
  {
    theSession = new coral::ConnectionService::SessionProxy( connectionString, accessMode, this );
    theSession->open( cert );
    {
      boost::mutex::scoped_lock lock(m_mutexLock);
      m_sessions.insert(theSession);
    }
    m_connectionPool->cleanUpTimedOutConnections();
  }
  catch ( const coral::Exception& )
  {
    delete theSession;
    theSession = 0;
    throw;
  }
  return static_cast<ISessionProxy*>(theSession);
}

coral::ISessionProxy*
coral::ConnectionService::ConnectionService::connect( const std::string& connectionString,
                                                      const std::string& asRole,
                                                      AccessMode accessMode,
                                                      const ICertificateData* cert )
{
  SessionProxy* theSession = 0;
  try
  {
    theSession = new coral::ConnectionService::SessionProxy( connectionString, asRole, accessMode, this );
    theSession->open( cert );
    {
      boost::mutex::scoped_lock lock(m_mutexLock);
      m_sessions.insert(theSession);
    }
    m_connectionPool->cleanUpTimedOutConnections();
  }
  catch( const coral::Exception& )
  {
    delete theSession;
    theSession = 0;
    throw;
  }
  return static_cast<ISessionProxy*>(theSession);
}


void
coral::ConnectionService::ConnectionService::purgeConnectionPool()
{
  m_connectionPool->cleanUpTimedOutConnections();
}


coral::IConnectionServiceConfiguration& coral::ConnectionService::ConnectionService::configuration()
{
  return *m_configuration;
}


const coral::IMonitoringReporter&
coral::ConnectionService::ConnectionService::monitoringReporter() const
{
  return m_configuration->monitoringService().reporter();
}


coral::IWebCacheControl&
coral::ConnectionService::ConnectionService::webCacheControl()
{
  return *m_webCacheControl;
}


coral::ConnectionService::ConnectionPool&
coral::ConnectionService::ConnectionService::connectionPool()
{
  return *m_connectionPool;
}


void
coral::ConnectionService::ConnectionService::unRegisterSession( SessionProxy* sessionProxy )
{
  boost::mutex::scoped_lock lock(m_mutexLock);
  m_sessions.erase(sessionProxy);
}


size_t
coral::ConnectionService::ConnectionService::numberOfIdleConnectionsInPool()
{
  return m_connectionPool->numberOfIdleConnections();
}


size_t
coral::ConnectionService::ConnectionService::numberOfActiveConnectionsInPool()
{
  return m_connectionPool->numberOfActiveConnections();
}
