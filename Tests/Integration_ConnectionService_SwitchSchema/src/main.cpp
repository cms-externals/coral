#include "TestApp.h"
#include "CoralBase/Exception.h"
#include <iostream>
#include <exception>
#include <cstdlib>


int main( int argc, char *argv[] )
{

  TestApp app("CS_SWITCHSCHEMA");

  if(app.check(argc, argv))
  {
    //need reader but with write permissions
    app.addServiceName(TEST_CORE_SERVICE_3);
    app.addServiceName(TEST_CORE_SCHEME_ADMIN);

    app.addServiceName(TEST_CORE_SERVICE_1);
    app.addServiceName(TEST_CORE_SERVICE_2);
    try
    {
      app.run();
    }
    TESTCORE_FETCHERRORS
      }
  return 1;
}
