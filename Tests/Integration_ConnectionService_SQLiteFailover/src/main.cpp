#include "TestApp.h"
#include "TestEnv/Testing.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>


int main(int argc, char *argv[])
{
  TestApp app("CS_SQLITE_FAILOVER");

  if(app.check(argc, argv))
  {
    app.addServiceName(TEST_CORE_SCHEME_ADMIN);
    app.addServiceName(TEST_CORE_SCHEME_ADMIN);

    try
    {
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
