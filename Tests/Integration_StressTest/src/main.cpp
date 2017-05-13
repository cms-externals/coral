#include "CoralBase/Exception.h"
#include "TestEnv/TestEnv.h"
#include <iostream>

#include "StressTest01.h"

int main(int argc, char *argv[]){

  TestEnv TC01("STEST");

  if(TC01.check(argc, argv))
  {

    //add the default connection strings to the test application
    TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_ADMIN);

    try{
      // Start with 1MB
      StressTest01 Test01(TC01, 1000000);

      Test01.createSession(0);

      Test01.prepare();

      Test01.deleteSession();
      Test01.createSession(0, coral::ReadOnly);

      Test01.run(20);

      Test01.deleteSession();

      StressTest01 Test02(TC01, 5000000);

      Test02.createSession(0);

      Test02.prepare();

      Test02.deleteSession();
      Test02.createSession(0, coral::ReadOnly);

      Test02.run(4);

      Test02.deleteSession();
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
