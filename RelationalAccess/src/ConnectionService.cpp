#include "RelationalAccess/ConnectionService.h"

#include "CoralBase/Exception.h"
#include "CoralBase/MessageStream.h"

#include "CoralKernel/Context.h"

namespace coral
{
  namespace Pimpl
  {
    class Holder
    {
    public:
      coral::IHandle<coral::IConnectionService> svc;
    };
  }
}

coral::ConnectionService::ConnectionService()
  : m_connectionService( new coral::Pimpl::Holder() )
{
}


coral::ConnectionService::~ConnectionService()
{
  delete m_connectionService;
}

void
coral::ConnectionService::setMessageVerbosityLevel( coral::MsgLevel level )
{
  coral::MessageStream::setMsgVerbosity( level );
}

void
coral::ConnectionService::loadConnectionService() const
{
  coral::Context& ctx = coral::Context::instance();

  coral::IHandle<coral::IConnectionService> conSvc = ctx.query<coral::IConnectionService>();

  if ( ! conSvc.isValid() )
  {
    std::string pluginName("CORAL/Services/ConnectionService");
    coral::MessageStream log( "CORAL" );
    log << coral::Info << "Loading default plugin for coral::IConnectionService: " << pluginName << std::flush;
    ctx.loadComponent( pluginName );
    conSvc = ctx.query<coral::IConnectionService>();
  }

  if( ! conSvc.isValid() )
    throw coral::Exception( "Could not load CORAL ConnectionService component", "coral::ConnectionService", "Constructor" );

  this->m_connectionService->svc = conSvc;
}


coral::ISessionProxy*
coral::ConnectionService::connect( const std::string& connectionName,
                                   coral::AccessMode accessMode )
{
  if( ! this->m_connectionService->svc.isValid() )
    this->loadConnectionService();
  return m_connectionService->svc->connect( connectionName, accessMode );
}


coral::ISessionProxy*
coral::ConnectionService::connect( const std::string& connectionName,
                                   const std::string& asRole,
                                   coral::AccessMode accessMode )
{
  if( ! this->m_connectionService->svc.isValid() )
    this->loadConnectionService();

  return m_connectionService->svc->connect( connectionName, asRole, accessMode );
}


coral::IConnectionServiceConfiguration&
coral::ConnectionService::configuration()
{
  if( ! this->m_connectionService->svc.isValid() )
    this->loadConnectionService();
  return m_connectionService->svc->configuration();
}


void
coral::ConnectionService::purgeConnectionPool()
{
  if( ! this->m_connectionService->svc.isValid() )
    this->loadConnectionService();
  m_connectionService->svc->purgeConnectionPool();
}


const coral::IMonitoringReporter&
coral::ConnectionService::monitoringReporter() const
{
  if( ! this->m_connectionService->svc.isValid() )
    this->loadConnectionService();
  return m_connectionService->svc->monitoringReporter();
}


coral::IWebCacheControl&
coral::ConnectionService::webCacheControl()
{
  if( ! this->m_connectionService->svc.isValid() )
    this->loadConnectionService();
  return m_connectionService->svc->webCacheControl();
}
