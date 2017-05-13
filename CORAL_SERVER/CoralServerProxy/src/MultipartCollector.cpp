//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: MultipartCollector.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class MultipartCollector...
//
// Author List:
//Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "MultipartCollector.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace coral {
namespace CoralServerProxy {

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

//----------------
// Constructors --
//----------------
MultipartCollector::MultipartCollector ()
  : m_packetMap()
{
}

//--------------
// Destructor --
//--------------
MultipartCollector::~MultipartCollector ()
{
}

// Add one more part of the multipart packet. It will reject
// single-part packets, and multi-part packets must come in order.
// Returns an iterator where packet was inserted and a bool flag
// which is true if adding was succesful, false otherwise.
std::pair<MultipartCollector::iterator,bool>
MultipartCollector::addPacket ( PacketPtr packet )
{
  assert ( packet ) ;

  const CTLPacketHeader& h = packet->ctlHeader() ;

  // check that packet is multi-part
  if ( h.segmentNumber() == 0 and not h.moreSegments() ) {
    PXY_ERR ( "MultipartCollector::addPacket: attempt to add single-part packet: " << *packet ) ;
    return std::pair<iterator,bool> ( iterator(), false ) ;
  }

  // packet unique key
  ClientAndRequestIds key ( h.clientID(), h.requestID() ) ;

  // try to find it first
  iterator i = m_packetMap.find ( key ) ;
  if ( i == m_packetMap.end() ) {

    // not there yet, must be first packet in a sequence
    if ( h.segmentNumber() != 0 ) {
      PXY_ERR ( "MultipartCollector::addPacket: first multi-part packet with non-zero seq#: " << *packet ) ;
      return std::pair<iterator,bool> ( iterator(), false ) ;
    }

    // good, just add it here
    std::pair<iterator,bool> res = m_packetMap.insert ( value_type( key, PacketPtrList() ) ) ;
    if ( res.second ) {
      PacketPtrList& list = res.first->second ;
      // reserve some space, guess may not be good, but better than nothing
      list.reserve( 64 ) ;
      list.push_back ( packet ) ;
    }

    return res ;

  } else {

    // there are already few packets here
    PacketPtrList& list = i->second ;

    // check that packets come in order
    unsigned packetNum = h.segmentNumber() ;
    if ( packetNum != list.size() ) {
      PXY_ERR ( "MultipartCollector::addPacket: packets out of order, currentsize="
                << list.size() << " packet: " << *packet ) ;
      return std::pair<iterator,bool> ( iterator(), false ) ;
    }

    // just append this packet
    list.push_back ( packet ) ;

    return std::pair<iterator,bool> ( i, true ) ;

  }


}

// Get the list of already collected packets for the given
// client ID and request ID (which come from the packet header)
// Returns end() if there is no such packet ids.
MultipartCollector::iterator
MultipartCollector::find ( const CTLPacketHeader& header )
{
  // packet unique key
  ClientAndRequestIds key ( header.clientID(), header.requestID() ) ;

  return m_packetMap.find ( key ) ;
}

// remove the entry from the collector, this is supposed to happen
// after you got the last packet
void
MultipartCollector::erase ( iterator i )
{
   m_packetMap.erase ( i ) ;
}

// clear all collected stuff
void
MultipartCollector::clear()
{
   m_packetMap.clear() ;
}

} // namespace CoralServerProxy
} // namespace coral
