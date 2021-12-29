// $Id: test_PacketSocket.cpp,v 1.15.2.6 2011/09/15 14:01:03 avalassi Exp $

// Include files
#include <cstring>
#include <iostream>
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralMonitor/StatsStorage.h"
#include "CoralServerBase/hexstring.h"
#include "CoralServerBase/CTLMagicWordException.h"
#include "CoralSockets/GenericSocketException.h"

// Local include files
#include "../../src/PacketSocket.h"
#include "../../src/RingBufferSocket.h"
#include "../../src/TcpSocket.h"

// Namespace
using namespace coral::CoralSockets;

// port used for testing
const int testPort = 50007;

std::unique_ptr<RingBufferPipes> pipes;

namespace coral
{

  class PacketSocketTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( PacketSocketTest );
    CPPUNIT_TEST( test_PacketSLAC );
    CPPUNIT_TEST( test_PacketSLAC_errorPackets );
    CPPUNIT_TEST( test_ReceiveInvalidPackets );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_tooSmall );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_tooLarge );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_checksumWrong );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_payloadSizeWrong );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_payloadSizeWrong2 );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_headerMissing );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_headerWrongVersion );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_statusCTLWrongMagicWord );
    CPPUNIT_TEST( test_ReceiveInvalidPackets_statusCTLWrongChecksum );
    CPPUNIT_TEST( test_sendPackets );
    CPPUNIT_TEST( test_statistics );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {
    }

    //-------------------------------------------------------------
    // helper methods

    void createSockets( ISocketPtr &src, ISocketPtr &dst)
    {
#if 0
      pipes = std::unique_ptr<RingBufferPipes>(
                                             new RingBufferPipes( 10000, "test pipes") );

      src=pipes->getSrc();
      dst=pipes->getDst();
#else
      TcpSocketPtr listenSocket;
      listenSocket = TcpSocketPtr( (TcpSocket*)0);

      while ( listenSocket.get() == 0 )
      {
        try
        {
          listenSocket = TcpSocket::listenOn( "localhost", testPort );
        }
        catch ( ErrorInHeaderException& e )
        {
          std::string expMsg="Error binding socket";
          if (expMsg == std::string( e.what(), expMsg.size() ) ) {
            std::cout << "Caught exeption: '" << e.what()
                      << "' sleeping."<< std::endl;
            sleep( 10 );
            continue;
          }
          throw;
        }
      };


      src = TcpSocket::connectTo( "localhost", testPort );
      dst = listenSocket->accept();
#endif

    }

    void checkByteBuffer( const ByteBuffer & buf1, const ByteBuffer & buf2 )
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "buffer used size",
                                    buf1.usedSize(), buf2.usedSize() );

      for (unsigned int i = 0; i< buf1.usedSize(); i++) {
        if ( *(buf1.data()+i) != *(buf2.data()+i) )
          CPPUNIT_FAIL("buffer content not equal");
      };
    };


    void checkHeader(const CTLPacketHeader &header,
                     const CTLPacketHeader& eHeader)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "version check",
                                    header.version(), eHeader.version() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "status check",
                                    header.status(), header.status() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "packetSize check",
                                    header.packetSize(),
                                    eHeader.packetSize() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "requestID check",
                                    header.requestID(), eHeader.requestID() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "clientID check",
                                    header.clientID(), eHeader.clientID() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("segmentNumber check",
                                   header.segmentNumber(), eHeader.segmentNumber() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("more Segments check",
                                   header.moreSegments(), eHeader.moreSegments() );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("payloadChecksum",
                                   header.payloadChecksum(),
                                   eHeader.payloadChecksum() );

    };

    void checkPacket(const CTLPacketHeader &header, const ByteBuffer & payload,
                     uint32_t ctlRequestID, uint32_t clientID, uint32_t ctlSegmentNumber,
                     bool ctlMoreSegments, const ByteBuffer& ePayload)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "version check",
                                    header.version(), CTLPACKET_CURRENT_VERSION );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "status check",
                                    header.status(), CTLOK );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "packetSize check",
                                    header.packetSize(),
                                    (uint32_t)( CTLPACKET_HEADER_SIZE + payload.usedSize() ) );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "requestID check",
                                    header.requestID(), ctlRequestID );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "clientID check",
                                    header.clientID(), clientID );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("segmentNumber check",
                                   header.segmentNumber(), ctlSegmentNumber );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("more Segments check",
                                   header.moreSegments(), ctlMoreSegments );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("payloadChecksum",
                                   header.payloadChecksum(),
                                   CTLPacketHeader::computeChecksum(
                                                                    ePayload.data(), ePayload.usedSize() ) );

      checkByteBuffer( payload, ePayload );

    }

    void testPacket( uint32_t ctlRequestID, uint32_t clientID,
                     uint32_t ctlSegmentNumber, bool ctlMoreSegments,
                     const ByteBuffer& payload)
    {
      PacketSLAC packet( ctlRequestID, clientID, ctlSegmentNumber,
                         ctlMoreSegments, payload);
      checkPacket(packet.getHeader(), packet.getPayload(),
                  ctlRequestID, clientID, ctlSegmentNumber, ctlMoreSegments,
                  payload);
    };


    // ------------------------------------------------------
    void test_PacketSLAC()
    {
      {
        const int payloadSize=100;
        ByteBuffer buffer( payloadSize );
        for (int i= 0; i< payloadSize; i++ )
          *(buffer.data()+i)= (unsigned char) i&0xff;
        buffer.setUsedSize( payloadSize );

        testPacket( 0xFFFFFFFF, 1, 0x00000000, false, buffer);
        testPacket( 0xFFFFFFFF, 2, 0x7FFFFFFF, false, buffer);
        testPacket( 0xFFFFFFFF, 3, 0x7FFFFFFF,  true, buffer);
        testPacket( 0xFFFFFFFF, 4, 0x00000000,  true, buffer);

      };

    };

    // ------------------------------------------------------

    void test_PacketSLAC_errorPackets()
    {
      {

        // error message constructor
        PacketSLAC packet( CTLWrongMagicWord );

        // construct the package by hand
        ByteBuffer buffer( CALPACKET_HEADER_SIZE );
        CALPacketHeader calHeader( 0 /*opcode*/, false /*fromProxy*/,
                                   false /*cacheable*/, 0 /*cacheablesize*/);
        ::memcpy( buffer.data(), calHeader.data(), CALPACKET_HEADER_SIZE );
        buffer.setUsedSize( CALPACKET_HEADER_SIZE );

        CTLPacketHeader header(CTLWrongMagicWord,
                               CTLPACKET_HEADER_SIZE+buffer.usedSize(), /*packet size*/
                               0, /*request id*/
                               0, /* client id*/
                               0, /* segment number */
                               false, /* more segments */
                               CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )
                               );


        CPPUNIT_ASSERT_EQUAL_MESSAGE("header 1", 0,
                                     memcmp( header.data(), packet.getHeader().data(),
                                             CTLPACKET_HEADER_SIZE ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("payload 1", 0,
                                     memcmp( buffer.data(), packet.getPayload().data(),
                                             packet.getPayload().usedSize() ) );

      };

    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets()
    {
      ISocketPtr src;
      ISocketPtr dst;

      {
        createSockets( src, dst );
        PacketSocket pSocket( dst );

        // setup home made correct packet
        ByteBuffer buffer(100);
        for (unsigned int i=0; i<buffer.maxSize(); i++)
          *(buffer.data()+i)=(unsigned char)(i&0xff);
        buffer.setUsedSize( buffer.maxSize() );

        CTLPacketHeader header(CTLOK,
                               123, /*packet size*/
                               0, /*request id*/
                               0, /* client id*/
                               0, /* segment number */
                               false, /* more segments */
                               CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )
                               );

        // write it to the one end
        src->writeAll( header.data(), CTLPACKET_HEADER_SIZE );
        src->writeAll( buffer.data(), buffer.usedSize() );

        // receive the packet on the other end
        PacketPtr recPacket = pSocket.receivePacket();

        // and check it
        checkHeader( recPacket->getHeader(), header);
        checkByteBuffer( recPacket->getPayload(), buffer );
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_tooSmall()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // packet size is too small (smaller than header size)-> overflow?
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(CTLOK,
                             122, /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )
                             );

      unsigned char charHeader[CTLPACKET_HEADER_SIZE];
      ::memcpy( &charHeader[0], header.data(), CTLPACKET_HEADER_SIZE );
      // manipulate the packet size
      uint32_t packetSize=22;
      ::memcpy( &charHeader[3], &packetSize, sizeof( packetSize ));

      // write it to the one end
      src->writeAll( charHeader, CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected packet size too small exception");
      }
      catch( ErrorInHeaderException& e )
      {
        std::string expMsg="packet size in received header is too small!";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_tooLarge()
    {
      ISocketPtr src;
      ISocketPtr dst;

      // packet size is too large > 16MB -> throw
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(CTLOK,
                             124, /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )
                             );

      unsigned char charHeader[CTLPACKET_HEADER_SIZE];
      ::memcpy( &charHeader[0], header.data(), CTLPACKET_HEADER_SIZE );
      // manipulate the packet size
      uint32_t packetSize=1024*1024*16+100;
      ::memcpy( &charHeader[3], &packetSize, sizeof( packetSize ));

      // write it to the one end
      src->writeAll( &charHeader[0], CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected packet size too large exception");
      }
      catch( ErrorInHeaderException& e )
      {
        std::string expMsg="packet size in received header is too large!";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_checksumWrong()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // checksum is wrong
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(CTLOK,
                             123, /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      // write it to the one end
      src->writeAll( header.data(), CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected packet wrong checksum exception");
      }
      catch( ErrorInHeaderException& e )
      {
        std::string expMsg="payload checksum in received header is wrong!";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_payloadSizeWrong()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // payload size is wrong (too large) -> checksum exception expected
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(CTLOK,
                             154, /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      // write it to the one end
      src->writeAll( header.data(), CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );
      // fill some garbage to make sure we can receive enough data
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected packet wrong checksum exception");
      }
      catch( ErrorInHeaderException& e )
      {
        std::string expMsg="payload checksum in received header is wrong!";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_payloadSizeWrong2()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // payload size is wrong (too small) -> checksum exception expected
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(CTLOK,
                             100, /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      // write it to the one end
      src->writeAll( header.data(), CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );
      // fill some garbage to make sure we can receive enough data
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected packet wrong checksum exception");
      }
      catch( ErrorInHeaderException& e )
      {
        std::string expMsg="payload checksum in received header is wrong!";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_headerMissing()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // header is missing -> wrong magic word exception
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(CTLOK,
                             123, /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      // write it to the one end
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected packet wrong magic word exception");
      }
      catch( CTLMagicWordException& )
      {
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_headerWrongVersion()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // header has wrong version -> exception
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(CTLOK,
                             CTLPACKET_HEADER_SIZE + buffer.usedSize(), /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      unsigned char charHeader[CTLPACKET_HEADER_SIZE];
      ::memcpy( &charHeader[0], header.data(), CTLPACKET_HEADER_SIZE );
      // manipulate the version
      charHeader[2] = 0x1 | 0x00;

      // write it to the one end
      src->writeAll( &charHeader[0], CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected packet wrong version exception");
      }
      catch( ErrorInHeaderException& e )
      {
        std::string expMsg="received packet with wrong version number!";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };
    // ------------------------------------------------------
#if 0
    // disable for now, fix later
    {
      // status is not CTLOK
      // this checks also that the status is checked before the packet size
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(
                             CTLWrongVersion,
                             123, /* packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      unsigned char charHeader[CTLPACKET_HEADER_SIZE];
      ::memcpy( &charHeader[0], header.data(), CTLPACKET_HEADER_SIZE );
      // manipulate the packet size
      uint32_t packetSize=22;
      ::memcpy( &charHeader[3], &packetSize, sizeof( packetSize ));

      // write it to the one end
      src->writeAll( charHeader, CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );
      // fill some garbage to make sure we can receive enough data
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected status wrong version");
      }
      catch( PacketStatusNotOkException& e )
      {
        std::string expMsg="Received a packet with status WrongVersion";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    }
#endif

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_statusCTLWrongMagicWord()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // status is not CTLOK
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(
                             CTLWrongMagicWord,
                             CTLPACKET_HEADER_SIZE+buffer.usedSize(), /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      // write it to the one end
      src->writeAll( header.data(), CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );
      // fill some garbage to make sure we can receive enough data
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected status wrong magic word");
      }
      catch( PacketStatusNotOkException& e )
      {
        std::string expMsg="Received a packet with status WrongMagicWord";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };

    // ------------------------------------------------------
    void test_ReceiveInvalidPackets_statusCTLWrongChecksum()
    {
      ISocketPtr src;
      ISocketPtr dst;
      // status is not CTLOK
      createSockets( src, dst );
      PacketSocket pSocket( dst );
      ByteBuffer buffer(100);
      for (unsigned int i=0; i<buffer.maxSize(); i++)
        *(buffer.data()+i)=(unsigned char)(i&0xff);
      buffer.setUsedSize( buffer.maxSize() );

      CTLPacketHeader header(
                             CTLWrongChecksum,
                             CTLPACKET_HEADER_SIZE+buffer.usedSize(), /*packet size*/
                             0, /*request id*/
                             0, /* client id*/
                             0, /* segment number */
                             false, /* more segments */
                             CTLPacketHeader::computeChecksum( buffer.data(), buffer.usedSize() )+40
                             );

      // write it to the one end
      src->writeAll( header.data(), CTLPACKET_HEADER_SIZE );
      src->writeAll( buffer.data(), buffer.usedSize() );
      // fill some garbage to make sure we can receive enough data
      src->writeAll( buffer.data(), buffer.usedSize() );

      // receive the packet on the other end
      try
      {
        pSocket.receivePacket();
        CPPUNIT_FAIL("expected status wrong checksum");
      }
      catch( PacketStatusNotOkException& e )
      {
        std::string expMsg="Received a packet with status WrongChecksum";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    };
    // ------------------------------------------------------

    void test_sendPackets()
    {
      ISocketPtr src;
      ISocketPtr dst;

      {
        // check with correct packet
        createSockets( src, dst );
        PacketSocket dSocket( dst );
        PacketSocket sSocket( src );

        ByteBuffer buffer(100);
        for (unsigned int i=0; i<buffer.maxSize(); i++)
          *(buffer.data()+i)=(unsigned char)(i&0xff);
        buffer.setUsedSize( buffer.maxSize() );

        PacketPtr packet( new PacketSLAC( 0 /* ctlRequestID */,
                                          0 /* clientID */,
                                          0 /* ctlSegmentNumber */,
                                          false /* ctlMoreSegments */,
                                          buffer) );

        sSocket.sendPacket( *packet );

        PacketPtr recPacket=dSocket.receivePacket();

        checkPacket( recPacket->getHeader(), recPacket->getPayload(),
                     0 /* ctlRequestID */,
                     0 /* clientID */,
                     0 /* ctlSegmentNumber */,
                     false /* ctlMoreSegments */,
                     buffer);

        checkByteBuffer( recPacket->getPayload(), buffer );
      }
      {
        // check with too large packet
        createSockets( src, dst );
        PacketSocket dSocket( dst );
        PacketSocket sSocket( src );

        std::unique_ptr<ByteBuffer> buffer( new ByteBuffer(1024*1024*17) );
        //for (unsigned int i=0; i<buffer.maxSize(); i++)
        //  *(buffer.data()+i)=(unsigned char)(i&0xff);
        buffer->setUsedSize( buffer->maxSize() );

        try
        {
          PacketPtr packet( new PacketSLAC( 0 /* ctlRequestID */,
                                            0 /* clientID */,
                                            0 /* ctlSegmentNumber */,
                                            false /* ctlMoreSegments */,
                                            *buffer) );

          sSocket.sendPacket( *packet );
          CPPUNIT_FAIL("expected packet size is too large exception");
        }
        catch( Exception& e )
        {
          std::string expMsg="CTL packet is longer than CTLPACKET_MAX_SIZE";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      }
    };

    void
    test_statistics()
    {
      std::vector<std::string> list;
      coral::StatsStorageInstance().report(list);
      std::cout << std::endl;
      for( std::vector<std::string>::iterator i = list.begin(); i != list.end(); ++i )
        std::cout << *i << std::endl;
    };

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( PacketSocketTest );

}

CORALCPPUNITTEST_MAIN( PacketSocketTest )
