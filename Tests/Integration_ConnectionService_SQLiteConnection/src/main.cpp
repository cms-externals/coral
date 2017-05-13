#include "TestApp.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>


int main(int argc, char *argv[])
{
  TestApp app("CS_SQLITE_CONNECTION");

  if(app.check(argc, argv))
  {
    app.addServiceName(TEST_CORE_SCHEME_ADMIN);

    try
    {
      app.run();
    }
    TESTCORE_FETCHERRORS
      }
  return 1;
}
