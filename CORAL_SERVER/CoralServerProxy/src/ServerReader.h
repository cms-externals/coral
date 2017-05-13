#ifndef CORALSERVERPROXY_SERVERREADER_H
#define CORALSERVERPROXY_SERVERREADER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ServerReader.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ServerReader.
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
#include "ClientConnManager.h"
#include "PacketQueue.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  The class which represents a thread which reads packets from a
 *  server connection and queues them for a later processing by the
 *  separate thread.
 *
 *  @see PacketDispatcher
 *
 *  @version $Id: ServerReader.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class PacketQueue ;
class RoutingTables ;

class ServerReader  {
public:

  // Constructor
  ServerReader ( ClientConnManager& connManager,
                 PacketQueue& queue,
                 unsigned timeoutSec ) ;

  // Destructor
  ~ServerReader () ;

  // this is the "run" method used by the Boost.thread
  void operator() () ;

protected:

private:

  // Data members

  ClientConnManager& m_connManager;     // connection manager
  PacketQueue& m_queue ;  // queue where to put all packets
  unsigned m_timeoutSec ; // timeout value for operations

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_SERVERREADER_H
