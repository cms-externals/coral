#include "QueriesApp.h"

#include "CoralBase/Exception.h"

#include <iostream>
#include <stdexcept>

int main( int, char** )
{
  try {
    //QueriesApp app( "oracle://cms_val_lb/CORAL", "frontier://lxfs6043.cern.ch:8080/Frontier3D/CORAL", "CORAL", "Frontier24" );
    //QueriesApp app( "oracle://bd3d/RADOVAN", "frontier://frontier3d.cern.ch:8080/Frontier/RADOVAN", "radovan", "radovan" );
    //QueriesApp app( "/dbdev/Frontier3DUpdate", "/dbdev/Frontier3DRead", "radovan", "radovan" );
    //QueriesApp app( "/dbdev/Frontier3D", "/dbdev/Frontier3DRead", "radovan", "radovan" );
    QueriesApp app( "/coral/cooldev", "/coral/CoolFrontier", "", "" );
    app.run();
  }
  catch ( const coral::Exception& e ) {
    std::cerr << "CORAL Exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( const std::exception& e ) {
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
