#include "TestEnv/SealSwitch.h"
#include "TestEnv/TestEnvExceptions.h"

#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"

#include <sstream>

namespace coral {

#ifdef CORAL_SEALED

#include "SealKernel/Context.h"

  //-----------------------------------------------------------------------------

  SealSwitcher::SealSwitcher()
    : m_context( new seal::Context )
  {

  }

  //-----------------------------------------------------------------------------

  SealSwitcher::~SealSwitcher()
  {

  }

  //-----------------------------------------------------------------------------

  void
  SealSwitcher::loadComponent(std::string component)
  {
    seal::PluginManager* pm = seal::PluginManager::get();
    pm->initialise();
    seal::Handle<seal::ComponentLoader> loader = new seal::ComponentLoader( m_context.get() );

    seal::IHandle<seal::IMessageService> msg =
      m_context->query<seal::IMessageService>( "SEAL/Services/MessageService" );
    if( !msg ) {
      loader->load( "SEAL/Services/MessageService" );
      std::vector< seal::Handle<seal::IMessageService> > v_msgSvc;
      m_context->query( v_msgSvc );
      if ( ! v_msgSvc.empty() ) {
        seal::Handle<seal::IMessageService>& msgSvc = v_msgSvc.front();
        msgSvc->setOutputLevel( seal::Msg::Info);
      }
    }

    loader->load( component );
  }

  //-----------------------------------------------------------------------------

  coral::IConnectionService&
  SealSwitcher::getConnectionService()
  {
    seal::IHandle<coral::IConnectionService> handle(m_context->query<coral::IConnectionService>( "CORAL/Services/ConnectionService" ));
    if ( ! handle ) loadComponent("CORAL/Services/ConnectionService");
    handle = m_context->query<coral::IConnectionService>( "CORAL/Services/ConnectionService" );
    if ( ! handle ) throw TestEnvException( "Could not load the CORAL connection service", "SealSwitcher::getConnectionService()" );
    return *handle;
  }
  //-----------------------------------------------------------------------------


  coral::IRelationalDomain&
  SealSwitcher::getDomain(const std::string& connectionString)
  {
    std::vector< seal::IHandle<coral::IRelationalService> > v_svc;
    m_context->query( v_svc );
    if ( v_svc.empty() ) throw TestEnvException( "Could not locate the relational service", "SealSwitcher::getDomain()" );
    seal::IHandle<coral::IRelationalService>& relationalService = v_svc.front();
    return relationalService->domainForConnection( connectionString );
  }

  //-----------------------------------------------------------------------------

  const coral::IAuthenticationCredentials&
  SealSwitcher::getCredentials(const std::string& connectionString)
  {
    seal::IHandle<coral::IAuthenticationService> authsvc = m_context->component( "CORAL/Services/XMLAuthenticationService" );
    if( ! authsvc ) throw TestEnvException( "Could not locate authentication service!", "SealSwitcher::getCredentials()" );
    return authsvc->credentials( connectionString );
  }

  //-----------------------------------------------------------------------------

  std::vector<std::string>
  SealSwitcher::getAllReplicas(const std::string serviceName, coral::AccessMode accessMode, size_t minreplicas)
  {
    seal::IHandle<coral::ILookupService> lookup = m_context->component("CORAL/Services/XMLLookupService");
    if ( ! lookup ) {
      loadComponent("CORAL/Services/XMLLookupService");
    }
    lookup = m_context->query<coral::ILookupService>("CORAL/Services/XMLLookupService");
    if(! lookup ) throw TestEnvException( "Could not locate the lookup service" , "SealSwitcher::getAllReplicas()" );

    coral::IDatabaseServiceSet * serviceSet = lookup->lookup(serviceName, accessMode);

    std::vector<std::string> connstrings;

    size_t size = serviceSet->numberOfReplicas();

    if(size < minreplicas) {
      std::ostringstream s;
      s << "Servicename (" << serviceName << ") contains (" << size << ") replicas, but the minum is set to (" << minreplicas << ")";
      throw TestEnvException( s.str() , "SealSwitcher::SealSwitcher::getAllReplicas()" );
    }

    for(size_t i = 0; i < size; i++ )
      connstrings.push_back(serviceSet->replica(i).connectionString());

    return connstrings;
  }

  //-----------------------------------------------------------------------------

  bool
  SealSwitcher::hasServiceName(const std::string serviceName)
  {
    seal::IHandle<coral::ILookupService> lookup = m_context->component("CORAL/Services/XMLLookupService");
    if ( ! lookup ) {
      loadComponent("CORAL/Services/XMLLookupService");
    }
    lookup = m_context->query<coral::ILookupService>("CORAL/Services/XMLLookupService");
    if(! lookup ) throw TestEnvException( "Could not locate the lookup service" , "SealSwitcher::hasServiceName()" );

    coral::IDatabaseServiceSet * serviceSet = lookup->lookup(serviceName);
    return (serviceSet->numberOfReplicas() > 0);
  }

  //-----------------------------------------------------------------------------

  const coral::IMonitoringReporter&
  SealSwitcher::getMonitoringReporter()
  {
    std::vector< seal::IHandle<coral::monitor::IMonitoringService> > v_svc;
    m_context->query( v_svc );
    if ( v_svc.empty() ) throw TestEnvException( "Could not locate the monitoring service" , "SealSwitcher::getMonitoringReporter()" );
    seal::IHandle<coral::monitor::IMonitoringService>& monitoringService = v_svc.front();
    return monitoringService->reporter();
  }

  //-----------------------------------------------------------------------------

  seal::Handle<seal::Context>&
  SealSwitcher::getSealContext()
  {
    return m_context;
  }

  //-----------------------------------------------------------------------------


#else

  SealSwitcher::SealSwitcher()
  {

  }

  //-----------------------------------------------------------------------------

  SealSwitcher::~SealSwitcher()
  {

  }

  //-----------------------------------------------------------------------------

  void
  SealSwitcher::loadComponent(std::string component)
  {
    coral::Context& context = coral::Context::instance();
    context.loadComponent( component );
  }

  //-----------------------------------------------------------------------------

  coral::IConnectionService&
  SealSwitcher::getConnectionService()
  {
    coral::IHandle<coral::IConnectionService> handle(coral::Context::instance().query<coral::IConnectionService>( "CORAL/Services/ConnectionService" ));
    if( !handle.isValid() ) loadComponent("CORAL/Services/ConnectionService");
    handle = coral::Context::instance().query<coral::IConnectionService>( "CORAL/Services/ConnectionService" );
    if( !handle.isValid() ) throw TestEnvException( "Could not load the CORAL connection service", "SealSwitcher::getConnectionService()" );
    return *(handle.get());
  }

  //-----------------------------------------------------------------------------

  coral::IRelationalDomain&
  SealSwitcher::getDomain(const std::string& connectionString)
  {
    coral::IHandle<coral::IRelationalService> svc = coral::Context::instance().query<coral::IRelationalService>();
    if ( ! svc.isValid() ) throw TestEnvException( "Could not locate the relational service", "SealSwitcher::getDomain()" );
    return svc->domainForConnection( connectionString );
  }

  //-----------------------------------------------------------------------------

  const coral::IAuthenticationCredentials&
  SealSwitcher::getCredentials(const std::string& connectionString)
  {
    coral::IHandle<coral::IAuthenticationService>
      authsvc = coral::Context::instance().query<coral::IAuthenticationService>( "CORAL/Services/XMLAuthenticationService" );

    if( ! authsvc.isValid() )
      throw TestEnvException( "Could not locate authentication service!", "SealSwitcher::getCredentials()" );

    return authsvc->credentials( connectionString );
  }

  //-----------------------------------------------------------------------------

  std::vector<std::string>
  SealSwitcher::getAllReplicas(const std::string serviceName, coral::AccessMode accessMode, size_t minreplicas)
  {
    coral::Context& ctx = coral::Context::instance();
    coral::IHandle<coral::ILookupService> lookupService = ctx.query<coral::ILookupService>();
    if ( ! lookupService.isValid() ) {
      ctx.loadComponent("CORAL/Services/XMLLookupService");
    }
    lookupService = ctx.query<coral::ILookupService>();
    if(! lookupService.isValid() )
      throw TestEnvException( "Could not locate the lookup service" , "SealSwitcher::getAllReplicas()" );

    coral::IDatabaseServiceSet * serviceSet = lookupService->lookup(serviceName, accessMode);

    std::vector<std::string> connstrings;

    size_t size = serviceSet->numberOfReplicas();
    //check if we have the minimum size of replicas
    if(size < minreplicas) {
      std::ostringstream s;
      s << "Servicename (" << serviceName << ") contains (" << size << ") replicas, but the minum is set to (" << minreplicas << ")";
      throw TestEnvException( s.str() , "SealSwitcher::SealSwitcher::getAllReplicas()" );
    }


    for(size_t i = 0; i < size; i++ )
      connstrings.push_back(serviceSet->replica(i).connectionString());

    return connstrings;
  }

  //-----------------------------------------------------------------------------

  bool
  SealSwitcher::hasServiceName(const std::string serviceName)
  {
    coral::Context& ctx = coral::Context::instance();
    coral::IHandle<coral::ILookupService> lookupService = ctx.query<coral::ILookupService>();
    if ( ! lookupService.isValid() ) {
      ctx.loadComponent("CORAL/Services/XMLLookupService");
    }
    lookupService = ctx.query<coral::ILookupService>();
    if(! lookupService.isValid() )
      throw TestEnvException( "Could not locate the lookup service" , "SealSwitcher::hasServiceName()" );

    coral::IDatabaseServiceSet * serviceSet = lookupService->lookup(serviceName);

    return (serviceSet->numberOfReplicas() > 0);
  }

  //-----------------------------------------------------------------------------

  const coral::IMonitoringReporter&
  SealSwitcher::getMonitoringReporter()
  {
    coral::IHandle<coral::monitor::IMonitoringService>
      monitoringService = coral::Context::instance().query<coral::monitor::IMonitoringService>();
    if( ! monitoringService.isValid() )
      throw TestEnvException( "Could not locate the monitoring service" , "SealSwitcher::getMonitoringReporter()" );

    return monitoringService->reporter();
  }

  //-----------------------------------------------------------------------------

#endif

}
