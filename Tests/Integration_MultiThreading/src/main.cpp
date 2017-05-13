#include "CoralBase/Exception.h"
#include "TestEnv/TestEnv.h"
#include "CoralBase/MessageStream.h"

// Test includes
#include "ThreadApp.h"
#include "SingleThreadWrite.h"
#include "SingleThreadRead.h"

#include <stdio.h>
#include <string>
#include <iostream>

#define LOG( msg ){ coral::MessageStream myMsg("MTHREAD"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

int main(int argc, char *argv[]){
  /* create new test environment */
  TestEnv TC01("MTHREAD");
  /* check arguments */
  if(TC01.check(argc, argv))
  {
    /* add the default connection strings to the test application */
    TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_ADMIN);
    /* start with the tests */
    try
    {
      ThreadApp thread(TC01);

      LOG("Starting[write] one session with multithreaded requests");

      thread.createSession(0);
      thread.write();
      thread.deleteSession();

      LOG("Starting[read] one session with multithreaded requests");

      thread.createSession(0, coral::ReadOnly);
      thread.read();
      thread.deleteSession();

      LOG("Starting[write] one session per thread");
      // multithreaded sessions write
      {
        std::vector< SingleThreadWrite* > threadBodies;

        for ( int i = 0; i < 10; ++i )
        {
          SingleThreadWrite* swrite = new SingleThreadWrite(TC01, i);
          threadBodies.push_back( swrite );
        }

        std::vector< boost::thread* > threads;
        for ( int i = 0; i < 10; ++i )
          threads.push_back( new boost::thread( *( threadBodies[i] ) ) );

        for ( int i = 0; i < 10; ++i )
          threads[i]->join();

        for ( int i = 0; i < 10; ++i )
        {
          delete threads[i];

          SingleThreadWrite* swrite = threadBodies[i];
          delete swrite;
        }
      }
      LOG("Starting[read] one session per thread");
      // multithreaded sessions read
      {
        std::vector< SingleThreadRead* > threadBodies;

        for ( int i = 0; i < 10; ++i )
        {
          SingleThreadRead* sread = new SingleThreadRead(TC01, i);
          threadBodies.push_back( sread );
        }

        std::vector< boost::thread* > threads;
        for ( int i = 0; i < 10; ++i )
          threads.push_back( new boost::thread( *( threadBodies[i] ) ) );

        for ( int i = 0; i < 10; ++i )
          threads[i]->join();

        for ( int i = 0; i < 10; ++i )
        {
          delete threads[i];

          SingleThreadRead* sread = threadBodies[i];
          delete sread;
        }
      }
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
