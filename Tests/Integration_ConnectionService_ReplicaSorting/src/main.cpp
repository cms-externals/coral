#include "TestApp.h"
#include "TestEnv/Testing.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>


int main( int argc, char *argv[] )
{
  TestApp app("CS_REPLICASORTING");

  if(app.check(argc, argv))
  {
    app.addServiceName(TEST_CORE_SCHEME_ADMIN);

    app.addServiceName(TEST_CORE_REPLICA_SERVICE_3);
    app.addServiceName(TEST_CORE_REPLICA_SERVICE_4);
    app.addServiceName(TEST_CORE_REPLICA_SERVICE_1);

    try
    {
      //create table for replica3
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
