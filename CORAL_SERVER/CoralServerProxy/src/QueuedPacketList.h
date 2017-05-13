#ifndef CORALSERVERPROXY_QUEUEDPACKETLIST_H
#define CORALSERVERPROXY_QUEUEDPACKETLIST_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: QueuedPacketList.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class QueuedPacketList.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <list>
#include <map>

//----------------------
// Base Class Headers --
//----------------------
#include "IPacketCache.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "CoralServerBase/CTLPacketHeader.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Class which manages the list of the packets that have been sent to
 *  the server and for which we expect to get a reply.
 *
 *  @see AdditionalClass
 *
 *  @version $Id: QueuedPacketList.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author
 */

namespace coral {
namespace CoralServerProxy {

class QueuedPacketList {
public:

  // Default constructor
  QueuedPacketList () ;

  // Destructor
  ~QueuedPacketList () ;

  // For the given request check if the is an identical request sitting
  // already in the queued packet list, return first found
  PacketPtr findIdenticalRequest ( PacketPtr request ) const ;

  // For the given reply header try to find the corresponding request in
  // the list of the queued packets. Returns request packet or empty pointer.
  PacketPtr findRequestForReply ( const CTLPacketHeader& replyHeader ) const ;

  // For the given request find all identical requests in the queued packet list,
  // optionally remove the them from the list
  void getRequests ( PacketPtr request, std::list<PacketPtr>& result, bool remove ) ;

  // Add request to the list of queued packet
  void add ( PacketPtr request ) ;

  // delete all packets coming from the specific client ID
  void deleteClientId ( unsigned id ) ;

  // clear all queue
  void clear() ;

protected:

private:

  typedef std::list<PacketPtr> QueuedPackets ;

  // Data members
  QueuedPackets m_queuedPackets ;

  // assignment is disabled
  QueuedPacketList operator = ( const QueuedPacketList& ) ;
};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_QUEUEDPACKETLIST_H
