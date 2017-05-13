//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: SingleClientReader.cpp,v 1.6.2.1 2010/05/25 12:53:22 avalassi Exp $
//
// Description:
//	Class SingleClientReader...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "SingleClientReader.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <unistd.h>
#include <errno.h>
#include <string.h>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "CoralServerBase/CoralServerProxyException.h"
#include "ClientConnManager.h"
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
SingleClientReader::SingleClientReader ( NetSocket socket,
                                         PacketQueue& queue,
                                         unsigned timeoutSec,
                                         RoutingTables& routing,
                                         ClientConnManager& cm )
  : m_socket ( socket )
  , m_queue ( queue )
  , m_timeoutSec ( timeoutSec )
  , m_routing ( routing )
  , m_connMgr ( cm )
{
}

//--------------
// Destructor --
//--------------
SingleClientReader::~SingleClientReader ()
{
}

// this is the "run" method used by the Boost.thread
void
SingleClientReader::operator() ()
{
  // the whole pupose of the SingleClientReader object is to read
  // the data from the client socket and put it into a packet queue

  bool connect_seen = false ;
  while ( true ) {

    PacketPtr p = Packet::read ( m_socket, Packet::Request ) ;
    if ( not p ) break ;

    PXY_DEBUG ( "SingleClientReader " << m_socket << ": received new packet " << *p );

    // the very first packet on the connection must be a ConnectRO packet
    if ( not connect_seen ) {

      CALOpcode opcode = p->calHeader().opcode() ;
      if ( opcode != CALOpcodes::ConnectRO ) {

        // send error packet
        const char* err = 0 ;
        if ( opcode == CALOpcodes::ConnectRW ) {
          err = "Protocol error: ConnectRW: proxy does not support update sessions" ;
        } else {
          err = "Protocol error: first packet is not a Connect request" ;
        }

        CoralServerProxyException exc ( err, "SingleClientReader::operator()" ) ;
        PacketPtr reply = Packet::buildReply ( CALOpcodes::getCALReplyException(opcode&0x3F),
                                               p->ctlHeader(),
                                               exc.asCALPayload(),
                                               p->calHeader().cacheable() ) ;
        reply->write ( m_socket ) ;

        // stop here, client is confused enough already
        break ;
      }

      connect_seen = true ;
    }

    // before sending it down for further processing we need to make sure
    // that client ID of this new packet is unique (only do this when
    // routing tables were given to us).
    uint32_t cId = p->ctlHeader().clientID() ;
    uint32_t newId = m_routing.getServerSideId ( m_socket, cId ) ;
    p->ctlHeader().setClientID( newId ) ;
    PXY_DEBUG ( "SingleClientReader " << m_socket << ": set packet client id = " << newId );

    // also set a "proxy" flag for every packet
    p->calHeader().setFromProxy( true ) ;

    // try to queue it for the processing but don't wait forever
    if ( m_queue.timed_push( p, m_timeoutSec ) ) {

      PXY_DEBUG ( "SingleClientReader " << m_socket << ": queued new packet "
          << *p << ", new queue size = " << m_queue.size() );

    } else {

      // the queue is filled up and nobody cares to clean it up
      // safest thing here is to disconnect all clients at least to let them know
      // that upstream server may be hanging. Another option is to send a reply
      // packet with specific opcode.
      // Consider also cleaning the queue from the packets coming from this connection.
      PXY_ERR ( "SingleClientReader " << m_socket << ": timeout ("
          << m_timeoutSec << "sec) while queuing new packet" );
      break ;

    }
  }

  // for every server-side id that we produced signal everybody else
  // that this client is gone
  std::vector<unsigned> serverIds ;
  m_routing.getServerSideIds ( m_socket, serverIds ) ;
  for ( std::vector<unsigned>::const_iterator i = serverIds.begin() ; i != serverIds.end() ; ++i ) {

    PacketPtr p = Packet::buildControl ( Packet::ClientDisconnect, *i ) ;
    PXY_TRACE ( "SingleClientReader " << m_socket << ": sending disconnect packet " << *p );
    // try to queue it for the processing but don't wait forever
    if ( not m_queue.timed_push( p, m_timeoutSec ) ) {
      PXY_ERR ( "SingleClientReader " << m_socket << ": timeout ("
          << m_timeoutSec << "sec) while queueing control packet" );
    }

  }

  // as we are done with this socket, delete all routing info for it
  PXY_DEBUG ( "SingleClientReader " << m_socket << ": delete routing info for socket" );
  m_routing.delSocket ( m_socket ) ;

  // remove socket from active connections
  m_connMgr.removeConnection ( m_socket ) ;

  // now close it (if it was not closed already)
  PXY_TRACE ( "SingleClientReader " << m_socket << ": closing socket" );
  if ( m_socket.close() < 0 ) {
    PXY_ERR ( "SingleClientReader " << m_socket << ": socket close failed: " << strerror(errno) );
  }

}

} // namespace CoralServerProxy
} // namespace coral
