// $Id: test_StopTimer.cpp,v 1.3.2.3 2010/12/20 13:04:03 avalassi Exp $
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Include files
#include <sstream>
#include <math.h>

#include "CoralBase/boost_thread_headers.h"

#include "CoralMonitor/StopTimer.h"

// Namespace
using namespace coral;

namespace coral
{

  class testBusyThread {
  public:
    testBusyThread( StopTimer &timer, const std::string & name)
      : m_timer( timer ), m_name( name )
    {};

    void operator() ()
    {
      m_timer.start();

      double result=0;
      for (int i=0; i<10000; i++)
        for (int j=0; j<10000; j++)
          result+=i*j;
      m_result=result;

      m_timer.stop( m_name );
    };

  private:
    StopTimer& m_timer;

    std::string m_name;

    double m_result;
  };

  class StopTimerTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( StopTimerTest );
    CPPUNIT_TEST( test_Real );
    CPPUNIT_TEST( test_User );
    CPPUNIT_TEST( test_Loop );
    CPPUNIT_TEST( test_thread );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {
      enableTimings();
    }

    void tearDown() {}

    // ------------------------------------------------------

    void test_Real()
    {
      StopTimer timer;
      timer.start();
      usleep(500000); // 500ms
      timer.stop("sleep_500ms");
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

    // ------------------------------------------------------

    void test_User()
    {
      StopTimer timer;
      timer.start();
      double result=0;
      for (int i = 0 ; i < 100000; i++)
        for ( int j = 0; j < 10000; j++)
          result = (double)j * (double)i;
      timer.stop("busyloop");
      std::cout << "Real " << timer.getRealTime() << std::endl;
      std::cout << "System " << timer.getSystemTime() << std::endl;
      std::cout << "User " << timer.getUserTime() << std::endl;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("user vs real in busy loop",
                                   true, fabs( timer.getRealTime()-timer.getUserTime() ) <
                                   0.1*timer.getRealTime());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("system time",
                                   true, timer.getSystemTime() <= 0.02 );
      printTimers();
    }

    // ------------------------------------------------------

    void test_Loop()
    {
      StopTimer timer;
      StopTimer innerLocal;
      StopTimer local;
      timer.start();
      double result=0;
      for ( int j = 0; j < 100; j++) {
        local.start();
        for (int i = 0 ; i < 1000; i++) {
          innerLocal.start();
          result+=i*j;
          innerLocal.stop("inner loop time");
        }
        local.stop("loop time");
      };

      timer.stop("start stop loop");
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
      printTimers();
    }

    // ------------------------------------------------------

    void test_thread()
    {
      deleteTimings();
      StopTimer mainTimer;
      StopTimer thread1Timer;
      StopTimer thread2Timer;
      mainTimer.start();
      testBusyThread threadObject1(thread1Timer, "busyThread1");
      testBusyThread threadObject2(thread2Timer, "busyThread2");
      boost::thread thread1( threadObject1 );
      boost::thread thread2( threadObject2 );

      thread1.join();
      thread2.join();
      mainTimer.stop("main thread");

      printTimers();

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


  };

  CPPUNIT_TEST_SUITE_REGISTRATION( StopTimerTest );

}

CORALCPPUNITTEST_MAIN( StopTimerTest )
