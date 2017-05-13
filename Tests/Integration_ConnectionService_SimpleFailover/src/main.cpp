#include "TestApp.h"
#include "TestEnv/Testing.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>

int main(int argc, char *argv[])
{
  TestApp app("CS_SIMPLE_FAILOVER");

  if(app.check(argc, argv))
  {
    //using the failover service name
    //the servicename should include two different database backends
    //the first is a none exiting one
    //the other is the failover
    app.addServiceName(TEXT_CORE_FAILOVER_SERVICE_0);

    try
    {
      app.runFailover(0);
    }
    TESTCORE_FETCHERRORS
      }
  return 1;
}
