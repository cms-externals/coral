#include "BulkInsertApp.h"

#include "CoralBase/Exception.h"

#include <iostream>
#include <exception>

int main( int, char** )
{
  try
  {
    BulkInsertApp app( "mysql://pcitdb59.cern.ch/test", "rado_pool", "rado_pool", coral::Update );
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
