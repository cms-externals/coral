
#include <stdio.h>
#include "CoralBase/Exception.h"
#include <iostream>
#include "TestDriver.h"
#include "TestEnv/TestEnv.h"
#include <exception>

int main( int argc, char *argv[] )
{
  TestEnv TC01("INOUT");

  if(TC01.check(argc, argv))
  {
    try
    {
      //add the default connection strings to the test application
      TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_ADMIN);

      InputOutput InOut(TC01);
      InOut.createSession(0);

      InOut.prepareSchema();
      InOut.deleteSession();
      InOut.createSession(0);

      InOut.writeSimple();
      InOut.deleteSession();

      InOut.createSession(0);
      InOut.writeBulk();
      InOut.deleteSession();

      InOut.createSession(0, coral::ReadOnly);
      InOut.read();
      InOut.deleteSession();

      InOut.createSession(0);
      InOut.prepareTypes();
      InOut.deleteSession();

      InOut.createSession(0, coral::ReadOnly);
      InOut.readTypes();
      InOut.deleteSession();
    }
    catch ( coral::Exception& e )
    {
      std::cerr << "CORAL Exception : " << e.what() << std::endl;
      return 1;
    }
    catch ( std::exception& e )
    {
      std::cerr << "C++ Exception : " << e.what() << std::endl;
      return 1;
    }
    catch ( ... )
    {
      std::cerr << "Unhandled exception " << std::endl;
      return 1;
    }
    std::cout << "[OVAL] Success" << std::endl;
    return 0;
  }
  return 1;
}
