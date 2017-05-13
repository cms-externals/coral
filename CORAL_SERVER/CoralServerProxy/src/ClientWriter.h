#ifndef CORALSERVERPROXY_CLIENTWRITER_H
#define CORALSERVERPROXY_CLIENTWRITER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ClientWriter.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ClientWriter.
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
#include "PacketHeaderQueue.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  This class represents a single thread that receives the packets
 *  (server replies) from a queue and sends them to multiple sockets.
 *  Destination socket is determined from the routing tables.
 *  The writer also handles control packets, when it receives the
 *  ServerShutdown packet it closes all client connections using
 *  the ClientConnManager object.
 *
 *  @see ClientConnManager
 *  @see RoutingTables
 *
 *  @version $Id: ClientWriter.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author
 */

namespace coral {
namespace CoralServerProxy {

class RoutingTables ;
class ClientConnManager ;

class ClientWriter  {
public:

  // Default constructor
  ClientWriter ( PacketHeaderQueue& queue, RoutingTables& routing, ClientConnManager& cm ) ;

  // Destructor
  ~ClientWriter () ;

  // this is the "run" method used by the Boost.thread
  void operator() () ;

protected:

private:

  // Data members

  PacketHeaderQueue& m_queue ;  // queue where to put all packets
  RoutingTables& m_routing ;
  ClientConnManager& m_connMgr ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_CLIENTWRITER_H
