// $Id: test_CALPacket.cpp,v 1.11.2.2 2010/12/20 09:10:11 avalassi Exp $
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Include files
#include <sstream>
#include "CoralBase/Exception.h"
#include "CoralServerBase/CALPacketHeader.h"

// Namespace
using namespace coral;

namespace coral
{

  class CALPacketTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( CALPacketTest );
    CPPUNIT_TEST( test_headerCtors );
    CPPUNIT_TEST( test_headerCtor2_tooFewBytes );
    CPPUNIT_TEST( test_headerCtor2_reservedFlags );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {}

    // ------------------------------------------------------

    void test_headerCtors()
    {
      // Create new header from arguments
      unsigned char opcode = 0xAB;
      bool fromProxy = true;
      bool cacheable = true;
      uint32_t cacheableSize = 0x01000203;

      CALPacketHeader header1( opcode,
                               fromProxy,
                               cacheable,
                               cacheableSize );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "opcode1", opcode, header1.opcode() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "fromProxy1", fromProxy, header1.fromProxy() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "cacheable1", cacheable, header1.cacheable() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "cacheableSize1", cacheableSize, header1.cacheableSize() );

      // Test the setFromProxyFlag
      header1.setFromProxy( !fromProxy );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "fromProxy1a", !fromProxy, header1.fromProxy() );
      header1.setFromProxy( fromProxy );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "fromProxy1b", fromProxy, header1.fromProxy() );

      // Encode as string
      unsigned char expHeader1AsString[CALPACKET_HEADER_SIZE] =
        { 0xC0, 0xAB,
          0x03, 0x02, 0x00, 0x01 };
      for ( size_t iChar = 0; iChar < CALPACKET_HEADER_SIZE; iChar++ )
      {
        std::stringstream msg;
        msg << "asString1[" << iChar << "]";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(),
            (unsigned int)((unsigned char*)expHeader1AsString)[iChar],
            (unsigned int)(*((unsigned char*)header1.data()+iChar)) );
      }


      // Create new header2 from string representation of header1 (decode)
      CALPacketHeader header2( header1.data(), CALPACKET_HEADER_SIZE );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "opcode2", opcode, header2.opcode() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "fromProxy2", fromProxy, header2.fromProxy() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "cacheable2", cacheable, header2.cacheable() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "cacheableSize2", cacheableSize, header2.cacheableSize() );
    }

    // ------------------------------------------------------

    void test_headerCtor2_tooFewBytes()
    {
      unsigned char shortHeaderAsString[CALPACKET_HEADER_SIZE-1] =
        { 0xC0, 0xBA,
          0x03, 0x02, 0x01 }; // Missing the last byte
      try
      {
        CALPacketHeader header( shortHeaderAsString, CALPACKET_HEADER_SIZE-1 );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( coral::Exception& e )
      {
        std::stringstream expMsg;
        expMsg << "Too few bytes to build a CALPacketHeader";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "Exception message",
            expMsg.str(),
            std::string( e.what(), expMsg.str().size() ) ); // cut footer
      }
    }

    // ------------------------------------------------------

    void test_headerCtor2_reservedFlags()
    {
      // Set one reserved flag and check the exception
      unsigned char badHeaderAsString[ CALPACKET_HEADER_SIZE ] =
        { 0x00, 0xBA,
          0x03, 0x02, 0x00, 0x01 };
      for ( unsigned int i=0; i<6; i++ )
      {
        std::stringstream msg;
        msg << "Bit#" << i << " ";
        unsigned char flags = ( 1 << i ); // 2^i
        badHeaderAsString[0] = flags;
        //std::cout << "Bit#" << i << ": " << hexstring(&flags,1) << std::endl;
        try
        {
          CALPacketHeader header( badHeaderAsString, CALPACKET_HEADER_SIZE );
          CPPUNIT_FAIL( msg.str() + "Expected an exception" );
        }
        catch( coral::Exception& e )
        {
          std::stringstream expMsg;
          expMsg << "Reserved flags are not = 0";
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( msg.str() + "Exception message",
              expMsg.str(),
              std::string( e.what(), expMsg.str().size() ) ); // cut footer
        }
      }
    }

    // ------------------------------------------------------

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( CALPacketTest );

}

CORALCPPUNITTEST_MAIN( CALPacketTest )
