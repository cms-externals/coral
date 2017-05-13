#include "PrefetchTest.h"

#include "CoralBase/Exception.h"

#include "TestEnv/TestEnv.h"

#include <stdio.h>
#include <string>
#include <iostream>

int main(int argc, char *argv[]){

  TestEnv TC01("PREFETCH");

  if(TC01.check(argc, argv))
  {
    /* add the default connection strings to the test application */
    TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_ADMIN);

    try
    {
      PreFetch prefetch(TC01);

      prefetch.createSession(0);
      prefetch.write();
      prefetch.deleteSession();

      prefetch.createSession(0, coral::ReadOnly);
      prefetch.read();
      prefetch.deleteSession();

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
