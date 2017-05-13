#include "CoralBase/Exception.h"
#include <iostream>

#include "Test01.h"

int main( int argc, char* argv[] )
{
  Test01 TC01("RTRYFAIL");

  if(TC01.check(argc, argv))
  {
    try
    {



    }
    TESTCORE_FETCHERRORS
      }
  return 1;

  /*
  try {
    coral::Testing::ApplicationBase* application = 0;
    try
    {
      std::string testName = "";
      if ( argc > 1 ) testName = argv[1];
      std::string args = "";
      for ( int iarg = 2; iarg < argc; ++iarg ) args += std::string( argv[iarg] ) + " ";

      application = coral::Testing::ApplicationFactory::instance().application( testName, args );

      application->run();
    }
    catch ( coral::Exception& e ) {
      std::cerr << "CORAL Exception : " << e.what() << std::endl;
    }
    catch ( std::exception& e ) {
      std::cerr << "C++ Standard Exception : " << e.what() << std::endl;
    }
    catch ( ... ) {
      std::cerr << "Unhandled exception ..." << std::endl;
    }

    delete application;
  }
  catch ( coral::Exception& e ) {
    std::cerr << "CORAL Exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << "C++ Standard Exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Unhandled exception ..." << std::endl;
    return 1;
  }
  return 0;
  */
}
