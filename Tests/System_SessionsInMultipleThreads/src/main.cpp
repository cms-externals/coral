#include "Reporter.h"
#include "GlobalStatus.h"
#include "TopThread.h"

#include "RelationalAccess/ConnectionService.h"

#include "CoralBase/Exception.h"

#include "CoralBase/boost_thread_headers.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>

#include "TestEnv.h"

int main(int argc, char *argv[])
{
  TestEnv env("SYS_THREADS");
  env.addDBType("oracle");

  if(env.check(argc, argv))
  {

    //create the 11 service names for the 11 threads
    //could be altered in the future
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);
    env.addServiceName(0, TEST_CORE_SERVICE_0);

    try {
      coral::ConnectionService connectionService;
      connectionService.setMessageVerbosityLevel( coral::Warning );
      Reporter reporter;

      GlobalStatus ok;
      std::vector< boost::thread* > threads;
      for ( int i = 0; i < 10; ++i )
        threads.push_back( new boost::thread( TopThread( connectionService, reporter, i + 1, ok, &env ) ) );

      for ( int i = 0; i < 10; ++i )
        threads[i]->join();

      for ( int i = 0; i < 10; ++i ) {
        delete threads[i];
      }

      if ( ! ( ok.isOk() ) )
        throw std::runtime_error( "Test failed!" );
    }
    TESTCORE_FETCHERRORS
      }
  return 1;
}
