#include <cstdio>
#include <iostream>
#include <string>
#include "CoralBase/Exception.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "TestEnv/TestEnv.h"
#include "DeleteCascade.h"

int main(int argc, char *argv[]){

  TestEnv TC01("DCASCADE");

  if(TC01.check(argc, argv))
  {
    /* add the default connection strings to the test application */
    TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_ADMIN);

    try
    {
      DeleteCascade dc(TC01);

      dc.createSession(0);
      // test foreign key contraints without cascade
      dc.setup( false );

      dc.test( false );
      // test foreign key contraints with cascade
#ifdef CORAL240DC
      dc.setup( true );

      dc.test( true );
#endif
      dc.deleteSession();
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
