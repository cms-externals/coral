#include "CoralBase/Exception.h"

#include "TestEnv/TestEnvLocked.h"

#include <stdio.h>
#include <string>
#include <iostream>

#include "NetworkFailure.h"
#include "StoneHandler.h"
#include "NetworkFailureThreadStone.h"
#include "NetworkFailureThread01.h"

#include "CoralBase/MessageStream.h"

#define DEBUG( msg ){ coral::MessageStream myMsg("NETFAULT"); myMsg << coral::Debug << msg << coral::MessageStream::endmsg; }

int main(int argc, char *argv[]){

  TestEnvLocked TC01("NETFAULT");

  TC01.setEnv("TNS_ADMIN", "$TESTDIR");

  if(TC01.check(argc, argv))
  {
    /* add the default connection strings to the test application */
    TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_PROXY);

    try
    {

#ifndef _WIN32

      StoneHandler sh;
      sh.activate();

      //NetworkFailure nf(&TC01);

      //nf.createSession(0);

      //nf.setup();

      //nf.deleteSession();


      //nf.createSession(0, coral::ReadOnly);

      //nf.test01();

      //nf.test04();

      //nf.deleteSession();


      NetworkFailureThreadStone stone( sh );

      NetworkFailureThread01 thread01( TC01 );
      NetworkFailureThread01 thread02( TC01 );
      NetworkFailureThread01 thread03( TC01 );
      NetworkFailureThread01 thread04( TC01 );

      //thread01.

      boost::thread bthread01( stone );
      boost::thread bthread02( thread01 );
      boost::thread bthread03( thread02 );
      boost::thread bthread04( thread03 );
      boost::thread bthread05( thread04 );

      bthread01.join();
      bthread02.join();
      bthread03.join();
      bthread04.join();
      bthread05.join();


#endif

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
