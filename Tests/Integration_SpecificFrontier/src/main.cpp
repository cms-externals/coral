int main( int, char** )
{
  return 0;
}

/*
#include "CoralBase/Exception.h"

#include "TestEnv/TestEnv.h"
#include "TestEnv/Testing.h"

#include <stdio.h>
#include <string>
#include <iostream>

#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("SPECFRONTIER"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

// Use local namespace to avoid name conflicts
namespace {

  class SpecificFrontierTest : public Testing
  {

  public:

    SpecificFrontierTest( const TestEnv& env )
    : Testing(env)
    {
    }

    ~SpecificFrontierTest()
    {
    }

    void setup()
    {

    }

  };
}

int main(int argc, char *argv[])
{
  TestEnv TC01("SPECFRONTIER");

  if(TC01.check(argc, argv))
  {
    // add the default connection strings to the test application
    TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_PROXY);

    try
    {

      SpecificFrontierTest test( TC01 );

      test.createSession(0);

      test.setup();

      test.deleteSession();

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
*/
