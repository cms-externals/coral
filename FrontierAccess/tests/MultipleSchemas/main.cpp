#include <iostream>
#include <exception>
#include "MultipleSchemasApp.h"
#include "CoralBase/Exception.h"

int main( int, char** )
{
  try {
    MultipleSchemasApp app( "frontier://lxfs6043.cern.ch:8080/Frontier3D)", "CORAL", "Frontier24", "RADOVAN" );
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
