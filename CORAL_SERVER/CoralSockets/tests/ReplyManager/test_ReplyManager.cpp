// $Id: test_ReplyManager.cpp,v 1.8.2.3 2010/09/30 10:37:13 avalassi Exp $

#include <iostream>

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Local include files
#include "../../CoralSockets/GenericSocketException.h"
#include "../../src/ReplyManager.h"
#include "../../src/ThreadManager.h"



// Namespace
using namespace coral::CoralSockets;


namespace coral
{

  class ReplyManagerTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( ReplyManagerTest );
    CPPUNIT_TEST( test_ReplySlot );
    CPPUNIT_TEST( test_ReplyIterator );
    CPPUNIT_TEST( test_ReplyIteratorTimeOut );
    CPPUNIT_TEST( test_ReplyIteratorClosed );
    CPPUNIT_TEST( test_ReplyIteratorClosedBeforeLastReply );
    CPPUNIT_TEST( test_ReplyManager );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {
    }

    typedef std::unique_ptr<ByteBuffer> BufferPtr;
    typedef boost::shared_ptr<ByteBuffer> BufferSPtr;

    static BufferPtr createBuffer( int num ) {
      BufferPtr buf( new ByteBuffer( sizeof( int ) ) );
      *((int*)buf->data()) = num;
      buf->setUsedSize( sizeof( int ) );
      return buf;
    };

    static int getBufferNum( BufferSPtr buf ) {
      return *(int*)buf->data();
    };

    static int getBufferNum( const ByteBuffer& buf ) {
      return *(int*)buf.data();
    };

    // ------------------------------------------------------
    void test_ReplySlot()
    {
      ReplySlot slot;

      // can't append messages befor taking the slot
      std::unique_ptr<ByteBuffer> buffer( new ByteBuffer(10) );
      CPPUNIT_ASSERT_THROW( slot.appendReply( buffer, 0, false ),
                            GenericSocketException );

      slot.takeSlot( 1 );

      // can't take a slot which is already taken
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "take slot 1", slot.takeSlot( 2 ), false );

      // check that messages put into the slot, can be fetched
      // on the other side in the correct order
      slot.appendReply( createBuffer( 10 ), 0, false );
      bool lastReply=false;
      BufferSPtr buf = slot.nextReply( lastReply );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 nextReply buf",
                                    getBufferNum( buf ), 10 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 nextReply last",
                                    lastReply, false );

      // wrong segment number will throw
      CPPUNIT_ASSERT_THROW( slot.appendReply( createBuffer( 15 ), 0, false ),
                            SegmentationErrorException);

      slot.appendReply( createBuffer( 20 ), 1, false );
      slot.appendReply( createBuffer( 30 ), 2, false );
      slot.appendReply( createBuffer( 40 ), 3, false );
      slot.appendReply( createBuffer( 50 ), 4, true );

      buf = slot.nextReply( lastReply );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "2 nextReply buf",
                                    getBufferNum( buf), 20 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "2 nextReply last",
                                    lastReply, false );

      buf = slot.nextReply( lastReply );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "3 nextReply buf",
                                    getBufferNum( buf), 30 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "3 nextReply last",
                                    lastReply, false );

      buf = slot.nextReply( lastReply );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "4 nextReply buf",
                                    getBufferNum( buf), 40  );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "4 nextReply last",
                                    lastReply, false );

      buf = slot.nextReply( lastReply );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "5 nextReply buf",
                                    getBufferNum( buf ), 50 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "5 nextReply last",
                                    lastReply, true );

      // can't append to an already closed slot
      CPPUNIT_ASSERT_THROW( slot.appendReply( createBuffer( 50 ), 5, true ),
                            GenericSocketException );

      // can't append to an freed slot
      slot.freeSlot();
      CPPUNIT_ASSERT_THROW( slot.appendReply( createBuffer( 50 ), 6, true ),
                            GenericSocketException );
    }

    // ------------------------------------------------------
    void test_ReplyIterator()
    {
      ReplySlot slot;

      // take slot, create iterator for it
      slot.takeSlot( 1 );
      {
        SocketReplyIterator it( slot );

        // fill the slot
        slot.appendReply( createBuffer( 10 ), 0, false );
        slot.appendReply( createBuffer( 20 ), 1, false );
        slot.appendReply( createBuffer( 30 ), 2, false );
        slot.appendReply( createBuffer( 40 ), 3, false );
        slot.appendReply( createBuffer( 50 ), 4, true );

        // before the first call to next(), the iterator should throw
        CPPUNIT_ASSERT_THROW( it.currentBuffer(),
                              GenericSocketException );
        CPPUNIT_ASSERT_THROW( it.isLast(),
                              GenericSocketException );

        // use the iterator to read from it
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 next", it.next(), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 isLast", it.isLast(), false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 currentBuffer",
                                      getBufferNum( it.currentBuffer() ), 10 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "2 next", it.next(), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "2 isLast", it.isLast(), false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "2 currentBuffer",
                                      getBufferNum( it.currentBuffer() ), 20 );


        CPPUNIT_ASSERT_EQUAL_MESSAGE( "3 next", it.next(), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "3 isLast", it.isLast(), false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "3 currentBuffer",
                                      getBufferNum( it.currentBuffer() ), 30 );


        CPPUNIT_ASSERT_EQUAL_MESSAGE( "4 next", it.next(), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "4 isLast", it.isLast(), false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "4 currentBuffer",
                                      getBufferNum( it.currentBuffer() ), 40 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "5 next", it.next(), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "5 isLast", it.isLast(), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "5 currentBuffer",
                                      getBufferNum( it.currentBuffer() ), 50 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "6 next", it.next(), false );

        // after next() returned false, the iterator should throw
        CPPUNIT_ASSERT_THROW( it.currentBuffer(),
                              GenericSocketException );
        CPPUNIT_ASSERT_THROW( it.isLast(),
                              GenericSocketException );
      }
      CPPUNIT_ASSERT_EQUAL_MESSAGE("iterator used 2",
                                   0, slot.used() );

    }


    // ------------------------------------------------------

    void test_ReplyIteratorTimeOut()
    {
      ReplySlot slot;

      // take slot, create iterator for it
      slot.takeSlot( 1 );
      SocketReplyIterator it( slot );

      // set the timeout to 10 seconds
      ReplySlot::timeout=10;

      // fill the slot, but don't append a final message
      slot.appendReply( createBuffer( 10 ), 0, false );

      // use the iterator to read from it
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 next", it.next(), true );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 isLast", it.isLast(), false );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "1 currentBuffer",
                                    getBufferNum( it.currentBuffer() ), 10 );
      try {
        it.next(); // this should time out
        CPPUNIT_FAIL("time out exception expected");
      }
      catch( GenericSocketException & e) {
        std::string expMsg="reply timed out";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    }

    // ------------------------------------------------------

    void test_ReplyIteratorClosed()
    {
      ReplySlot slot;

      // take slot, create iterator for it
      slot.takeSlot( 1 );
      SocketReplyIterator it( slot );

      // set the timeout to 10 seconds
      ReplySlot::timeout=10;

      // fill the slot, but don't append a final message
      slot.appendReply( createBuffer( 10 ), 0, false );

      // mark as connection closed
      slot.setConnectionClosed();

      try {
        it.next();
        CPPUNIT_FAIL("connection closed exception expected");
      }
      catch( GenericSocketException & e) {
        std::string expMsg="connection closed";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }

      // reset time out
      ReplySlot::timeout=30*60;
    }

    // ------------------------------------------------------

    void test_ReplyIteratorClosedBeforeLastReply( )
    {
      {
        ReplySlot slot;

        // take slot, create iterator for it
        slot.takeSlot( 1 );
        SocketReplyIterator *it = new SocketReplyIterator( slot );

        // fill the slot, but don't append a final message
        slot.appendReply( createBuffer( 10 ), 0, false );

        // delete the iterator before the final message arrived
        delete it;
        it = 0;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("iterator closed",
                                     ReplySlot::ITERATOR_CLOSED, slot.m_status);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("iterator used 1",
                                     1, slot.used() );

        // final message arrives, no exception should be thrown
        slot.appendReply( createBuffer( 20 ), 1, true );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("slot free",
                                     ReplySlot::OK, slot.m_status );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("iterator used 2",
                                     0, slot.used() );
      }
      {
        ReplySlot slot;

        // take slot, create iterator for it
        slot.takeSlot( 1 );
        SocketReplyIterator *it = new SocketReplyIterator( slot );

        // fill the slot with final message
        slot.appendReply( createBuffer( 10 ), 0, true );

        // delete the iterator before the final message arrived
        delete it;
        it = 0;

        // slot should be freed
        CPPUNIT_ASSERT_EQUAL_MESSAGE("slot free",
                                     ReplySlot::OK, slot.m_status );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("iterator used 3",
                                     0, slot.used() );
      }
    }

    // ------------------------------------------------------
#define MSG_COUNT 100

#if 1
    // reply consumer thread for tests
    class ReplyConsumerClass : public Runnable {
    private:
      boost::shared_ptr<SocketReplyIterator> m_it;
      bool m_allOk;
      bool m_finished;
      boost::mutex &m_mutex;
      bool &m_result;

    public:

      const std::string desc() const
      {
        return "ReplyConsumerClass";
      }

      ReplyConsumerClass( boost::shared_ptr<SocketReplyIterator> it,
                          boost::mutex &mutex, bool &result )
        : m_it( it )
        , m_allOk( true )
        , m_finished( false )
        , m_mutex( mutex )
        , m_result( result )
      {
      };

      void endThread()
      {};

      void operator()() {

        int i = 0;
        while (m_it->next() ) {
          if ( getBufferNum( m_it->currentBuffer() ) != i*20 ) {
            m_allOk=false;
            m_finished=true;
            std::cout << "Mismatch at " << i << " got "
                      << getBufferNum( m_it->currentBuffer() ) << std::endl;
            return;
          };
          i++;
        }
        if ( i!= MSG_COUNT+1 ) {
          std::cout << "Mismatch at " << i << " expected "
                    << MSG_COUNT+1 << std::endl;
          m_allOk=false;
        };

        m_finished=true;
        boost::mutex::scoped_lock lock( m_mutex );
        m_result = m_result && m_allOk;
      };

      bool allOk() {
        return m_finished && m_allOk;
      };
    };
#endif

#if 1

    // reply creator thread for tests
    class ReplyCreatorClass : public Runnable {

    public:

      ReplyCreatorClass( ReplyManager& manager,
                         boost::mutex& mutex,
                         uint32_t requestID,
                         int sleep )
        : m_mutex( mutex )
        , m_manager( manager )
        , m_requestID( requestID )
        , m_sleep( sleep )
      {};

      const std::string desc() const
      {
        return "ReplyCreatorClass";
      }

      void endThread()
      {};

      void operator()()
      {
        for ( int i = 0; i<MSG_COUNT; i++ )
        {
          if ( m_sleep != 0 ) usleep( m_sleep );
          {
            boost::mutex::scoped_lock lock( m_mutex);
            ReplySlot &slot=m_manager.findSlot( m_requestID );
            slot.appendReply( createBuffer( i*20 ), i, false );
          }
        };

        if ( m_sleep != 0 ) usleep( m_sleep );
        {
          boost::mutex::scoped_lock lock( m_mutex);
          ReplySlot &slot=m_manager.findSlot( m_requestID );
          slot.appendReply( createBuffer( MSG_COUNT*20 ), MSG_COUNT, true );
        }
      };

    private:

      boost::mutex& m_mutex;
      ReplyManager& m_manager;
      uint32_t m_requestID;
      int m_sleep;

    };

#endif

    // ------------------------------------------------------

    void test_ReplyManager()
    {
      // test with one creator and one consumer
      {
        boost::mutex creatorMutex;

        boost::mutex resultMutex;
        bool result=true;

        ReplyManager manager(1000);
        ThreadManager thrManager;

        ReplySlot &slot=manager.requestSlot( 1001 );

        thrManager.addThread( new ReplyConsumerClass(
                                                     boost::shared_ptr<SocketReplyIterator>(
                                                                                            new SocketReplyIterator( slot ) ), resultMutex, result ) );

        thrManager.addThread( new ReplyCreatorClass( manager, creatorMutex,
                                                     1001, 0 ) );

        // wait until threads are finished
        thrManager.joinAll();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("ReplyManager 1", result, true);
      };

      // test with several creators and consumers
      // this test is not deterministic!
      {
        boost::mutex creatorMutex;

        boost::mutex resultMutex;
        bool result=true;

        ReplyManager manager(1000);
        ThreadManager thrManager;

        const int nThreads=20;

        for ( int i = 0; i < nThreads; i++ ) {
          ReplySlot &slot=manager.requestSlot( i );

          thrManager.addThread( new ReplyConsumerClass(
                                                       boost::shared_ptr<SocketReplyIterator>(
                                                                                              new SocketReplyIterator( slot ) ), resultMutex, result ) );

          thrManager.addThread( new ReplyCreatorClass( manager, creatorMutex,
                                                       i, 0*(nThreads-i/10)* 1000 ) );
        };

        // wait until threads are finished
        thrManager.joinAll();


        CPPUNIT_ASSERT_EQUAL_MESSAGE("ReplyManager 2", result, true);
      };
    };

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( ReplyManagerTest );
}

CORALCPPUNITTEST_MAIN( ReplyManagerTest )
