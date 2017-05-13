#include <iostream>
#include <exception>
#include "QueriesApp.h"
#include "CoralBase/Exception.h"

int main( int, char** )
{
  try {
    QueriesApp app( "mysql://pcitdb59.cern.ch/test", "rado_pool", "rado_pool" );
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
