// $Id: test_CALOpcode.cpp,v 1.2.2.2 2010/12/20 09:10:10 avalassi Exp $
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Include files
#include <sstream>
#include "CoralServerBase/CALOpcode.h"
#include "CoralServerBase/CALOpcodeException.h"

// Namespace
using namespace coral;

namespace coral
{

  class CALOpcodeTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( CALOpcodeTest );
    CPPUNIT_TEST( test_isReserved );
    CPPUNIT_TEST( test_isCALRequest );
    CPPUNIT_TEST( test_isCALReplyOK );
    CPPUNIT_TEST( test_isCALReplyException );
    CPPUNIT_TEST( test_isCALReply );
    CPPUNIT_TEST( test_getCALRequest );
    CPPUNIT_TEST( test_getCALReplyOK );
    CPPUNIT_TEST( test_getCALReplyException );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {}

    // ------------------------------------------------------

    void test_isReserved()
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub1", false,
          CALOpcodes::isReserved( CALOpcodes::ConnectRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub2", false,
          CALOpcodes::isReserved( CALOpcodes::ConnectRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub3", false,
          CALOpcodes::isReserved( CALOpcodes::Disconnect ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub4", false,
          CALOpcodes::isReserved( CALOpcodes::StartTransactionRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub5", false,
          CALOpcodes::isReserved( CALOpcodes::StartTransactionRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub6", false,
          CALOpcodes::isReserved( CALOpcodes::CommitTransaction ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub7", false,
          CALOpcodes::isReserved( CALOpcodes::RollbackTransaction ) );

      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        bool exp = ( i >= 192 ); // 0xC0 <= code
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(), exp, CALOpcodes::isReserved( i ) );
      }

    }

    // ------------------------------------------------------

    void test_isCALRequest()
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub1", true,
          CALOpcodes::isCALRequest( CALOpcodes::ConnectRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub2", true,
          CALOpcodes::isCALRequest( CALOpcodes::ConnectRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub3", true,
          CALOpcodes::isCALRequest( CALOpcodes::Disconnect ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub4", true,
          CALOpcodes::isCALRequest( CALOpcodes::StartTransactionRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub5", true,
          CALOpcodes::isCALRequest( CALOpcodes::StartTransactionRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub6", true,
          CALOpcodes::isCALRequest( CALOpcodes::CommitTransaction ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub7", true,
          CALOpcodes::isCALRequest( CALOpcodes::RollbackTransaction ) );

      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        bool exp = ( i < 64 ); // code < 0x40
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(), exp, CALOpcodes::isCALRequest( i ) );
      }

    }

    // ------------------------------------------------------

    void test_isCALReplyOK()
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub1", false,
          CALOpcodes::isCALReplyOK( CALOpcodes::ConnectRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub2", false,
          CALOpcodes::isCALReplyOK( CALOpcodes::ConnectRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub3", false,
          CALOpcodes::isCALReplyOK( CALOpcodes::Disconnect ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub4", false,
          CALOpcodes::isCALReplyOK( CALOpcodes::StartTransactionRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub5", false,
          CALOpcodes::isCALReplyOK( CALOpcodes::StartTransactionRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub6", false,
          CALOpcodes::isCALReplyOK( CALOpcodes::CommitTransaction ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub7", false,
          CALOpcodes::isCALReplyOK( CALOpcodes::RollbackTransaction ) );

      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        bool exp = ( i >= 64 && i< 128 ); // 0x40 <= code < 0x80
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(), exp, CALOpcodes::isCALReplyOK( i ) );
      }

    }

    // ------------------------------------------------------

    void test_isCALReplyException()
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub1", false,
          CALOpcodes::isCALReplyException( CALOpcodes::ConnectRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub2", false,
          CALOpcodes::isCALReplyException( CALOpcodes::ConnectRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub3", false,
          CALOpcodes::isCALReplyException( CALOpcodes::Disconnect ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub4", false,
          CALOpcodes::isCALReplyException( CALOpcodes::StartTransactionRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub5", false,
          CALOpcodes::isCALReplyException( CALOpcodes::StartTransactionRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub6", false,
          CALOpcodes::isCALReplyException( CALOpcodes::CommitTransaction ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub7", false,
          CALOpcodes::isCALReplyException( CALOpcodes::RollbackTransaction ) );

      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        bool exp = ( i >= 128 && i< 192 ); // 0x80 <= code < 0xC0
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(), exp, CALOpcodes::isCALReplyException( i ) );
      }

    }

    // ------------------------------------------------------

    void test_isCALReply()
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub1", false,
          CALOpcodes::isCALReply( CALOpcodes::ConnectRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub2", false,
          CALOpcodes::isCALReply( CALOpcodes::ConnectRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub3", false,
          CALOpcodes::isCALReply( CALOpcodes::Disconnect ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub4", false,
          CALOpcodes::isCALReply( CALOpcodes::StartTransactionRO ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub5", false,
          CALOpcodes::isCALReply( CALOpcodes::StartTransactionRW ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub6", false,
          CALOpcodes::isCALReply( CALOpcodes::CommitTransaction ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "pub7", false,
          CALOpcodes::isCALReply( CALOpcodes::RollbackTransaction ) );

      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        bool exp = ( i >= 64 && i< 192 ); // 0x40 <= code < 0xC0
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(), exp, CALOpcodes::isCALReply( i ) );
      }

    }

    // ------------------------------------------------------

    void test_getCALRequest()
    {
      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        if ( i >= 64 && i< 192 ) // 0x40 <= code < 0xC0
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( msg.str(), i%64,
              (unsigned int)CALOpcodes::getCALRequest( i ) );
        }
        else
        {
          try
          {
            CALOpcodes::getCALRequest( i );
            CPPUNIT_FAIL( "Expected an exception in " + msg.str() );
          }
          catch( CALOpcodeException& ) {}
        }
      }
    }

    // ------------------------------------------------------

    void test_getCALReplyOK()
    {
      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        if ( i < 64 ) //  code < 0x40
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( msg.str(), i+64,
              (unsigned int)CALOpcodes::getCALReplyOK( i ) );
        }
        else
        {
          try
          {
            CALOpcodes::getCALReplyOK( i );
            CPPUNIT_FAIL( "Expected an exception in " + msg.str() );
          }
          catch( CALOpcodeException& ) {}
        }
      }
    }

    // ------------------------------------------------------

    void test_getCALReplyException()
    {
      for ( unsigned int i = 0; i < 256; i++ )
      {
        std::stringstream msg;
        msg << "test" << i;
        if ( i < 64 ) // code < 0x40
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( msg.str(), i+128,
              (unsigned int)CALOpcodes::getCALReplyException( i ) );
        }
        else
        {
          try
          {
            CALOpcodes::getCALReplyException( i );
            CPPUNIT_FAIL( "Expected an exception in " + msg.str() );
          }
          catch( CALOpcodeException& ) {}
        }
      }
    }

    // ------------------------------------------------------

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( CALOpcodeTest );

}

CORALCPPUNITTEST_MAIN( CALOpcodeTest )
