#ifndef CORALSERVERPROXY_SERVERWRITER_H
#define CORALSERVERPROXY_SERVERWRITER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ServerWriter.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ServerWriter.
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
 *  This class represents a single thread that receives the packets
 *  from a queue and sends them to a single socket.
 *
 *  @see AdditionalClass
 *
 *  @version $Id: ServerWriter.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author
 */

namespace coral {
namespace CoralServerProxy {

class ServerWriter  {
public:

  // Default constructor
  ServerWriter ( PacketQueue& queue, NetSocket& socket ) ;

  // Destructor
  ~ServerWriter () ;

  // this is the "run" method used by the Boost.thread
  void operator() () ;

protected:

private:

  // Data members

  PacketQueue& m_queue ;
  NetSocket& m_socket;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_SERVERWRITER_H
