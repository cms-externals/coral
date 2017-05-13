// $Id: test_DummyRequestHandler.cpp,v 1.2.2.4 2011/09/16 16:23:10 avalassi Exp $

// Include files
#include <iostream>
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralSockets/GenericSocketException.h"

// Local include files
#include "../../src/DummyRequestHandler.h"

// Namespace
using namespace coral::CoralSockets;

namespace coral
{

  class DummyRequestHandlerTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( DummyRequestHandlerTest );
    CPPUNIT_TEST( test_SimpleReplyIterator );
    CPPUNIT_TEST( test_DummyRequestHandler );
    CPPUNIT_TEST( test_DummyRequestHandler_MultiReply );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {}

    // ---------------------------------------------------

    // helpers
    static boost::shared_ptr<ByteBuffer> createBuffer( int num )
    {
      boost::shared_ptr<ByteBuffer> buf( new ByteBuffer( sizeof( int ) ) );
      *((int*)buf->data()) = num;
      return buf;
    };

    static int getBufferNum( boost::shared_ptr<ByteBuffer> buf )
    {
      return *(int*)buf->data();
    };

    static int getBufferNum( const ByteBuffer& buf ) {
      return *(int*)buf.data();
    };

    boost::shared_ptr<ByteBuffer> string2ByteBuffer( std::string str )
    {
      boost::shared_ptr<ByteBuffer> result( new ByteBuffer( str.length() ) );
      const char * cstr=str.c_str();
      for (unsigned int i=0; i<str.length(); i++ )
        *(result->data()+i) = *(cstr+i);
      result->setUsedSize( str.length() );
      return result;
    }

    std::string byteBuffer2String( const ByteBuffer& buf ) {
      return std::string( (char*)buf.data(), buf.usedSize() );
    };

    // ------------------------------------------------------
    void test_SimpleReplyIterator()
    {
      {
        SimpleReplyIterator it( createBuffer( 1 ), false );

        // next() throws before it received the last message
        CPPUNIT_ASSERT_THROW( it.next(),
                              GenericSocketException );

        it.addBuffer( createBuffer( 2 ), false );
        it.addBuffer( createBuffer( 3 ), true );

        // after inserting a last buffer, no buffers are accepted any more
        CPPUNIT_ASSERT_THROW( it.addBuffer( createBuffer( 4 ), true ),
                              GenericSocketException );

        // currentBuffer throws before calling next()
        CPPUNIT_ASSERT_THROW( it.currentBuffer(),
                              GenericSocketException );
        CPPUNIT_ASSERT_THROW( it.isLast(),
                              GenericSocketException );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("next 1",
                                     true, it.next() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer 1",
                                     1, getBufferNum( it.currentBuffer() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("isLast 1",
                                     false, it.isLast() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("next 2",
                                     true, it.next() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer 2",
                                     2, getBufferNum( it.currentBuffer() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("isLast 2",
                                     false, it.isLast() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("next 3",
                                     true, it.next() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer 3",
                                     3, getBufferNum( it.currentBuffer() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("isLast 3",
                                     true, it.isLast() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("next 4",
                                     false, it.next() );

        // currentBuffer throws after next() return false
        CPPUNIT_ASSERT_THROW( it.currentBuffer(),
                              GenericSocketException );
        CPPUNIT_ASSERT_THROW( it.isLast(),
                              GenericSocketException );
      }
    }

    // ------------------------------------------------------

    void test_DummyRequestHandler()
    {
      DummyRequestHandler handler;

      boost::shared_ptr<ByteBuffer> buf =
        string2ByteBuffer("This is a request");

      IByteBufferIteratorPtr it=handler.replyToRequest( *buf );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 1 next",
                                   it->next(), true );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 1 buffer",
                                   byteBuffer2String( it->currentBuffer() ),
                                   std::string( "Thank you for your request 'This is a request") );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 1 isLast",
                                   it->isLast(), true );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 1 next last",
                                   it->next(), false );
    }

    // ------------------------------------------------------

    void test_DummyRequestHandler_MultiReply()
    {
      DummyRequestHandler handler;

      boost::shared_ptr<ByteBuffer> buf =
        string2ByteBuffer("copy 5 This is a request");

      IByteBufferIteratorPtr it=handler.replyToRequest( *buf );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 1 next",
                                   it->next(), true );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 1 buffer",
                                   byteBuffer2String( it->currentBuffer() ),
                                   std::string( "Thank you for your request 0 'copy 5 This is a request") );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 1 isLast",
                                   it->isLast(), false );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 2 next",
                                   it->next(), true );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 2 buffer",
                                   byteBuffer2String( it->currentBuffer() ),
                                   std::string( "Thank you for your request 1 'copy 5 This is a request") );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 2 isLast",
                                   it->isLast(), false );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 3 next",
                                   it->next(), true );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 3 buffer",
                                   byteBuffer2String( it->currentBuffer() ),
                                   std::string( "Thank you for your request 2 'copy 5 This is a request") );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 3 isLast",
                                   it->isLast(), false );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 4 next",
                                   it->next(), true );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 4 buffer",
                                   byteBuffer2String( it->currentBuffer() ),
                                   std::string( "Thank you for your request 3 'copy 5 This is a request") );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 4 isLast",
                                   it->isLast(), false );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 5 next",
                                   it->next(), true );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 5 buffer",
                                   byteBuffer2String( it->currentBuffer() ),
                                   std::string( "Thank you for your request 4 'copy 5 This is a request") );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 5 isLast",
                                   it->isLast(), true );


      CPPUNIT_ASSERT_EQUAL_MESSAGE("reply 6 next last",
                                   it->next(), false );
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( DummyRequestHandlerTest );

}

CORALCPPUNITTEST_MAIN( DummyRequestHandlerTest )
