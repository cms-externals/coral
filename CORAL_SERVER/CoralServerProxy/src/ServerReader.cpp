//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ServerReader.cpp,v 1.3.2.3 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ServerReader...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ServerReader.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <unistd.h>
#include <errno.h>
#include <string.h>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger.h"
#include "Packet.h"
#include "RoutingTables.h"

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
ServerReader::ServerReader ( ClientConnManager& connManager,
                             PacketQueue& queue,
                             unsigned timeoutSec )
  : m_connManager ( connManager )
  , m_queue ( queue )
  , m_timeoutSec ( timeoutSec )
{
}

//--------------
// Destructor --
//--------------
ServerReader::~ServerReader ()
{
}

// this is the "run" method used by the Boost.thread
void
ServerReader::operator() ()
{
  // the whole purpose of the ServerReader object is to read
  // the data from the client socket and put it into a packet queue

  NetSocket& socket = m_connManager.serverConnection() ;
  while ( socket.isOpen() ) {

    PacketPtr p = Packet::read ( socket, Packet::Reply ) ;
    if ( not p ) break ;

    PXY_DEBUG ( "ServerReader " << socket << ": received new packet " << *p ) ;

    // set a "proxy" flag for every packet
    p->buffer()[23] |= 0x40 ;

    // try to queue it for the processing but don't wait forever
    if ( m_queue.timed_push( p, m_timeoutSec ) ) {

      PXY_DEBUG ( "ServerReader " << socket << ": queued new packet " <<
          *p << ", new queue size = " << m_queue.size() ) ;

    } else {

      // the queue is filled up and nobody cares to clean it up
      // safest thing here is to disconnect all clients at least to let them know
      // that upstream server may be hanging. Another option is to send a reply
      // packet with specific opcode.
      // Consider also cleaning the queue from the packets coming from this connection.
      PXY_ERR ( "ServerReader " << socket << ": timeout (" <<
          m_timeoutSec << "sec) while queuing new packet" ) ;
      break ;

    }
  }

  // Say that server is gone (or we decided to close connection ourself)
  // We also want to close all clients connections so that clients saw
  // server is gone and do something smart. For that we just send special
  // control packet down the stream, and guys below should recognize it.

  PacketPtr p = Packet::buildControl ( Packet::ServerShutdown, 0 ) ;
  PXY_TRACE ( "ServerReader " << socket << ": sending shutdown packet " << *p ) ;
  // try to queue it for the processing but don't wait forever
  if ( not m_queue.timed_push( p, m_timeoutSec ) ) {
    PXY_ERR ( "ServerReader " << socket << ": timeout (" << m_timeoutSec
        << "sec) while queuing control packet" ) ;
  }

  // now close it
  if ( socket.isOpen() ) {
    PXY_TRACE ( "ServerReader " << socket << ": closing socket" ) ;
    if ( socket.close() < 0 ) {
      PXY_ERR ( "ServerReader " << socket << ": socket close failed: " << strerror(errno) ) ;
    }
  }
}

} // namespace CoralServerProxy
} // namespace coral
