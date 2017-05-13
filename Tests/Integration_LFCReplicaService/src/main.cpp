#include "TestApp.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>


int main( int, char** )
{
  try {
    // Set the authentication path
    if ( ! ::getenv( "CORAL_AUTH_PATH" ) ) {
      ::putenv( "CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db/LFCReplicaService" );
    }
    if ( ! ::getenv( "CORAL_DBLOOKUP_PATH" ) ) {
      ::putenv( "CORAL_DBLOOKUP_PATH=/afs/cern.ch/sw/lcg/app/pool/db/LFCReplicaService" );
    }

    if ( ! ::getenv( "LFC_HOST" ) ) {
      ::putenv( "LFC_HOST=lxb1925.cern.ch" );
    }
    if ( ! ::getenv( "CORAL_LFC_BASEDIR" ) ) {
      ::putenv( "CORAL_LFC_BASEDIR=/databasetest" );
    }

    TestApp app;
    app.run();
  }
  catch ( coral::Exception& e )
  {
    std::cerr << "CORAL exception " << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e )
  {
    std::cerr << "Standard C++ exception " << e.what() << std::endl;
    return 1;
  }
  catch( ... )
  {
    std::cerr << "Unknown exception ..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
