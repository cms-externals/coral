// $Id: test_RingBufferSocket.cpp,v 1.4.2.6 2011/09/16 11:46:49 avalassi Exp $

// Include files
#include <cstring>
#include <iostream>
#include <sys/time.h> // for gettimeof day debug
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralServerBase/hexstring.h"
#include "CoralSockets/GenericSocketException.h"

// Local include files
#include "../../src/RingBufferSocket.h"

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

  class RingBufferSocketTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( RingBufferSocketTest );
    CPPUNIT_TEST( test_RingBuffer );
    CPPUNIT_TEST( test_read_write );
    CPPUNIT_TEST( test_read_after_close );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {}

    // ------------------------------------------------------

    void test_RingBuffer()
    {
      const size_t bufSize=100;
      RingBuffer buffer(bufSize);

      const size_t testSize=200;
      unsigned char sendBuf[ testSize ];
      unsigned char recBuf[ testSize ];

      for (size_t i = 0; i < testSize; i++ )
        sendBuf[i] = (unsigned char) i&0xff;

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 1", (size_t)0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 1", 50,
                                   buffer.write( &sendBuf[0], 50 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 2", (size_t)50,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 1", 50,
                                   buffer.read( &recBuf[0], 50) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 3", (size_t)0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 1", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 50 ) );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 2", 50,
                                   buffer.write( &sendBuf[0], 50 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 4", (size_t)50,
                                   buffer.usedSize() );

      memset(&recBuf[0], 0, 50 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 2", 50,
                                   buffer.read( &recBuf[0], 50) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 5", (size_t)0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 2", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 50 ) );



      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 2a", 50,
                                   buffer.write( &sendBuf[0], 50 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 4a", (size_t)50,
                                   buffer.usedSize() );

      memset(&recBuf[0], 0, 50 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 2a", 50,
                                   buffer.read( &recBuf[0], 50) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 5a", (size_t)0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 2a", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 50 ) );



      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 3", 75,
                                   buffer.write( &sendBuf[0], 75 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 6", (size_t) 75,
                                   buffer.usedSize() );

      memset(&recBuf[0], 0, 75 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 3",  75,
                                   buffer.read( &recBuf[0], 75) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 7", (size_t) 0,
                                   buffer.usedSize() );

      /*
      std::cout << std::endl;
      std::cout << "send " << hexstring( &sendBuf[0], 75 ) << std::endl;
      std::cout << "send " << hexstring( &recBuf[0], 75 ) << std::endl;

      for ( size_t i=0; i< 75; i++)
        if ( sendBuf[i]!=recBuf[i] )
          std::cout << "mismatch at "<< i << std::endl;
      */

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 3", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 75 ) );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 4", 75,
                                   buffer.write( &sendBuf[0], 75 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 8", (size_t) 75,
                                   buffer.usedSize() );

      memset(&recBuf[0], 0, 75 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 5",  75,
                                   buffer.read( &recBuf[0], 75) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 9", (size_t) 0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 4", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 75 ) );



      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 5", 75,
                                   buffer.write( &sendBuf[0], 75 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 10", (size_t) 75,
                                   buffer.usedSize() );

      memset(&recBuf[0], 0, 75 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 6",  75,
                                   buffer.read( &recBuf[0], 75) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 11", (size_t) 0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 5", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 75 ) );


      // now try to over fill the buffer

      size_t written=buffer.write( &sendBuf[0], testSize );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 7", bufSize-1, written);

      // buffer is full, writing should not write anything
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 8", -1,
                                   buffer.write( &sendBuf[0], testSize ) );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 10", bufSize-1,
                                   buffer.usedSize() );

      memset(&recBuf[0], 0, testSize );
      size_t read=buffer.read( &recBuf[0], testSize );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 7", bufSize-1, read);

      // buffer is empty, read should return 0
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 8", -1,
                                   buffer.read( &recBuf[0], testSize) );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 6", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], read ) );

      {
        // check bug fix
        RingBuffer buffer2(100);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size bug", (size_t)0,
                                     buffer2.usedSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write bug", (int)bufSize-1,
                                     buffer2.write( &sendBuf[0], testSize ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size bug", (size_t) bufSize-1,
                                     buffer2.usedSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size bug", (size_t) 0,
                                     buffer2.freeSize() );
      };

    }

    // ------------------------------------------------------

#define largePacketLen 3000000 //3M*4 bytes=12 M bytes

    // sender thread for tests
    class senderClass
    {

      ISocketPtr m_socket;

    public:

      senderClass( ISocketPtr socket)
        : m_socket( socket )
      {};

      void operator()()
      {
        int *sendbuf = new int[ largePacketLen ];
        try
        {
          for ( int i=0; i< largePacketLen; i++)
            sendbuf[i]=i;
          m_socket->writeAll((unsigned char*) sendbuf,
                             largePacketLen *sizeof( sendbuf[0] ) );
        }
        catch (GenericSocketException &e) {
          std::cerr << "Exception in senderClass: " << e.what() << std::endl;
        };
        delete[] sendbuf;
      };

    };

    void test_read_write()
    {
      {
        // establish a connection
        RingBufferPipes pipes( 10000, "test_pipe1");
        ISocketPtr connect=pipes.getSrc();
        ISocketPtr accept=pipes.getDst();
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
        // buffers should be full, so we assume we can read len/10 every time
        CPPUNIT_ASSERT_EQUAL_MESSAGE("read 2", nread, len );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cmp 2 ", memcmp( test, rec, len ), 0 );
      }

      // write/receive very large data sets (this needs threads)
      {
        // establish a connection
        RingBufferPipes pipes( 10000, "test_pipe" );
        ISocketPtr connect=pipes.getSrc();
        ISocketPtr accept=pipes.getDst();;
        senderClass sender( connect );
        std::auto_ptr<boost::thread> sendThread =
          std::auto_ptr<boost::thread>( new boost::thread( sender ));
        int *recbuf= new int[ largePacketLen ];
        memset( (unsigned char*)recbuf, 0,
                largePacketLen * sizeof( recbuf[0] ) );
        accept->readAll( (unsigned char*)recbuf,
                         largePacketLen * sizeof( recbuf[ 0 ] ) );
        for ( int i=0; i< largePacketLen ; i++)
          if ( recbuf[i] != i )
            CPPUNIT_FAIL("Large send buffer mismatch");
        delete[] recbuf;
      };
    };

    void test_read_after_close()
    {
      const size_t bufSize=100;
      RingBuffer buffer(bufSize);

      const size_t testSize=200;
      unsigned char sendBuf[ testSize ];
      unsigned char recBuf[ testSize ];

      for (size_t i = 0; i < testSize; i++ )
        sendBuf[i] = (unsigned char) i&0xff;

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 1", (size_t)0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 1", 50,
                                   buffer.write( &sendBuf[0], 50 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 2", (size_t)50,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 1", 50,
                                   buffer.read( &recBuf[0], 50) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 3", (size_t)0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 1", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 50 ) );


      // test if we can read the contents of the buffer after it
      // is closed
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 2", 50,
                                   buffer.write( &sendBuf[0], 50 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 4", (size_t)50,
                                   buffer.usedSize() );

      buffer.close();

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer read 2", 50,
                                   buffer.read( &recBuf[0], 50) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer size 5", (size_t)0,
                                   buffer.usedSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer cmp 1", 0,
                                   memcmp( &sendBuf[0], &recBuf[0], 50 ) );

      // can't write any more, returns 0 for closed
      CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer write 3", 0,
                                   buffer.write( &sendBuf[0], 50 ) );

    }
  };

  CPPUNIT_TEST_SUITE_REGISTRATION( RingBufferSocketTest );

}

CORALCPPUNITTEST_MAIN( RingBufferSocketTest )
