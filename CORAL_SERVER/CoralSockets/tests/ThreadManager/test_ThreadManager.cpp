// Include files
#include <iostream>
//#include "CoralMonitor/StatsManager.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Local include files
#include "../../src/ThreadManager.h"

// Namespace
using namespace coral::CoralSockets;

namespace coral
{

  class ThreadTest : public Runnable {
  public:
    ThreadTest( boost::mutex &mutex, int &count )
      : m_mutex( mutex )
      , m_count( count )
    {};

    const std::string desc() const
    {
      return "ThreadTest";
    }

    void endThread()
    {}

    void operator()() {
      boost::mutex::scoped_lock lock( m_mutex );
      m_count++;
    }
  private:
    boost::mutex & m_mutex;
    int & m_count;
  };

  class ThreadManagerTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( ThreadManagerTest );

    CPPUNIT_TEST( test_Basic );

    // Test 20 threads, but disable tests with 100-1000 threads (bug #73032)
    // as they always fail with a resource error. Decreasing the allowed stack
    // size for threads would fix it, but we dont need so many threads (not for
    // ATLAS HLT). In tcsh: limit stacksize 1024 (maybe even lower)
    // Explanation: there is a limit on the total stack size (I guess set by
    // the administrator) limiting the stacksize per thread allows more
    // threads, but risks running out of stack in a thread.
    CPPUNIT_TEST( test_20Threads );
    //CPPUNIT_TEST( test_100Threads );
    //CPPUNIT_TEST( test_1000Threads );

    CPPUNIT_TEST( test_statistics );

    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {
    }


    // ------------------------------------------------------

    void test_Basic()
    {
      ThreadManager manager;
      boost::mutex mutex;
      int count=0;

      manager.addThread( new ThreadTest( mutex, count ) );
      manager.joinAll();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("count 1", 1, count );

      // can't add one class twice
      ThreadTest *test=new ThreadTest( mutex, count );
      manager.addThread( test );
      CPPUNIT_ASSERT_THROW( manager.addThread( test ),
                            GenericSocketException );

      // make sure the threads are properly cleaned up
      CPPUNIT_ASSERT_EQUAL_MESSAGE("existsThread 1",
                                   true, manager.existsThread( test ) );
      manager.joinAll();
      manager.housekeeping();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("existsThread 2",
                                   false, manager.existsThread( test ) );

    }

    // ------------------------------------------------------

    void test_NThreads( int nthreads )
    {
      ThreadManager manager;
      boost::mutex mutex;
      int count=0;
      for ( int i=0; i < nthreads; i++ )
      {
        manager.addThread( new ThreadTest(mutex, count) );
      }
      manager.joinAll();
      // make sure every thread has finished
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "count", nthreads, count );
    }

    void test_20Threads()
    {
      test_NThreads( 20 );
    }

    void test_100Threads()
    {
      test_NThreads( 100 );
    }

    void test_1000Threads()
    {
      test_NThreads( 1000 );
    }

    // ------------------------------------------------------

    void
    test_statistics()
    {
      /*
      std::vector<std::string> list;
      coral::StatsManager::instance().reportStats(list);
      std::cout << std::endl;
      for( std::vector<std::string>::iterator i = list.begin();
           i != list.end(); ++i )
        std::cout << *i << std::endl;
      */
    };

    // ------------------------------------------------------

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( ThreadManagerTest );

}

CORALCPPUNITTEST_MAIN( ThreadManagerTest )
