#ifndef CORALSERVERPROXY_SINGLECLIENTREADER_H
#define CORALSERVERPROXY_SINGLECLIENTREADER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: SingleClientReader.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class SingleClientReader.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "NetSocket.h"
#include "PacketQueue.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  The class which represents a thread which reads packets from a
 *  single connection and queues them for a later processing by the
 *  separate thread. When it receives the packet it overwrites the
 *  packet's ClientID based on routing tables, in that way all packets
 *  going to the queue have their unique server-side ClientID.
 *  The thread exits when the socket is closed, and it unregisters
 *  itself in the ClientConnManager at that point.
 *
 *  @see TPCManager
 *
 *  @version $Id: SingleClientReader.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class ClientConnManager ;
class PacketQueue ;
class RoutingTables ;

class SingleClientReader  {
public:

  // Constructor
  SingleClientReader ( NetSocket socket,
                       PacketQueue& queue,
                       unsigned timeoutSec,
                       RoutingTables& routing,
                       ClientConnManager& cm ) ;

  // Destructor
  ~SingleClientReader () ;

  // this is the "run" method used by the Boost.thread
  void operator() () ;

protected:

private:

  // Data members

  NetSocket m_socket;     // socket to read client data
  PacketQueue& m_queue ;  // queue where to put all packets
  unsigned m_timeoutSec ; // timeout value for operations
  RoutingTables& m_routing ;
  ClientConnManager& m_connMgr ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_SINGLECLIENTREADER_H
