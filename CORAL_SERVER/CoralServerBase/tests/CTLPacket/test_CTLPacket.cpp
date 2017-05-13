// $Id: test_CTLPacket.cpp,v 1.15.2.2 2010/12/20 09:10:11 avalassi Exp $
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Include files
#include <sstream>
#include "CoralServerBase/CALPacketHeader.h"
#include "CoralServerBase/CTLPacketHeader.h"
#include "CoralServerBase/CTLMagicWordException.h"

// Namespace
using namespace coral;

namespace coral
{

  class CTLPacketTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( CTLPacketTest );
    CPPUNIT_TEST( test_headerCtors );
    CPPUNIT_TEST( test_headerCtor1_status );
    CPPUNIT_TEST( test_headerCtor1_moreSegments );
    CPPUNIT_TEST( test_headerCtor1_badSegmentNumber );
    CPPUNIT_TEST( test_headerCtor1_packetTooShort );
    CPPUNIT_TEST( test_headerCtor1_packetTooLong );
    CPPUNIT_TEST( test_headerCtor2_tooFewBytes );
    CPPUNIT_TEST( test_headerCtor2_badMagicWord );
    CPPUNIT_TEST( test_headerCtor2_packetTooShort );
    CPPUNIT_TEST( test_headerCtor2_packetTooLong );
    CPPUNIT_TEST( test_computeChecksum );
    CPPUNIT_TEST( test_headerCast );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {}

    // ------------------------------------------------------

    void test_headerCtors()
    {
      // Create a header from arguments
      CTLStatus status = CTLWrongChecksum;
      uint32_t packetSize      = 0x00001203; // max size is 0x0000FFFF
      uint32_t requestID       = 0x01000204;
      uint32_t clientID        = 0x01000205;
      uint32_t segmentNumber   = 0x01000206;
      bool moreSegments    = true;
      uint32_t payloadChecksum = 0x01000207;

      CTLPacketHeader header1( status,
                               packetSize,
                               requestID,
                               clientID,
                               segmentNumber,
                               moreSegments,
                               payloadChecksum );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "version1", CTLPACKET_CURRENT_VERSION, header1.version() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "status1", status, header1.status() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "packetSize1", packetSize, header1.packetSize() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "requestID1", requestID, header1.requestID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "clientID1", clientID, header1.clientID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "segmentNumber1", segmentNumber, header1.segmentNumber() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "moreSegments1", moreSegments, header1.moreSegments() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "payloadChecksum1", payloadChecksum, header1.payloadChecksum() );

      // Encode as a string
      unsigned char expHeader1AsString[CTLPACKET_HEADER_SIZE] =
        { 0xFE, 0xED,
          0x00 | 0xC0,
          0x03, 0x12, 0x00, 0x00, // max size is 0x0000FFFF
          0x04, 0x02, 0x00, 0x01,
          0x05, 0x02, 0x00, 0x01,
          0x06, 0x02, 0x00, 0x01 | 0x80,
          0x07, 0x02, 0x00, 0x01 };
      for ( size_t iChar = 0; iChar < CTLPACKET_HEADER_SIZE; iChar++ )
      {
        std::stringstream msg;
        msg << "asString1[" << iChar << "]";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(),
            (unsigned int)((unsigned char*)expHeader1AsString)[iChar],
            (unsigned int)(*((unsigned char*)header1.data()+iChar)) );
      }

      // Test the setClientID method
      header1.setClientID( clientID+1 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "clientID1a", clientID+1, header1.clientID() );
      header1.setClientID( clientID );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "clientID1b", clientID, header1.clientID() );

      // Test the setPayloadChecksum method
      header1.setPayloadChecksum( payloadChecksum+1 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "payloadChecksum1a", payloadChecksum+1, header1.payloadChecksum() );
      header1.setPayloadChecksum( payloadChecksum );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "payloadChecksum1b", payloadChecksum, header1.payloadChecksum() );

      // Create a new header2 from the string representation of header1 (decode)
      CTLPacketHeader header2( header1.data(), CTLPACKET_HEADER_SIZE );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "version2", CTLPACKET_CURRENT_VERSION, header2.version() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "status2", status, header2.status() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "packetSize2", packetSize, header2.packetSize() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "requestID2", requestID, header2.requestID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "clientID2", clientID, header2.clientID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "segmentNumber2", segmentNumber, header2.segmentNumber() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "moreSegments2", moreSegments, header2.moreSegments() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "payloadChecksum2", payloadChecksum, header2.payloadChecksum() );
    }

    // ------------------------------------------------------

    void test_headerCtor1_status()
    {
      uint32_t size = CTLPACKET_HEADER_SIZE + CALPACKET_HEADER_SIZE;
      {
        CTLStatus status = CTLOK;
        CTLPacketHeader header( status, size, 0, 0, 0, false, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "status1", status, header.status() );
      }
      {
        CTLStatus status = CTLWrongMagicWord;
        CTLPacketHeader header( status, size, 0, 0, 0, false, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "status2", status, header.status() );
      }
      {
        CTLStatus status = CTLWrongVersion;
        CTLPacketHeader header( status, size, 0, 0, 0, false, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "status3", status, header.status() );
      }
      {
        CTLStatus status = CTLWrongChecksum;
        CTLPacketHeader header( status, size, 0, 0, 0, false, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "status4", status, header.status() );
      }
    }

    // ------------------------------------------------------

    void test_headerCtor1_moreSegments()
    {
      CTLStatus status = CTLOK;
      uint32_t size = CTLPACKET_HEADER_SIZE + CALPACKET_HEADER_SIZE;
      {
        bool more = false;
        CTLPacketHeader header( status, size, 0, 0, 0, more, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "moreF1", more, header.moreSegments() );
      }
      {
        bool more = true;
        CTLPacketHeader header( status, size, 0, 0, 0, more, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "moreT1", more, header.moreSegments() );
      }
      {
        bool more = false;
        CTLPacketHeader header( status, size, 0, 0, 0x7FFFFFFF, more, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "moreF2", more, header.moreSegments() );
      }
      {
        bool more = true;
        CTLPacketHeader header( status, size, 0, 0, 0x7FFFFFFF, more, 0 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "moreT2", more, header.moreSegments() );
      }
    }

    // ------------------------------------------------------

    void test_headerCtor1_badSegmentNumber()
    {
      // Set a bad segment number and check the exception
      CTLStatus status = CTLWrongChecksum;
      uint32_t packetSize       = 0x00001203; // max size is 0x0000FFFF
      uint32_t requestID        = 0x01000204;
      uint32_t clientID         = 0x01000205;
      uint32_t badSegmentNumber = 0x81000206;
      bool moreSegments     = true;
      uint32_t payloadChecksum  = 0x01000207;

      try
      {
        CTLPacketHeader header( status,
                                packetSize,
                                requestID,
                                clientID,
                                badSegmentNumber,
                                moreSegments,
                                payloadChecksum );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( std::exception& e )
      {
        std::stringstream expMsg;
        expMsg << "Segment number " << badSegmentNumber << " is > 0x7FFFFFFF";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "Exception message",
            expMsg.str(),
            std::string( e.what(), expMsg.str().size() ) ); // cut trailing CORAL info
      }
    }

    // ------------------------------------------------------

    void test_headerCtor1_packetTooShort()
    {
      uint32_t size = CTLPACKET_HEADER_SIZE + CALPACKET_HEADER_SIZE - 1; // Packet too short
      try
      {
        CTLPacketHeader header( CTLOK, size, 0, 0, 0, false, 0 );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( std::exception& e )
      {
        std::stringstream expMsg;
        expMsg << "CTL packet is shorter than CTL+CAL headers";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "Exception message",
            expMsg.str(),
            std::string( e.what(), expMsg.str().size() ) ); // cut trailing CORAL info
      }
      CTLPacketHeader header( CTLOK, size+1, 0, 0, 0, false, 0 ); // min size
    }

    // ------------------------------------------------------

    void test_headerCtor1_packetTooLong()
    {
      uint32_t size = CTLPACKET_MAX_SIZE + 1; // Packet too long
      try
      {
        CTLPacketHeader header( CTLOK, size, 0, 0, 0, false, 0 );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( std::exception& e )
      {
        std::stringstream expMsg;
        expMsg << "CTL packet is longer than CTLPACKET_MAX_SIZE";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "Exception message",
            expMsg.str(),
            std::string( e.what(), expMsg.str().size() ) ); // cut trailing CORAL info
      }
      CTLPacketHeader header( CTLOK, size-1, 0, 0, 0, false, 0 ); // max size
    }

    // ------------------------------------------------------

    void test_headerCtor2_tooFewBytes()
    {
      unsigned char shortHeaderAsString[CTLPACKET_HEADER_SIZE-1] =
        { 0xBA, 0xAD,
          0x00 | 0xC0,
          0x03, 0x12, 0x00, 0x00, // max size is 0x0000FFFF
          0x04, 0x02, 0x00, 0x01,
          0x05, 0x02, 0x00, 0x01,
          0x06, 0x02, 0x00, 0x01 | 0x80,
          0x07, 0x02, 0x00 }; // Missing the last byte
      try
      {
        CTLPacketHeader header( shortHeaderAsString, CTLPACKET_HEADER_SIZE - 1 );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( std::exception& e )
      {
        std::stringstream expMsg;
        expMsg << "Too few bytes to build a CTLPacketHeader";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "Exception message",
            expMsg.str(),
            std::string( e.what(), expMsg.str().size() ) ); // cut trailing CORAL info
      }
    }

    // ------------------------------------------------------

    void test_headerCtor2_badMagicWord()
    {
      // Set a bad magic word and check the exception
      unsigned char badHeaderAsString[ CTLPACKET_HEADER_SIZE ] =
        { 0xBA, 0xAD,
          0x00 | 0xC0,
          0x03, 0x12, 0x00, 0x00, // max size is 0x0000FFFF
          0x04, 0x02, 0x00, 0x01,
          0x05, 0x02, 0x00, 0x01,
          0x06, 0x02, 0x00, 0x01 | 0x80,
          0x07, 0x02, 0x00, 0x01 };
      try
      {
        CTLPacketHeader header( badHeaderAsString, CTLPACKET_HEADER_SIZE );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( CTLMagicWordException& ) {}
      catch( std::exception& e )
      {
        std::cout << "Exception caught: " << e.what() << std::endl;
        throw;
      }
    }

    // ------------------------------------------------------

    void test_headerCtor2_packetTooShort()
    {
      unsigned char headerAsString[CTLPACKET_HEADER_SIZE] =
        { 0xFE, 0xED,
          0x00 | 0xC0,
          0x16, 0x00, 0x00, 0x00, // min size is 23 (0x00000017)
          0x04, 0x02, 0x00, 0x01,
          0x05, 0x02, 0x00, 0x01,
          0x06, 0x02, 0x00, 0x01 | 0x80,
          0x07, 0x02, 0x00, 0x01 };
      try
      {
        CTLPacketHeader header( headerAsString, CTLPACKET_HEADER_SIZE );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( std::exception& e )
      {
        std::stringstream expMsg;
        expMsg << "CTL packet is shorter than CTL+CAL headers";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "Exception message",
            expMsg.str(),
            std::string( e.what(), expMsg.str().size() ) ); // cut trailing CORAL info
      }
    }

    // ------------------------------------------------------

    void test_headerCtor2_packetTooLong()
    {
      unsigned char headerAsString[CTLPACKET_HEADER_SIZE] =
        { 0xFE, 0xED,
          0x00 | 0xC0,
          0x00, 0x00, 0x00, 0x01, // max size is 0x00FFFFFF
          0x04, 0x02, 0x00, 0x01,
          0x05, 0x02, 0x00, 0x01,
          0x06, 0x02, 0x00, 0x01 | 0x80,
          0x07, 0x02, 0x00, 0x01 };
      try
      {
        CTLPacketHeader header( headerAsString, CTLPACKET_HEADER_SIZE );
        CPPUNIT_FAIL( "Expected an exception" );
      }
      catch( std::exception& e )
      {
        std::stringstream expMsg;
        expMsg << "CTL packet is longer than CTLPACKET_MAX_SIZE";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "Exception message",
            expMsg.str(),
            std::string( e.what(), expMsg.str().size() ) ); // cut trailing CORAL info
      }
    }

    // ------------------------------------------------------

    // Check a few pre computed crc sums
    void test_computeChecksum()
    {
      const int bufferSize = 101; // skip the first byte
      unsigned char buffer[bufferSize];
      buffer[0] = 0xff; // skipped

      for ( int i=0; i<bufferSize-1; i++ )
        buffer[i+1] = (unsigned char)(i&0xff);
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "checksum 1",
                                    (uint32_t)1489580789UL,
                                    CTLPacketHeader::computeChecksum( buffer, bufferSize ) );

      for ( int i=0; i<bufferSize-1; i++ )
        buffer[i+1] = (unsigned char)((bufferSize-1-i)&0xff);
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "checksum 2",
                                    (uint32_t)3017524937UL,
                                    CTLPacketHeader::computeChecksum( buffer, bufferSize ) );

      for ( int i=0; i<bufferSize-1; i++ )
        buffer[i+1] = (unsigned char)((i*15)&0xff);
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "checksum 3",
                                    (uint32_t)3670247311UL,
                                    CTLPacketHeader::computeChecksum( buffer, bufferSize ) );

    }

    // ------------------------------------------------------

    void test_headerCast()
    {
      // Create a header from arguments
      CTLStatus status = CTLWrongChecksum;
      uint32_t packetSize      = 0x00001203; // max size is 0x0000FFFF
      uint32_t requestID       = 0x01000204;
      uint32_t clientID        = 0x01000205;
      uint32_t segmentNumber   = 0x01000206;
      bool moreSegments    = true;
      uint32_t payloadChecksum = 0x01000207;

      CTLPacketHeader header1( status,
                               packetSize,
                               requestID,
                               clientID,
                               segmentNumber,
                               moreSegments,
                               payloadChecksum );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "version1", CTLPACKET_CURRENT_VERSION, header1.version() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "status1", status, header1.status() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "packetSize1", packetSize, header1.packetSize() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "requestID1", requestID, header1.requestID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "clientID1", clientID, header1.clientID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "segmentNumber1", segmentNumber, header1.segmentNumber() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "moreSegments1", moreSegments, header1.moreSegments() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "payloadChecksum1", payloadChecksum, header1.payloadChecksum() );

      // Encode as a string
      unsigned char expHeader1AsString[CTLPACKET_HEADER_SIZE] =
        { 0xFE, 0xED,
          0x00 | 0xC0,
          0x03, 0x12, 0x00, 0x00, // max size is 0x0000FFFF
          0x04, 0x02, 0x00, 0x01,
          0x05, 0x02, 0x00, 0x01,
          0x06, 0x02, 0x00, 0x01 | 0x80,
          0x07, 0x02, 0x00, 0x01 };
      for ( size_t iChar = 0; iChar < CTLPACKET_HEADER_SIZE; iChar++ )
      {
        std::stringstream msg;
        msg << "asString1[" << iChar << "]";
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( msg.str(),
            (unsigned int)((unsigned char*)expHeader1AsString)[iChar],
            (unsigned int)(*((unsigned char*)header1.data()+iChar)) );
      }

      // Use reinterpret_cast as in the CoralServerProxy package
      CTLPacketHeader* pHeader =
        reinterpret_cast<CTLPacketHeader*>( expHeader1AsString );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "version1", CTLPACKET_CURRENT_VERSION, pHeader->version() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "status1", status, pHeader->status() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "packetSize1", packetSize, pHeader->packetSize() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "requestID1", requestID, pHeader->requestID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "clientID1", clientID, pHeader->clientID() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "segmentNumber1", segmentNumber, pHeader->segmentNumber() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "moreSegments1", moreSegments, pHeader->moreSegments() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "payloadChecksum1", payloadChecksum, pHeader->payloadChecksum() );

    }

    // ------------------------------------------------------

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( CTLPacketTest );

}

CORALCPPUNITTEST_MAIN( CTLPacketTest )
