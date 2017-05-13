//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ClientWriter.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ClientWriter...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ClientWriter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
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
ClientWriter::ClientWriter ( PacketHeaderQueue& queue,
                             RoutingTables& routing,
                             ClientConnManager& cm )
  : m_queue ( queue )
  , m_routing ( routing )
  , m_connMgr ( cm )
{
}

//--------------
// Destructor --
//--------------
ClientWriter::~ClientWriter ()
{
}

// this is the "run" method used by the Boost.thread
void
ClientWriter::operator() ()
{
  while ( true ) {

    // receive next packet from a queue
    const PacketHeaderQueue::value_type& ph = m_queue.pop() ;
    PacketPtr p = ph.first ;
    const CTLPacketHeader& reqHeader = ph.second ;
    PXY_DEBUG ( "ClientWriter: got packet from queue " << *p ) ;

    if ( p->type() == Packet::Control ) {

      /*
      // this is special control packet handled internally
      if ( p->header().opcode() == Packet::ServerShutdown ) {
        // server closed connection, will shutdown all clients too
        PXY_INFO ( "ClientWriter: got shutdown packet, closing all client connections" ) ;
        m_connMgr.closeAllConnections() ;
      }
      */

    } else {

      // regular packet

      // need to restore few things from the  original request - client ID, request ID,
      // and also find the destination where this packet is going to.
      unsigned int sId = reqHeader.clientID() ;
      std::pair<NetSocket,unsigned int> cId = m_routing.getClientId ( sId ) ;
      NetSocket cliSock = cId.first ;
      unsigned int clientId = cId.second ;

      // do we have a route?
      if ( not cliSock.isOpen() ) {
        //PXY_ERR ( "ClientWriter : packet cannot be routed, client went away?" ) ;
      } else {

        // build header, take some parts from the original request
        // and other from a (cached) reply
        const CTLPacketHeader& pHeader = p->ctlHeader() ;
        CTLPacketHeader header( pHeader.status(),
                                pHeader.packetSize(),
                                reqHeader.requestID(),
                                clientId,
                                pHeader.segmentNumber(),
                                pHeader.moreSegments(),
                                pHeader.payloadChecksum() ) ;

        // send it down to the wire
        int s = p->write ( cliSock, header ) ;
        if ( s <= 0 ) {
          PXY_ERR ( "ClientWriter: failed to send data to the client" ) ;
        }

      }
    }

  }
}

} // namespace CoralServerProxy
} // namespace coral
