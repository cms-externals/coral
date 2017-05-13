#ifndef CORALSERVERPROXY_MULTIPARTCOLLECTOR_H
#define CORALSERVERPROXY_MULTIPARTCOLLECTOR_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: MultipartCollector.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class MultipartCollector.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <vector>
#include <map>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "Packet.h"
#include "CoralServerBase/CTLPacketHeader.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Class for the collection of the pieces of multipart packets.
 *  Client should add pieces of packets one by one, in the strict
 *  order. When the last piece is fed into it, client can extract
 *  and delete the whole sequence of packets at once.
 *
 *  @see PacketDispatcher
 *
 *  @version $Id: MultipartCollector.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class MultipartCollector  {
public:

  // all type definitions
  typedef std::pair<unsigned,unsigned> ClientAndRequestIds ;
  typedef std::vector<PacketPtr> PacketPtrList ;
  typedef std::map< ClientAndRequestIds, PacketPtrList > PacketMap ;
  typedef PacketMap::value_type value_type ;
  typedef PacketMap::iterator iterator ;
  typedef PacketMap::const_iterator const_iterator ;

  // Default constructor
  MultipartCollector () ;

  // Destructor
  ~MultipartCollector () ;

  // begin/end iterators
  iterator begin() { return m_packetMap.begin() ; }
  iterator end() { return m_packetMap.end() ; }

  // Add one more part of the multipart packet. It will reject
  // single-part packets, and multi-part packets must come in order.
  // Returns an iterator where packet was inserted and a bool flag
  // which is true if adding was succesful, false otherwise.
  std::pair<iterator,bool> addPacket ( PacketPtr packet ) ;

  // Get the list of already collected packets for the given
  // client ID and request ID (which come from the packet header)
  // Returns end() if there is no such packet ids.
  iterator find ( const CTLPacketHeader& header ) ;

  // remove the entry from the collector, this is supposed to happen
  // after you got the last packet
  void erase ( iterator i ) ;

  // clear all collected stuff
  void clear() ;

protected:

private:

  // Data members
  PacketMap m_packetMap ;

  // assignment is disabled by default
  MultipartCollector operator = ( const MultipartCollector& ) ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_MULTIPARTCOLLECTOR_H
