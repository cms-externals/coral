#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h" // Needed on WIN32
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"

#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"

#include <iostream>

int main( int, char** )
{

  try
  {
    coral::Context& ctx = coral::Context::instance();

    ctx.loadComponent( "CORAL/Services/ConnectionService" );
    ctx.loadComponent( "CORAL/Services/XMLAuthenticationService" );

    coral::IHandle<coral::IConnectionService>
      connSvc = ctx.query<coral::IConnectionService>( "CORAL/Services/ConnectionService" );

    connSvc->configuration().setAuthenticationService
      ( "CORAL/Services/XMLAuthenticationService" );

    std::string connectW = "/coral/cooldev";
    std::string connectR = "/coral/CoolFrontier";

    // 1. Write sample data
    std::cout << "_TEST Write sample data" << std::endl;
    coral::AccessMode accessModeW = coral::Update;
    std::unique_ptr<coral::ISessionProxy>
      sessionW( connSvc->connect( connectW, accessModeW ) );
    sessionW->transaction().start( false );
    sessionW->transaction().commit();

    // 2. Read sample data
    std::cout << "_TEST Read sample data" << std::endl;
    coral::AccessMode accessModeR = coral::ReadOnly;
    std::unique_ptr<coral::ISessionProxy>
      sessionR( connSvc->connect( connectR, accessModeR ) );
    sessionR->transaction().start( true );
    std::unique_ptr<coral::IQuery> query( sessionR->nominalSchema().newQuery() );
    query->addToTableList( "DUAL" );
    query->addToOutputList( "1" );
    std::cout << "_TEST Execute the query" << std::endl;
    // This will fail with a segmentation fault for Frontier (bug #19758)
    query->execute();
    std::cout << "_TEST Executed the query - now commit" << std::endl;
    sessionR->transaction().commit();
    std::cout << "_TEST Completed successfully" << std::endl;

  }

  catch ( std::exception& e ) {
    std::cerr << "Standard C++ exception caught: " << e.what() << std::endl;
    return 1;
  }

  catch ( ... ) {
    std::cerr << "Unknown exception caught" << std::endl;
    return 1;
  }

  return 0;

}
