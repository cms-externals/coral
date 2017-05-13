#include <iostream>
#include <exception>
#include "DmlApp.h"
#include "CoralBase/Exception.h"

int main( int, char** )
{
  try {
    DmlApp app( "mysql://itrac424.cern.ch/test", "rado_pool", "rado_pool" );
    //DmlApp app( "mysql://pcitdb59.cern.ch/test", "rado_pool", "rado_pool" );
    //DmlApp app( "mysql://pcitdb59.cern.ch:3307/test", "rado_pool", "rado_pool" );
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
