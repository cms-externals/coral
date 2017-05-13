#include <iostream>
#include "CoralAuthenticationService.h"
#include "CredentialsTable.h"
#include "LC2PCTable.h"
#include "Config.h"
#include "QueryMgr.h"

#include "CoralBase/Exception.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"

#include "CoralKernel/IHandle.h"
#include "CoralKernel/Context.h"

#define DEBUG(out) do { std::cout << out << std::endl; } while (0 )

using namespace coral;

//-----------------------------------------------------------------------------

coral::CoralAuthenticationService::CoralAuthenticationService(
                                                              const std::string& componentName )
  : coral::IAuthenticationService()
  , coral::Service( componentName )
{
  DEBUG("CoralAuthenticationService constructor");
}

//-----------------------------------------------------------------------------

coral::CoralAuthenticationService::~CoralAuthenticationService()
{
}

//-----------------------------------------------------------------------------

const IAuthenticationCredentials& CoralAuthenticationService::credentials(
                                                                          const std::string& connectionString ) const
{
  return credentials( connectionString, IAuthenticationCredentials::defaultRole(), 0 );
}

//-----------------------------------------------------------------------------
/*
const IAuthenticationCredentials& CoralAuthenticationService::credentials(
          const std::string& connectionString, const std::string& role ) const
{
  return credentials( connectionString, role, (ICertificateData*)0 );
}
*/
 //-----------------------------------------------------------------------------

const IAuthenticationCredentials& CoralAuthenticationService::credentials(
                                                                          const std::string& connectionString, const std::string& role,
                                                                          const ICertificateData *cert ) const
{
#if 0
  std::string initAuthenticationService("");
  char* authPath = ::getenv ( "CORAL_AUTH_PATH" );
  if ( authPath ) {
    initAuthenticationService = "CORAL/Services/XMLAuthenticationService";
  }
  else {
    if ( ::getenv ( "CORAL_AUTH_USER" ) && ::getenv ( "CORAL_AUTH_PASSWORD" ) )
      initAuthenticationService = "CORAL/Services/EnvironmentAuthenticationService";
    else
      initAuthenticationService = "CORAL/Services/XMLAuthenticationService";
  }
  coral::IHandle<coral::IAuthenticationService> initServ =
    coral::Context::instance().query<coral::IAuthenticationService>(initAuthenticationService);
  if(!initServ.isValid()) {
    coral::Context::instance().loadComponent( initAuthenticationService );
    initServ = coral::Context::instance().query<coral::IAuthenticationService>( initAuthenticationService );
  }

#endif
  DEBUG( "credentials( " << connectionString << ", " << role
         << ", " << cert << ")" );
  coral::IConnectionService *connService = new coral::ConnectionService();

  std::string serviceName = getConfig().dbConnectionString();
  coral::ISessionProxy *session = connService->connect( serviceName,
                                                        coral::ReadOnly );
  if ( !session ) {
    //delete connService;
    throw coral::Exception("Could not connect to " + serviceName,"main()","demo" );
  };

  // start an update transaction
  session->transaction().start( true /*readonly*/ );

  ISchema &schema( session->nominalSchema() );

  QueryMgr queryMgr( schema );

  boost::shared_ptr< AuthenticationCredentials > cred(
                                                      new AuthenticationCredentials( queryMgr.getCredentials( connectionString, role, cert ) ) );

  std::string temp( connectionString );
  temp += role;
  m_store[ temp ] = cred;
  //FIXME CLEAN UP!
  return ( *cred );
}

//-----------------------------------------------------------------------------

IDatabaseServiceSet* CoralAuthenticationService::lookup( const std::string& logicalName,
                                                         AccessMode accessMode, std::string authenticationMechanism ) const
{
  DEBUG("CoralAuthenticationService::lookup( "<< logicalName << ", " << accessMode << ", "
        << authenticationMechanism << " )");

  coral::IConnectionService *connService = new coral::ConnectionService();

  const std::string& serviceName = getConfig().dbConnectionString();
  coral::ISessionProxy *session = connService->connect( serviceName,
                                                        coral::ReadOnly );
  if ( !session ) {
    //delete connService;
    throw coral::Exception("Could not connect to " + serviceName,"main()","demo" );
  };

  // start an update transaction
  session->transaction().start( true /*readonly*/ );

  ISchema &schema( session->nominalSchema() );

  QueryMgr mgr( schema );

  DatabaseServiceSet *serviceSet = mgr.lookup( logicalName, accessMode, authenticationMechanism );

  return serviceSet;
}

//-----------------------------------------------------------------------------

void CoralAuthenticationService::setInputFileName(  const std::string& inputFileName )
{
  DEBUG("CoralAuthenticationService::setInputFileName( "<< inputFileName << " )");
}
