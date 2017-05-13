// $Id: test_Packet.cpp,v 1.2.2.1 2010/09/30 10:37:13 avalassi Exp $

#include <iostream>

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Local include files
#include "../../src/Packet.h"
#include "../../src/PacketCacheMemory.h"

// Namespace
using namespace coral::CoralServerProxy;

namespace coral
{

  class PacketTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( PacketTest );
    CPPUNIT_TEST( test_PacketBasic );
    CPPUNIT_TEST( test_PacketCompare );
    CPPUNIT_TEST( test_PacketCache );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {}

    //-------------------------------------------------------------
    // helper methods

    PacketPtr makePacket ( Packet::PacketType type, CALOpcode opcode, size_t dsize, bool cacheable = true )
    {
      size_t psize = dsize + Packet::HEADERS_SIZE ;

      unsigned char* data = new unsigned char[psize] ;
      new( data+CTLPACKET_HEADER_SIZE) CALPacketHeader ( opcode, true, cacheable, dsize );

      // calculate checksum
      uint32_t checksum =
        CTLPacketHeader::computeChecksum( data+CTLPACKET_HEADER_SIZE,
                                          psize-CTLPACKET_HEADER_SIZE );

      // build CTL header
      new(data) CTLPacketHeader ( CTLOK,
                                  psize,
                                  1,
                                  0,
                                  0,
                                  false,
                                  checksum ) ;

      // packet takes ownership over the allocated buffer
      Packet* p = new Packet ( data, type ) ;
      return PacketPtr(p) ;
    }

    PacketPtr makeRequest ( CALOpcode opcode, const char* req, size_t dsize, uint32_t clientId, uint32_t reqId, bool cacheable = true )
    {
      size_t psize = dsize + Packet::HEADERS_SIZE ;

      unsigned char* data = new unsigned char[psize] ;
      for ( std::string::size_type i = 0 ; i < dsize ; ++ i ) {
        data[i+Packet::HEADERS_SIZE] = req[i] ;
      }

      new( data+CTLPACKET_HEADER_SIZE) CALPacketHeader ( opcode, true, cacheable, dsize );

      // calculate checksum
      uint32_t checksum =
        CTLPacketHeader::computeChecksum( data+CTLPACKET_HEADER_SIZE,
                                          psize-CTLPACKET_HEADER_SIZE );

      // build CTL header
      new(data) CTLPacketHeader ( CTLOK,
                                  psize,
                                  reqId,
                                  clientId,
                                  0,
                                  false,
                                  checksum ) ;

      // packet takes ownership over the allocated buffer
      Packet* p = new Packet ( data, Packet::Request ) ;
      return PacketPtr(p) ;
    }

    PacketPtr makeReply ( PacketPtr request, const char* data, size_t dsize, bool cacheable=true )
    {
      CALOpcode replyOpcode = CALOpcodes::getCALReplyOK(request->calHeader().opcode()) ;
      return Packet::buildReply( replyOpcode, request->ctlHeader(), std::string(data,dsize), cacheable ) ;
    }

    // ------------------------------------------------------
    void test_PacketBasic()
    {
      // Make sure that we can create simple request packet
      PacketPtr p1 = makePacket( Packet::Request, 19, 59, true ) ;
      CPPUNIT_ASSERT( p1 ) ;
      CPPUNIT_ASSERT_EQUAL( Packet::Request, p1->type() ) ;
      CPPUNIT_ASSERT_EQUAL( (size_t)59U, p1->dataSize() ) ;
      CPPUNIT_ASSERT_EQUAL( (size_t)88U, p1->packetSize() ) ;
      CPPUNIT_ASSERT_EQUAL( int(19), int(p1->calHeader().opcode()) ) ;
      CPPUNIT_ASSERT( p1->calHeader().cacheable() ) ;
      CPPUNIT_ASSERT_EQUAL( 59U, p1->calHeader().cacheableSize() ) ;
      CPPUNIT_ASSERT( p1->calHeader().fromProxy() ) ;
      CPPUNIT_ASSERT_EQUAL( 88U, p1->ctlHeader().packetSize() ) ;

      // can also create reply for it
      PacketPtr p2 = makeReply( p1, "12345", 5, true ) ;
      CPPUNIT_ASSERT( p2 ) ;
      CPPUNIT_ASSERT_EQUAL( Packet::Reply, p2->type() ) ;
      CPPUNIT_ASSERT_EQUAL( (size_t)5U, p2->dataSize() ) ;
      CPPUNIT_ASSERT_EQUAL( (size_t)34U, p2->packetSize() ) ;
      CPPUNIT_ASSERT_EQUAL( int(19 | 0x40), int(p2->calHeader().opcode()) ) ;
      CPPUNIT_ASSERT( p2->calHeader().cacheable() ) ;
      CPPUNIT_ASSERT_EQUAL( 5U, p2->calHeader().cacheableSize() ) ;
      CPPUNIT_ASSERT( p2->calHeader().fromProxy() ) ;
      CPPUNIT_ASSERT_EQUAL( 34U, p2->ctlHeader().packetSize() ) ;

    }

    // ------------------------------------------------------
    void test_PacketCompare()
    {
      // make two identical packets
      PacketPtr p1 = makePacket( Packet::Request, 19, 59, true ) ;
      PacketPtr p2 = makePacket( Packet::Request, 19, 59, true ) ;
      unsigned char* data1 = p1->data() ;
      unsigned char* data2 = p2->data() ;
      for ( int i = 0 ; i < 59 ; ++ i ) {
        data1[i] = data2[i] = (unsigned char)(i+64) ;
      }

      // they must be equal
      CPPUNIT_ASSERT( *p1 == *p2 ) ;

      // less than comparison is false
      CPPUNIT_ASSERT( not ( *p1 < *p2 ) ) ;
      CPPUNIT_ASSERT( not ( *p2 < *p1 ) ) ;
    }


    // ------------------------------------------------------
    void test_PacketCache()
    {
      // instantiate cache
      PacketCacheMemory cache(10,1000) ;

      // make request and reply packets
      const char* q ;
      const char* r ;

      q = "\x02\x04\x08\x04R\x00\x00\x00" ;
      r = "000" ;
      PacketPtr p = makeRequest( 23, q, 8, 1, 2 ) ;
      cache.insert ( p, std::vector<PacketPtr>( 1, makeReply(p,r,3) ) ) ;

      q = "\x02\x04\x08\x04R\x00\x00\x00\x00\x00" ;
      r = "000" ;
      p = makeRequest( 20, q, 10, 1, 4 ) ;
      cache.insert ( p, std::vector<PacketPtr>( 1, makeReply(p,r,3) ) ) ;

      q = "\x02\x04\x08\x04R\x00\x00\x00\x00\x00\x10\x00CORALSERVER_TEST" ;
      r = "000" ;
      p = makeRequest( 21, q, 28, 1, 8 ) ;
      cache.insert ( p, std::vector<PacketPtr>( 1, makeReply(p,r,3) ) ) ;

      q = "\x02\x04\x08\x04R\x00\x00\x00\x01\x02\x00\x02\x00ID\x07\x01\x00x\x0d\x00\x00\x00\x00\x00\x01\x00\x10\x00CORALSERVER_TEST\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" ;
      r = "000" ;
      p = makeRequest( 19, q, 59, 1, 9 ) ;
      cache.insert ( p, std::vector<PacketPtr>( 1, makeReply(p,r,3) ) ) ;

      q = "\x02\x04\x08\x04R\x00\x00\x00\x01\x02\x00\x02\x00ID\x07\x01\x00x\x0d\x00\x00\x00\x00\x00\x01\x00\x10\x00CORALSERVER_TEST\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" ;
      r = "000" ;
      p = makeRequest( 19, q, 59, 1, 12 ) ;
      CPPUNIT_ASSERT ( cache.find ( p ) ) ;
      if ( not cache.find ( p ) ) {
        cache.insert ( p, std::vector<PacketPtr>( 1, makeReply(p,r,3) ) ) ;
      }

    }


  };


  CPPUNIT_TEST_SUITE_REGISTRATION( PacketTest );

}

CORALCPPUNITTEST_MAIN( PacketTest )

