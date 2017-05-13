// $Id: test_StopTimer.cpp,v 1.2.2.5 2010/12/20 13:04:02 avalassi Exp $
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Include files
#include <math.h>
#include <sstream>
#include <stdlib.h> // For setenv and _putenv
#include "CoralBase/boost_thread_headers.h"
#include "CoralMonitor/StopTimer.h"

//-----------------------------------------------------------------------------

namespace coral
{

  // Helper class for the test
  class testBusyThread
  {
  public:
    testBusyThread( StopTimer& timer )
      : m_timer( timer )
      , m_result( 0 )
    {}
    void operator() ()
    {
      m_timer.start();
      double result=0;
      for (int i=0; i<10000; i++)
        for (int j=0; j<10000; j++)
          result+=i*j;
      m_result=result;
      m_timer.stop();
    }
  private:
    StopTimer& m_timer;
    double m_result;
  };

  // Forward declare the test (for easier indentation)
  class StopTimerTest;
}

//-----------------------------------------------------------------------------

// The test class
class coral::StopTimerTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( StopTimerTest );
  CPPUNIT_TEST( test_Real );
  CPPUNIT_TEST( test_User );
  CPPUNIT_TEST( test_Loop );
  CPPUNIT_TEST( test_thread );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_Real()
  {
    StopTimer timer;
    timer.start();
    usleep(500000); // 500ms
    timer.stop();
    std::cout << "Real " << timer.getRealTime() << std::endl;
    std::cout << "System " << timer.getSystemTime() << std::endl;
    std::cout << "User " << timer.getUserTime() << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("system time",
                                 true, timer.getSystemTime() < 0.10 );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("user time",
                                 true, timer.getUserTime() < 0.01 );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("real time",
                                 true, fabs( timer.getRealTime()-0.5 ) < 0.1 );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_User()
  {
    StopTimer timer;
    timer.start();
    double result=0;
    for (int i = 1 ; i <= 100000; i++)
      for ( int j = 1; j <= 10000; j++)
        result = (double)j * (double)i;

    timer.stop();
    std::cout << "#loops: " << result << std::endl;
    std::cout << "Real " << timer.getRealTime() << std::endl;
    std::cout << "System " << timer.getSystemTime() << std::endl;
    std::cout << "User " << timer.getUserTime() << std::endl;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "user vs real in busy loop",
                                  true, fabs( timer.getRealTime() - timer.getUserTime() ) < 0.3 * timer.getRealTime() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "system time",
                                  true, timer.getSystemTime() <= 0.02 );
    //printTimers();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_Loop()
  {
    StopTimer timer;
    StopTimer innerLocal;
    StopTimer local;
    timer.start();
    double result=0;
    for ( int j = 0; j < 100; j++)
    {
      local.start();
      for (int i = 0 ; i < 1000; i++)
      {
        innerLocal.start();
        result+=i*j;
        innerLocal.stop();
      }
      local.stop();
    }
    timer.stop();
    /*
      std::cout << "Real " << timer.getRealTime() << std::endl;
      std::cout << "System " << timer.getSystemTime() << std::endl;
      std::cout << "User " << timer.getUserTime() << std::endl;
    */
#if 0
    CPPUNIT_ASSERT_EQUAL_MESSAGE("user vs real in busy loop",
                                 true, fabs( timer.getRealTime()-timer.getUserTime() ) <
                                 0.1*timer.getRealTime());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("system time",
                                 true, timer.getSystemTime() < 0.02 );
#endif
    //printTimers();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_thread()
  {
    //deleteTimings(); // AV: was this really needed?
    StopTimer mainTimer;
    StopTimer thread1Timer;
    StopTimer thread2Timer;
    mainTimer.start();
    testBusyThread threadObject1(thread1Timer);
    testBusyThread threadObject2(thread2Timer);
    boost::thread thread1( threadObject1 );
    boost::thread thread2( threadObject2 );

    thread1.join();
    thread2.join();
    mainTimer.stop();

    //printTimers();

    // thread should have used the same user and time
    CPPUNIT_ASSERT_EQUAL_MESSAGE("user thread1 vs thread2",
                                 true, fabs( thread1Timer.getUserTime()-thread2Timer.getUserTime() )
                                 <=
                                 0.1*thread1Timer.getUserTime()+0.01);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("system thread1 vs thread2",
                                 true, fabs( thread1Timer.getSystemTime()
                                             - thread2Timer.getSystemTime() )
                                 <=
                                 0.1*thread1Timer.getSystemTime()+0.01);

    // main thread should not have spend much user and system time
    CPPUNIT_ASSERT_EQUAL_MESSAGE("system main thread time",
                                 true, mainTimer.getSystemTime() <= 0.02 );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("system main thread time",
                                 true, mainTimer.getUserTime() <= 0.02 );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void setUp()
  {
    //enableTimings();
    setenv( "CORALSERVER_TIMING", "yes", 1 );
  }

  void tearDown() {}

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::StopTimerTest );

CORALCPPUNITTEST_MAIN( StopTimerTest )
