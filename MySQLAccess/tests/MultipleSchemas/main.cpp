#include <iostream>
#include <exception>
#include "MultipleSchemasApp.h"
#include "CoralBase/Exception.h"

int main( int, char** )
{
  try {
    MultipleSchemasApp app( "mysql://pcitdb59.cern.ch/RADO_POOL", "rado_pool", "rado_pool", "test", coral::ReadOnly );
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
