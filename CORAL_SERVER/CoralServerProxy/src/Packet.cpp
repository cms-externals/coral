//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: Packet.cpp,v 1.11.2.4 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class Packet...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "Packet.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger.h"
#include "NetSocket.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace coral {
namespace CoralServerProxy {

//----------------
// Constructors --
//----------------
// constructor from the buffer and possibly timestamp,
// packet object takes ownership over the buffer which
// must be allocated with new[]
Packet::Packet ( unsigned char *buf, PacketType type )
  : m_type(type)
  , m_recvTime()
  , m_wholePacket(buf)
{
}

Packet::Packet ( unsigned char *buf, PacketType type, const TimeStamp& m_recvTime )
  : m_type(type)
  , m_recvTime(m_recvTime)
  , m_wholePacket(buf)
{
}

//--------------
// Destructor --
//--------------
Packet::~Packet ()
{
  delete [] m_wholePacket ;
}

// compare two packets for the purpose of caching. they are equal iff:
// 1. opcode is the same
// 2. cacheable flag is the same is the same
// 3. content of the cacheable data in payload is the same for cacheable
//    packets or whole payload is the same for non-cacheable packets
bool
Packet::operator == ( const Packet& other ) const
{
  const CALPacketHeader& calh = calHeader() ;
  const CALPacketHeader& ocalh = other.calHeader() ;

  // different opcodes means different
  if ( calh.opcode() != ocalh.opcode() ) return false ;

  // data buffers
  const unsigned char* myData = data() ;
  const unsigned char* otherData = other.data() ;

  if ( calh.cacheable() and ocalh.cacheable() ) {

    // both are cacheable

    // cacheable sizes must be equal
    if ( calh.cacheableSize() != ocalh.cacheableSize() ) return false ;

    // compare data
    return std::equal ( myData, myData+calh.cacheableSize(), otherData ) ;

  } else if ( calh.cacheable() != ocalh.cacheable() ) {

    // packets with different cacheable flags are different
    return false ;

  } else {

    // for non-cacheable packets compare whole payload

    // size must be equal
    if ( dataSize() != other.dataSize() ) return false ;

    // compare data
    return std::equal ( myData, myData+dataSize(), otherData ) ;


  }
}

// compare two packets for the purpose of sorting,
// only cache-related things are compared for cacheable packets
bool
Packet::operator < ( const Packet& other ) const
{
  const CALPacketHeader& calh = calHeader() ;
  const CALPacketHeader& ocalh = other.calHeader() ;

  // opcodes first
  if ( calh.opcode() < ocalh.opcode() ) return true ;
  if ( calh.opcode() > ocalh.opcode() ) return false ;

  const unsigned char* myData = data() ;
  const unsigned char* otherData = other.data() ;


  if ( calh.cacheable() and ocalh.cacheable() ) {

    // both are cacheable

    size_t mySize = calh.cacheableSize() ;
    size_t otherSize = ocalh.cacheableSize() ;

    // shorter packet compare less than longer
    if ( mySize < otherSize ) return true ;
    if ( mySize > otherSize ) return false ;

    // compare data
    return std::lexicographical_compare( myData, myData+mySize, otherData, otherData+otherSize ) ;

  } else if ( calh.cacheable() and not ocalh.cacheable() ) {

    // cacheable packets are "smaller" than non-cacheable
    return true ;

  } else if ( not calh.cacheable() and ocalh.cacheable() ) {

    // cacheable packets are "smaller" than non-cacheable
    return false ;

  } else {

    size_t mySize = dataSize() ;
    size_t otherSize = other.dataSize() ;

    // shorter packet compare less than longer
    if ( mySize < otherSize ) return true ;
    if ( mySize > otherSize ) return false ;

    // compare data
    return std::lexicographical_compare( myData, myData+mySize, otherData, otherData+otherSize ) ;
  }
}

// Build the reply packet, take clientId, requestId, segNum from request header.
PacketPtr
Packet::buildReply ( CALOpcode opcode,
                     const CTLPacketHeader& reqHdr,
                     const std::string& payload,
                     bool cacheable )
{
  size_t dsize = payload.size() ;
  size_t psize = dsize + HEADERS_SIZE ;

  // allocate data for it
  unsigned char* data = new unsigned char[psize] ;

  // build CAL header first, have to calculate checksum for CTL header
  new( data+CTLPACKET_HEADER_SIZE) CALPacketHeader ( opcode,
                                                     true,
                                                     cacheable,
                                                     dsize );

  // copy the data too
  std::copy ( payload.begin(), payload.end(), data+HEADERS_SIZE ) ;

  // calculate checksum
  uint32_t checksum =
    CTLPacketHeader::computeChecksum( data+CTLPACKET_HEADER_SIZE,
                                      psize-CTLPACKET_HEADER_SIZE );

  // build CTL header
  new(data) CTLPacketHeader ( CTLOK,
                              psize,
                              reqHdr.requestID(),
                              reqHdr.clientID(),
                              0,
                              false,
                              checksum ) ;

  // make packet now
  try {
    // packet takes ownership over the allocated buffer
    Packet* p = new Packet ( data, Reply ) ;
    return PacketPtr(p) ;
  } catch ( std::exception& ) {
    PXY_ERR ( "Packet::buildReply - memory allocation failed" ) ;
    delete [] data ;
    return PacketPtr() ;
  }
}

// Build the control packet, the only thing that makes sense is client ID
PacketPtr
Packet::buildControl( CALOpcode opcode, size_t clientId )
{
  size_t psize = HEADERS_SIZE ;

  // allocate data for it
  unsigned char* data = new unsigned char[psize] ;

  // build CAL header first, have to calculate checksum for CTL header
  new( data+CTLPACKET_HEADER_SIZE) CALPacketHeader ( opcode,
                                                     true,
                                                     false,
                                                     0 );

  // calcualte checksum
  uint32_t checksum =
    CTLPacketHeader::computeChecksum( data+CTLPACKET_HEADER_SIZE,
                                      psize-CTLPACKET_HEADER_SIZE );

  // build CTL header
  new(data) CTLPacketHeader ( CTLOK,
                              psize,
                              0,
                              clientId,
                              0,
                              false,
                              checksum ) ;

  // make packet now
  try {
    // packet takes ownership over the allocated buffer
    Packet* p = new Packet ( data, Control ) ;
    return PacketPtr(p) ;
  } catch ( std::exception& ) {
    PXY_ERR ( "Packet::buildControl - memory allocation failed" ) ;
    delete [] data ;
    return PacketPtr() ;
  }
}


// Read the packet from socket.
// Returns new packet object, 0 on EOF or any error.
boost::shared_ptr<Packet>
Packet::read( NetSocket& socket, PacketType type )
{
  unsigned char buf[CTLPACKET_HEADER_SIZE] ;

  // read header first
  int s = socket.read ( buf, CTLPACKET_HEADER_SIZE ) ;
  if ( s < 0 ) {
    PXY_ERR ( "Packet::read " << socket << " - failed to read packet header: " << strerror(errno) ) ;
    return PacketPtr() ;
  } else if ( s == 0 ) {
    // Connection closed, this is OK
    PXY_INFO ( "Packet::read " << socket << " - connection closed." ) ;
    return PacketPtr() ;
  } else if ( s != int(CTLPACKET_HEADER_SIZE) ) {
    // Too little data received, not good
    PXY_INFO ( "Packet::read " << socket << " - too little data (partial header), connection dropped?" ) ;
    return PacketPtr() ;
  }

  // get the header
  CTLPacketHeader* ctlHeader = reinterpret_cast<CTLPacketHeader*>(buf) ;

  PXY_DEBUG ("Packet::read " << socket << " - read " << s << " bytes, packet size = " << ctlHeader->packetSize() ) ;

  // check the packet size
  if ( ctlHeader->packetSize() < HEADERS_SIZE ) {
    PXY_ERR ( "Packet::read " << socket << " - packet size is smaller than header size: " << ctlHeader->packetSize() ) ;
    return PacketPtr() ;
  }

  // allocate buffer for packet, copy the data
  unsigned char* packetBuf ;
  try {
    packetBuf = new unsigned char[ctlHeader->packetSize()] ;
    std::copy ( buf, buf+s, packetBuf ) ;
  } catch ( std::exception& ) {
    PXY_ERR ( "Packet::read " << socket << " - memory allocation failed, packet size = " << ctlHeader->packetSize() ) ;
    return PacketPtr() ;
  }

  // read remaining data if any
  if ( uint32_t(s) < ctlHeader->packetSize() ) {

    size_t bytesLeft = ctlHeader->packetSize() - s ;
    PXY_DEBUG ("Packet::read " << socket << " - read remaining " << bytesLeft << " bytes" ) ;
    s = socket.read ( packetBuf+s, bytesLeft ) ;
    if ( s < 0 ) {
      PXY_ERR ( "Packet::read " << socket << " - failed to read packet data: " << strerror(errno) ) ;
      delete [] packetBuf ;
      return PacketPtr() ;
    } else if ( s != int(bytesLeft) ) {
      PXY_INFO ( "Packet::read " << socket << " - connection closed while reading packet data: " << strerror(errno) ) ;
      delete [] packetBuf ;
      return PacketPtr() ;
    }

  }

  // build the packet
  try {
    // packet takes ownership over the allocated buffer
    Packet* p = new Packet ( packetBuf, type, TimeStamp::now() ) ;
    return PacketPtr(p) ;
  } catch ( std::exception& ) {
    PXY_ERR ( "Packet::read " << socket << " - memory allocation failed" ) ;
    delete [] packetBuf ;
    return PacketPtr() ;
  }
}

// Write the packet to socket.
// Returns number of bytes written, 0 on EOF, negative on error.
int
Packet::write ( NetSocket& socket )
{
  // calculate whole packet size
  size_t fullsize =  packetSize() ;

  // send it down the stream
  int s = socket.write ( m_wholePacket, fullsize ) ;
  //PXY_DEBUG ( "Packet::write: fullsize=" << fullsize << ", s=" << s ) ;
  if ( s > 0 and s < int(fullsize) ) {
    // seems like close happened during transmission
    s = 0 ;
  }

  return s ;
}

// Write the packet to socket, but use different client ID.
// Returns number of bytes written, 0 on EOF, negative on error.
int
Packet::write ( NetSocket& socket, const CTLPacketHeader& ctlh  )
{
  const size_t psize = this->packetSize() ;

  // optimization, packets shorter than 16k are sent in one packet
  const unsigned oneBufSize = 1024*16 ;
  if ( psize <= oneBufSize ) {

    // make buffer big enough
#ifdef __APPLE__
    std::unique_ptr<unsigned char> aBuf( new unsigned char[psize] );
    unsigned char* buf = aBuf.get();
#else
    unsigned char buf[psize] ;
#endif

    // copy whole data
    std::copy ( m_wholePacket, m_wholePacket+psize, buf ) ;

    // override some fields from alternative header, only take
    // packet size and checksum from the packet itself
    const unsigned char* hdata = ctlh.data() ;
    std::copy ( hdata, hdata+3, buf ) ;
    std::copy ( hdata+7, hdata+7+3*4, buf+7 ) ;

    // send it down the stream
    int s = socket.write ( buf, psize ) ;
    //PXY_DEBUG ( "Packet::write: psize=" << psize << ", s=" << s ) ;
    if ( s > 0 and s < int(psize) ) {
      // seems like close happened during transmission
      s = 0 ;
    }

    return s ;

  } else {

    // will send header separately
    unsigned char buf[CTLPACKET_HEADER_SIZE] ;

    // make sure that data size and checksum in header
    // corresponds to real data
    const unsigned char* hdata = ctlh.data() ;
    std::copy ( hdata, hdata+CTLPACKET_HEADER_SIZE, buf ) ;
    std::copy ( m_wholePacket+3, m_wholePacket+3+4, buf+3 ) ;
    std::copy ( m_wholePacket+19, m_wholePacket+19+4, buf+19 ) ;

    // send the header
    int s = socket.write ( buf, CTLPACKET_HEADER_SIZE ) ;
    //PXY_DEBUG ( "Packet::write: header=" << sizeof(HeaderType) << ", s=" << s ) ;
    if ( s < 0 ) {
      // error happened
      return s ;
    } else if ( s >= 0 and s < int(CTLPACKET_HEADER_SIZE) ) {
      // seems like close happened during transmission
      return 0 ;
    }

    // send data part
    size_t dsize = psize-CTLPACKET_HEADER_SIZE ;
    int s2 = socket.write ( m_wholePacket+CTLPACKET_HEADER_SIZE, dsize ) ;
    //PXY_DEBUG ( "Packet::write: dataSize=" << dataSize << ", s2=" << s2 ) ;
    if ( s2 < 0 ) {
      // error happened
      return s2 ;
    } if ( s2 > 0 and s2 < int(dsize) ) {
      // seems like close happened during transmittion
      return 0 ;
    }
    s += s2 ;

    return s ;

  }
}

std::ostream&
operator << ( std::ostream& o, const Packet& p)
{
  const coral::CTLPacketHeader& ctlh = p.ctlHeader() ;
  const coral::CALPacketHeader& calh = p.calHeader() ;
  o << "[Packet type=" << char(p.type()) << " op=" << int(calh.opcode())
    << " size=" << p.dataSize()
    << " ID=<" << (uint32_t)( ctlh.clientID() )
    << ":" << (uint32_t)( ctlh.requestID() )
    << "#" << (uint32_t)( ctlh.segmentNumber() )
    << ( ctlh.moreSegments() ? "+" : "" ) << '>' ;
  if ( calh.cacheable() ) o << " cache=" << calh.cacheableSize() ;

  o << " data=" ;
  o.fill('0') ;
  o.setf ( std::ios::hex, std::ios::basefield ) ;
  const unsigned char* b = p.data() ;
  const unsigned char* e = b + p.dataSize() ;
  for ( ; b != e ; ++ b ) {
    if ( isprint(*b) ) {
      o << *b ;
    } else {
      o << "\\x" << std::setw(2) << (int(*b) & 0xFF) ;
    }
  }
  o.setf ( std::ios::dec, std::ios::basefield ) ;
  o.fill(' ') ;

  return o << ']' ;
}

} // namespace CoralServerProxy
} // namespace coral
