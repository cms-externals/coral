#include <iostream>
#include <memory>
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h" // Needed on WIN32
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ConnectionService.h"

int main( int, char** )
{

  try {

    coral::ConnectionService connSvc;

    std::string connectW = "/coral/cooldev";
    std::string connectR = "/coral/cooldevr";

    // 1. Write sample data
    std::cout << "_TEST Write sample data" << std::endl;
    coral::AccessMode accessModeW = coral::Update;

    std::unique_ptr<coral::ISessionProxy>  sessionW( connSvc.connect( connectW, accessModeW ) );
    sessionW->transaction().start( false );
    sessionW->transaction().commit();

    // 2. Read sample data
    std::cout << "_TEST Read sample data" << std::endl;
    coral::AccessMode accessModeR = coral::ReadOnly;

    std::unique_ptr<coral::ISessionProxy>  sessionR( connSvc.connect( connectR, accessModeR ) );

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
