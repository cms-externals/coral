#ifndef CORALSERVERPROXY_PACKET_H
#define CORALSERVERPROXY_PACKET_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: Packet.h,v 1.3.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class Packet.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <iosfwd>
#include <time.h>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "CoralServerBase/CALPacketHeader.h"
#include "CoralServerBase/CTLPacketHeader.h"
#include "TimeStamp.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Data packet class. Consists of a header and data parts.
 *  Handles memory for the packet in an efficient way.
 *  Knows how to (de)serialize itself from/to the socket.
 *
 *  @version $Id: Packet.h,v 1.3.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class NetSocket ;

class Packet ;
typedef boost::shared_ptr<Packet> PacketPtr ;

class Packet  {
public:

  enum { HEADERS_SIZE = CTLPACKET_HEADER_SIZE + CALPACKET_HEADER_SIZE } ;

  enum PacketType { Request = 'Q', Reply = 'A', Control = 'C' } ;
  enum ControlOpcode { ServerShutdown, ClientDisconnect } ;
  enum RegularOpcode { ProxyControl = 0xFF } ;

  // constructor from the buffer and possibly timestamp,
  // packet object takes ownership over the buffer which
  // must be allocated with new[]
  Packet ( unsigned char *buf, PacketType type ) ;
  Packet ( unsigned char *buf, PacketType type, const TimeStamp& m_recvTime ) ;

  // Destructor
  ~Packet () ;

  // get packet type
  PacketType type() const { return m_type ; }

  // access CTL header
  CTLPacketHeader& ctlHeader() {
    return *(reinterpret_cast<CTLPacketHeader*>(m_wholePacket)) ;
  }
  const CTLPacketHeader& ctlHeader() const {
    return *(reinterpret_cast<CTLPacketHeader*>(m_wholePacket)) ;
  }

  // access CAL header
  CALPacketHeader& calHeader() {
    return *(reinterpret_cast<CALPacketHeader*>(m_wholePacket+CTLPACKET_HEADER_SIZE)) ;
  }
  const CALPacketHeader& calHeader() const {
    return *(reinterpret_cast<CALPacketHeader*>(m_wholePacket+CTLPACKET_HEADER_SIZE)) ;
  }

  // time when the packet was received
  const TimeStamp& recvTime() const { return m_recvTime ; }

  // returns pointer to payload data
  unsigned char* data() {
    return m_wholePacket + HEADERS_SIZE ;
  }
  const unsigned char* data() const {
    return m_wholePacket + HEADERS_SIZE ;
  }

  // returns pointer to packet data buffer
  unsigned char* buffer() {
    return m_wholePacket ;
  }
  const unsigned char* buffer() const {
    return m_wholePacket ;
  }

  // get the size of the data besides headers
  size_t dataSize() const { return ctlHeader().packetSize() - HEADERS_SIZE ; }

  // get the size of the whole packet
  size_t packetSize() const { return ctlHeader().packetSize() ; }

  // compare two packets for the purpose of caching. they are equal iff:
  // 1. opcode is the same
  // 2. cacheable flag is the same is the same
  // 3. content of the cacheable data in payload is the same for cacheable
  //    packets or whole payload is the same for non-cacheable packets
  bool operator == ( const Packet& other ) const ;

  // compare two packets for the purpose of sorting,
  // only cache-related things are compared for cacheable packets
  bool operator < ( const Packet& other ) const ;

  // Build the reply packet, take clientId, requestId, segNum from request header.
  static PacketPtr buildReply ( CALOpcode opcode,
                                const CTLPacketHeader& reqHdr,
                                const std::string& payload,
                                bool cacheable ) ;

  // Build the control packet, the only thing that makes sense is client ID
  static PacketPtr buildControl( CALOpcode opcode, size_t clientId ) ;

  // Read the packet from socket.
  // Returns new packet object, 0 on EOF or any error.
  static PacketPtr read( NetSocket& socket, PacketType type ) ;

  // Write the packet to socket.
  // Returns number of bytes written, 0 on EOF, negative on error.
  int write ( NetSocket& socket ) ;

  // Write the packet to socket, but use different header.
  // Returns number of bytes written, 0 on EOF, negative on error.
  int write ( NetSocket& socket, const CTLPacketHeader& ctlHeader ) ;

protected:

private:

  // Data members
  PacketType m_type ;   // packet type
  TimeStamp m_recvTime ;
  unsigned char* m_wholePacket ;

};

std::ostream&
operator << ( std::ostream& o, const Packet& p) ;

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_PACKET_H
