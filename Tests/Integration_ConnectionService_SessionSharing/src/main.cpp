#include "TestApp.h"
#include "TestEnv/Testing.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>


int main( int argc, char *argv[] )
{
  TestApp app("CS_SESSIONSHARING");

  if(app.check(argc, argv))
  {
    app.addServiceName(TEST_CORE_SCHEME_ADMIN);
    app.addServiceName(TEST_CORE_SCHEME_ADMIN);

    app.addServiceName(TEST_CORE_DEVSERVICE_0);
    app.addServiceName(TEST_CORE_DEVSERVICE_1);

    try
    {
      Testing tests(&app);
      tests.createSession(3);
      tests.createSimpleTable();
      tests.deleteSession();

      app.run();
    }
    TESTCORE_FETCHERRORS
      }
  return 1;
}
