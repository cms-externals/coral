#include "TestApp.h"
#include "TestEnv/Testing.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>


int main( int argc, char *argv[] )
{
  //initialize the app
  TestApp app("CS_CLEANUPPOOL");
  //check environment parameters to set db backend or display
  if(app.check(argc, argv))
  {
    try
    {
      app.addServiceName(TEST_CORE_SCHEME_ADMIN);

      Testing tests(&app);
      tests.createSession(0);
      tests.createSimpleTable();
      tests.deleteSession();


      app.run();
    }
    TESTCORE_FETCHERRORS
      }
  return 1;
}
