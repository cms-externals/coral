#include <iostream>
#include <exception>
#include "SchemaCopyApp.h"
#include "CoralBase/Exception.h"

int main( int, char** )
{
  try {
    SchemaCopyApp app( "mysql://itrac424.cern.ch", "RADO_POOL", "test", "rado_pool", "rado_pool" );
    //SchemaCopyApp app( "mysql://pcitdb59.cern.ch", "Schema_A", "Schema_B", "rado_pool", "rado_pool" );
    //SchemaCopyApp app( "mysql://pcitdb29.cern.ch/rado_pool", "rado_pool", "rado_pool" );
    app.run();
  }
  catch ( coral::Exception& e ) {
    std::cerr << "CORAL Exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << "C++ Exception : " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Unknown exception ..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
