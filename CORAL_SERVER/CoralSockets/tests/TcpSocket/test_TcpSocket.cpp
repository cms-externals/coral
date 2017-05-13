// Include files
#include <cstring>
#include <iostream>
#include <sys/time.h> // for gettimeof day debug
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralSockets/GenericSocketException.h"
#include "CoralSockets/NonFatalSocketException.h"

// Local include files
#include "../../src/TcpSocket.h"

// port used for testing
int testPort = 50007;

const unsigned char testMessage[] ="0123456789012345789";
const int testMessageLen = sizeof( testMessage ) / sizeof( testMessage[0] );

// Namespace
using namespace coral::CoralSockets;

namespace coral
{

  uint64_t getTimeMS()
  {
    timeval val;
    gettimeofday( &val, 0);
    return (uint64_t) val.tv_usec/1000 + (uint64_t) val.tv_sec*1000;
  }

  class TcpSocketTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( TcpSocketTest );
    CPPUNIT_TEST( test_basic );
    CPPUNIT_TEST( test_listenOn );
    CPPUNIT_TEST( test_ConnectTo );
    CPPUNIT_TEST( test_poll );
    CPPUNIT_TEST( test_read_write );
    CPPUNIT_TEST( test_read_last_packets );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown()
    {

    retry:

      try
      {
        TcpSocket::listenOn( "localhost", testPort );
      }
      catch (GenericSocketException &e)
      {
        std::string expMsg="Error binding socket: Address already in use";
        if ( expMsg == std::string( e.what(), expMsg.size() ) ) {
          testPort+=1;
          if (testPort > 50100)
            throw;
          goto retry;
        }
        else throw;
      };

      //std::cout << "starting teardown sleep" << std::endl;
      //sleep(50); // to make sure old connnections are properly closed
    }

    // ------------------------------------------------------
    void test_basic()
    {
      // some basic connection tests

      TcpSocketPtr listenSocket   = TcpSocket::listenOn( "localhost", testPort );
      TcpSocketPtr connectSocket1 = TcpSocket::connectTo( "localhost",
                                                          testPort );
      TcpSocketPtr acceptSocket1  = listenSocket->accept();

      // write to one end and receive on the other
      connectSocket1->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket1->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 1", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 1", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket1->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=connectSocket1->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 2", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 2", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }

      // establish another connection
      TcpSocketPtr connectSocket2 = TcpSocket::connectTo( "localhost",
                                                          testPort );
      TcpSocketPtr acceptSocket2  = listenSocket->accept();

      // write to one end
      connectSocket2->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket2->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 3", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 3", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket2->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=connectSocket2->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 4", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 4", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }

      // close one end and try to write to other end
      connectSocket1->close();
      sleep(1);
      try {
        int i=0;
        while (i<1000) {
          acceptSocket1->write( testMessage, testMessageLen);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException&) {
        // ok
      };
      // reading should fail as well
      try {
        int i=0;
        while (i<1000) {
          unsigned char receiveBuffer[100];
          acceptSocket1->read( receiveBuffer, 100);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException&) {
        // ok
      };


      // check if other connection is still alive
      // write to one end
      connectSocket2->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket2->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 4", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 4", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket2->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=connectSocket2->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 5", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 5", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }

      // this time we close the other end
      // close one end and try read or write to other end
      acceptSocket2->close();
      sleep(1);
      // reading should fail as well
      try {
        int i=0;
        while (i<1000) {
          unsigned char receiveBuffer[100];
          connectSocket2->read( receiveBuffer, 100);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException&) {
        // ok
      };
      // writing should fail
      try {
        int i=0;
        while (i<1000) {
          connectSocket2->write( testMessage, testMessageLen);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException&) {
        // ok
      };

    }

    // ------------------------------------------------------

    void test_listenOn()
    {
      {
        // create one listening socket
        TcpSocketPtr tmp = TcpSocket::listenOn( "localhost", testPort );
        // creating a second listening socket on this port should fail
        CPPUNIT_ASSERT_THROW( TcpSocket::listenOn( "localhost", testPort ),
                              GenericSocketException );
        tmp.reset();
      };

      {
        // first socket has gone out of scope, we should be able to create
        // a new one
        TcpSocketPtr listen1 = TcpSocket::listenOn( "localhost", testPort );

        // can't read from or write to a listening socket
        CPPUNIT_ASSERT_THROW( listen1->write( testMessage, testMessageLen ),
                              SocketClosedException );
        unsigned char buf[100];
        CPPUNIT_ASSERT_THROW( listen1->read( buf, 100 ),
                              SocketClosedException );
      };

      {
        // some connection creation stress tests
        TcpSocketPtr listen1 = TcpSocket::listenOn( "localhost", testPort );

        {
          // 100 pending connections should be ok
          const int nConnections=100;
          TcpSocketPtr connectTo[ nConnections ];
          for (int i = 0; i<nConnections; i++)
            connectTo[i] = TcpSocket::connectTo( "localhost", testPort );

          TcpSocketPtr accept[ nConnections ];
          for (int i = 0; i<nConnections; i++)
            accept[i] = listen1->accept();
          for (int i = 0; i<nConnections; i++)
            accept[i]->close();
        }
        // no more pending connections
        CPPUNIT_ASSERT_THROW( listen1->accept(), NonFatalSocketException );
        /*
           connect blocks, so this makes not a lot of sense...
          {
            // 10000 pending connections should fail..
            const int nConnections=10000;
            TcpSocketPtr connectTo[ nConnections ];
            for (int i = 0; i<nConnections; i++) {
              std::cout <<"Connection " << i << std::endl;
              connectTo[i] = TcpSocket::connectTo( "localhost", testPort );
            };

            TcpSocketPtr accept[ nConnections ];
            for (int i = 0; i<nConnections; i++)
              accept[i] = listen1->accept();
          }
        */

      };
    };

    // ------------------------------------------------------

    void test_ConnectTo()
    {
      {
        // port is not open = connection refused
        CPPUNIT_ASSERT_THROW( TcpSocket::connectTo( "localhost", testPort ),
                              GenericSocketException );

        // test host resolving
        CPPUNIT_ASSERT_THROW( TcpSocket::connectTo( "invalid host name", testPort),
                              GenericSocketException );
        CPPUNIT_ASSERT_THROW( TcpSocket::connectTo( "127.0.0.1:300", testPort),
                              GenericSocketException );

        // create a connection
        TcpSocketPtr listen = TcpSocket::listenOn( "localhost", testPort );
        TcpSocketPtr connect = TcpSocket::connectTo( "localhost", testPort );
        TcpSocketPtr accept = listen->accept();

        CPPUNIT_ASSERT_THROW( connect->accept(), GenericSocketException );
        CPPUNIT_ASSERT_THROW( accept->accept(), GenericSocketException );
      };
    };

    // ------------------------------------------------------

    void test_poll()
    {
      {
        // establish a connection
        TcpSocketPtr listen=TcpSocket::listenOn( "localhost", testPort );
        TcpSocketPtr connect=TcpSocket::connectTo( "localhost", testPort );
        TcpSocketPtr accept=listen->accept();

        // make sure read queue is empty
        {
          const int len=3000;
          unsigned char buf[len];
          int res=0;
          do {
            res = accept->read(buf,len,100);
          } while ( res == len );
        };
        uint64_t start;
        start=getTimeMS();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("poll 1 ", accept->poll(
                                                             TcpSocket::P_READ, 200 ), false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("timeout 1",  getTimeMS() - start > 150, true );

        connect->write( testMessage, testMessageLen);
        start=getTimeMS();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("poll 2 ", accept->poll(
                                                             TcpSocket::P_READ, 200 ), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("timeout 2",  getTimeMS() - start < 20, true );

        // test write poll
        // write queue is empty, so poll should return immediatly
        start=getTimeMS();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("poll 3", accept->poll(
                                                            TcpSocket::P_WRITE, 200), true );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "timeout 3", getTimeMS() - start < 20, true );

        // now fill the write queue and check write poll
        {
          int res=0;
          const int len=30000;
          unsigned char message[len];
          for (int i = 0; i<len; i++)
            message[i]=(unsigned char) i&0xff;

          uint64_t max_sleep=0;
          uint64_t time;

          for (int i = 0; i<10; i++ ) {
            do {
              res=accept->write( message, len);
            } while ( res == len );

            time=getTimeMS();
            accept->poll( TcpSocket::P_WRITE, 200);
            time=getTimeMS()-time;

            if ( time >max_sleep )
              max_sleep = time;
          };
          CPPUNIT_ASSERT_EQUAL_MESSAGE("timeout 4", max_sleep > 150, true );
        };
        /*
        // this test seems to be unreliable
       // write queue is full, so poll should wait and return false
       start=getTimeMS();
       CPPUNIT_ASSERT_EQUAL_MESSAGE("poll 4", accept->poll(
            TcpSocket::P_WRITE, 200), false );
       CPPUNIT_ASSERT_EQUAL_MESSAGE( "timeout 4", getTimeMS() - start > 150, true );
        */
        accept->close();
        connect->close();
        listen->close();

      }
    };

    // ------------------------------------------------------
#define  largePacketLen 3000000
    //3M*4 bytes=12 M bytes

    // sender thread for tests
    class senderClass {

    public:
      void operator()() {
        int *sendbuf = new int[ largePacketLen ];
        try {
          for ( int i=0; i< largePacketLen; i++)
            sendbuf[i]=i;

          TcpSocketPtr connect=TcpSocket::connectTo("localhost", testPort );

          connect->writeAll((unsigned char*) sendbuf, largePacketLen *sizeof( sendbuf[0] ) );
        }
        catch (GenericSocketException &e) {
          std::cerr << "Exception in senderClass: " << e.what() << std::endl;
        };
        delete[] sendbuf;
      };

    };

    void test_read_write()
    {
      // establish a connection
      TcpSocketPtr listen=TcpSocket::listenOn( "localhost", testPort );
      TcpSocketPtr connect=TcpSocket::connectTo( "localhost", testPort );
      TcpSocketPtr accept=listen->accept();
      uint64_t mTime;
      const int len = 3000;
      unsigned char buf[len];

      // check read timeout
      mTime = getTimeMS();
      int res=accept->read( buf, len, 200);
      mTime = getTimeMS() - mTime;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read timeout 1", res == 0, true);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read timeout 2", mTime > 150, true);

      // FIXME how to test write timeout? Is this necessary?

      // write several small pieces and receive a large chunk
      unsigned char test[ len ];
      for (int i=0; i<len; i++)
        test[i]=(unsigned char) ( i & 0xff);

      for (int i=0; i<10; i++)
        connect->writeAll( &test[ i*len/10 ], len/10 );

      unsigned char rec[ len ];
      int nread = accept->read( rec, len, 100);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read 1", nread, len );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("cmp 1", memcmp( test, rec, len ), 0 );

      // write one big chunk and receive several small ones
      connect->writeAll( &test[0], len );

      nread=0;
      for ( int i = 0; i < 10; i++)
        nread+=accept->read( &rec[ i*len/10 ], len/10, 100 );
      // the buffers should be full, so we assume we can read len/10 every time
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read 2", nread, len );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("cmp 2 ", memcmp( test, rec, len ), 0 );

      // write/receive very large data sets (this needs threads)
      {

        senderClass sender;
        std::auto_ptr<boost::thread> sendThread = std::auto_ptr<boost::thread>(
                                                                               new boost::thread( sender ));

        listen->poll(TcpSocket::P_READ, -1); // wait until the thread has connected
        TcpSocketPtr accept1 = listen->accept();

        int *recbuf= new int[ largePacketLen ];
        memset( (unsigned char*)recbuf, 0, largePacketLen * sizeof( recbuf[0] ) );
        accept1->readAll( (unsigned char*)recbuf, largePacketLen * sizeof( recbuf[ 0 ] ) );

        for ( int i=0; i< largePacketLen ; i++)
          if ( recbuf[i] != i )
            CPPUNIT_FAIL("Large send buffer mismatch");
        delete[] recbuf;
      };


    };

    void test_read_last_packets()
    {
      // establish a connection
      TcpSocketPtr listen=TcpSocket::listenOn( "localhost", testPort );
      TcpSocketPtr connect=TcpSocket::connectTo( "localhost", testPort );
      TcpSocketPtr accept=listen->accept();

      // write to one end and receive on the other
      connect->write( testMessage, testMessageLen );
      {
        unsigned char receiveBuffer[100];
        int nRead=accept->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 1", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 1", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the end  and close immediatly
      connect->write( testMessage, testMessageLen );
      connect->close();
      sleep(1);
      // the last message should still be available on the other end
      {
        unsigned char receiveBuffer[100];
        int nRead=accept->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 2", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 2", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      unsigned char receiveBuffer[100];
      CPPUNIT_ASSERT_THROW( accept->read( receiveBuffer, sizeof( receiveBuffer )
                                          / sizeof( receiveBuffer[0] ), 200 ),
                            SocketClosedException );

    };



    // todo  test send/rec 0 bytes

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( TcpSocketTest );

}

CORALCPPUNITTEST_MAIN( TcpSocketTest )
