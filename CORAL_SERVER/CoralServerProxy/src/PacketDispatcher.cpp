//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: PacketDispatcher.cpp,v 1.8.2.3 2010/05/26 08:21:13 avalassi Exp $
//
// Description:
//	Class PacketDispatcher...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "PacketDispatcher.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <sstream>
#include <algorithm>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "CoralServerBase/CoralServerProxyException.h"
#include "IPacketCache.h"
#include "MsgLogger.h"
#include "Packet.h"
#include "PacketHeaderQueue.h"
#include "PacketQueue.h"
#include "StatCollector.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  using namespace coral ;

  bool str2uint ( const std::string& str, unsigned& u )
  {
    char *eptr = 0 ;
    u = strtoul ( str.c_str(), &eptr, 10 ) ;
    return *eptr == '\0' ;
  }


  std::string opcode2string ( CALOpcode opcode )
  {
    std::string str ;
    str.reserve(64) ;

    str += "Opcode(" ;
    if ( CALOpcodes::isCALRequest(opcode) ) {
      str += "Request|" ;
    } else if ( CALOpcodes::isCALReplyOK(opcode) ) {
      str += "ReplyOK|" ;
      opcode = CALOpcodes::getCALRequest(opcode) ;
    } else if ( CALOpcodes::isCALReplyException(opcode) ) {
      str += "ReplyException|" ;
      opcode = CALOpcodes::getCALRequest(opcode) ;
    } else if ( CALOpcodes::isReserved(opcode) ) {
      str += "Reserved|" ;
    }
    if ( CALOpcodes::isCALRequest(opcode) ) {
      switch ( opcode ) {
        case CALOpcodes::ConnectRO:
          str += "ConnectRO" ;
          break ;
        case CALOpcodes::ConnectRW:
          str += "ConnectRW";
          break ;
        case CALOpcodes::Disconnect:
          str += "Disconnect";
          break ;
        case CALOpcodes::StartTransactionRO:
          str += "StartTransactionRO";
          break ;
        case CALOpcodes::StartTransactionRW:
          str += "StartTransactionRW";
          break ;
        case CALOpcodes::CommitTransaction:
          str += "CommitTransaction";
          break ;
        case CALOpcodes::RollbackTransaction:
          str += "RollbackTransaction";
          break ;
        default:
          str += "Unknown";
          break ;
      }
    } else {
      if ( CALOpcodes::isReserved(opcode) ) opcode &= 0x3f ;
      char buf[16] ;
      snprintf ( buf, sizeof buf,  "%#x", (unsigned int)(opcode) & 0xFF ) ;
      str += buf ;
    }
    str += ")" ;
    return str ;
  }

}


//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace coral {
namespace CoralServerProxy {

//----------------
// Constructors --
//----------------
PacketDispatcher::PacketDispatcher ( PacketQueue& inputq,
                                     PacketQueue& serverq,
                                     PacketHeaderQueue& clientq,
                                     IPacketCache& cache,
                                     StatCollector* statCollector )
  : m_inputq ( inputq )
  , m_serverq ( serverq )
  , m_clientq ( clientq )
  , m_queuedPackets()
  , m_multipartCollector()
  , m_cache ( cache )
  , m_statCollector ( statCollector )
{
}

//--------------
// Destructor --
//--------------
PacketDispatcher::~PacketDispatcher ()
{
}

// this is the "run" method used by the Boost.thread
void
PacketDispatcher::operator() ()
{
  // wait for the next packet to come from the queue
  while ( true ) {

    PacketPtr p = m_inputq.pop() ;
    PXY_DEBUG ( "PacketDispatcher: received new packet " << *p ) ;

    // dispatch it to particular processor
    if ( p->type() == Packet::Request ) {
      processRequest ( p ) ;
    } else if ( p->type() == Packet::Reply ) {
      processReply ( p ) ;
    } else if ( p->type() == Packet::Control ) {
      processControl ( p ) ;
    } else {
      // unexpected something, just get rid of it
      PXY_ERR ( "PacketDispatcher: unknown packet type: " << p->type() ) ;
    }

  }
}

void
PacketDispatcher::processRequest ( PacketPtr p )
{
  const CTLPacketHeader& ctlh = p->ctlHeader() ;
  const CALPacketHeader& calh = p->calHeader() ;

  // get opcode
  CALOpcode opcode = calh.opcode() ;
  PXY_DEBUG ( "PacketDispatcher: received " << ::opcode2string(opcode) ) ;

  // handle special reserved opcodes
  // (including opcodes reserved for proxy-control)
  if ( CALOpcodes::isReserved(opcode) ) {
    processProxyControl ( p ) ;
    return ;
  }

  // gather a bit of statistics about the packets
  if ( m_statCollector ) {
    m_statCollector->addClientPacket ( p ) ;
  }

  // verify that what we got is really a request
  // (else it is a reply - we already checked it is not reserved)
  if ( not CALOpcodes::isCALRequest(opcode) ) {

    // somebody sent a reply to us, send them back an exception
    opcode = CALOpcodes::getCALRequest(opcode) ;
    CALOpcode replyOpcode = CALOpcodes::getCALReplyException(opcode) ;
    const char* msg = "Received Reply packet from client" ;
    CoralServerProxyException exc ( msg, "PacketDispatcher::processRequest" ) ;
    PacketPtr rep = Packet::buildReply ( replyOpcode, ctlh, exc.asCALPayload(), false ) ;
    m_clientq.push ( rep, rep->ctlHeader() ) ;
    return ;
  }

  // the transaction requests and CloseSession requests do not pass through,
  // we return OK for read-only transactions, Exception for update transactions
  if ( opcode == CALOpcodes::Disconnect or
       opcode == CALOpcodes::StartTransactionRO or
       opcode == CALOpcodes::CommitTransaction or
       opcode == CALOpcodes::RollbackTransaction ) {

    // send OK packet, do not forward it to server
    CALOpcode replyOpcode = CALOpcodes::getCALReplyOK(opcode) ;
    PacketPtr rep = Packet::buildReply ( replyOpcode, ctlh, "", false ) ;
    m_clientq.push ( rep, rep->ctlHeader() ) ;
    return ;
  }

  if ( opcode == CALOpcodes::StartTransactionRW ) {

    // we don't allow anything that can update
    CALOpcode replyOpcode = CALOpcodes::getCALReplyException(opcode) ;
    const char* msg = "Update transactions are not supported with proxy" ;
    CoralServerProxyException exc ( msg, "PacketDispatcher::processRequest" ) ;
    PacketPtr rep = Packet::buildReply ( replyOpcode, ctlh, exc.asCALPayload(), false ) ;
    m_clientq.push ( rep, rep->ctlHeader() ) ;
    return ;
  }
  
  if ( opcode == CALOpcodes::ConnectRO ) {
    
    std::string url ( std::string ( (char*)p->data(), p->dataSize() ) ) ;
    if ( url == "proxy:internal" ) {
      // send OK packet, do not forward it to server
      CALOpcode replyOpcode = CALOpcodes::getCALReplyOK(opcode) ;
      PacketPtr rep = Packet::buildReply ( replyOpcode, ctlh, "", false ) ;
      m_clientq.push ( rep, rep->ctlHeader() ) ;
      return ;
    }
  }

  // analyze packet header
  if ( ctlh.segmentNumber() or ctlh.moreSegments() ) {

    // multi-segment requests are not handled now, just send it upstream
    PXY_DEBUG ( "PacketDispatcher: multi-segment request, moving it to server queue" ) ;
    m_serverq.push ( p ) ;

  } else if ( calh.cacheable() ) {

    // packet (or rather response to it) can be cached

    // do some validation, we have already seen some strange stuff coming from the pipe
    if ( calh.cacheableSize() > p->dataSize() ) {

      PXY_ERR ( "PacketDispatcher - malformed packet received (cacheableSize>dataSize): " << *p ) ;

      CALOpcode replyOpcode = CALOpcodes::getCALReplyException(opcode) ;
      const char* msg = "Malformed packet (cacheableSize>dataSize)" ;
      CoralServerProxyException exc ( msg, "PacketDispatcher::processRequest" ) ;
      PacketPtr rep = Packet::buildReply ( replyOpcode, ctlh, exc.asCALPayload(), false ) ;
      m_clientq.push ( rep, rep->ctlHeader() ) ;
      return ;
    }

    // check cache first
    const IPacketCache::PacketPtrList* r = m_cache.find( p ) ;
    if ( r ) {

      // cached already, just send it back to the client
      PXY_DEBUG ( "PacketDispatcher: reply found in cache" ) ;
      for ( IPacketCache::PacketPtrList::const_iterator it = r->begin() ; it != r->end() ; ++ it ) {
        m_clientq.push ( *it, ctlh ) ;
        if ( m_statCollector ) {
          // gather a bit of statistics about the packets
          m_statCollector->addServerPacket ( *it, ctlh, true ) ;
        }
      }

    } else {

      // It's not in the cache yet, try to see if the identical request was
      // already sent to the server
      PacketPtr firstRequest = m_queuedPackets.findIdenticalRequest ( p ) ;
      if ( firstRequest ) {

        // Not in cache yet, but identical request is already processing.
        PXY_DEBUG ( "PacketDispatcher: identical packet is queued already" ) ;
        m_queuedPackets.add ( p ) ;

        // See if there are any parts of the multi-part reply arrived
        MultipartCollector::iterator it = m_multipartCollector.find ( firstRequest->ctlHeader() ) ;
        if ( it != m_multipartCollector.end() ) {

          PXY_DEBUG ( "PacketDispatcher: partial reply received from server" ) ;

          // send all already received parts to client parts
          const MultipartCollector::PacketPtrList& replies = it->second ;
          for ( MultipartCollector::PacketPtrList::const_iterator it2 = replies.begin() ; it2 != replies.end() ; ++ it2 ) {
            m_clientq.push ( *it2, ctlh ) ;
            if ( m_statCollector ) {
              // gather a bit of statistics about the packets
              m_statCollector->addServerPacket ( *it2, ctlh, true ) ;
            }
          }

        }

      } else {

        // no trace yet of the similar packets
        PXY_DEBUG ( "PacketDispatcher: packet not in cache or queue" ) ;
        m_queuedPackets.add ( p ) ;
        m_serverq.push ( p ) ;

      }

    }

  } else {

    // non-cacheable packet, just send it upstream
    PXY_DEBUG ( "PacketDispatcher: packet non-cacheable, moving it to server queue" ) ;
    m_serverq.push ( p ) ;

  }
}

void
PacketDispatcher::processReply ( PacketPtr p )
{
  // See if the request for this reply is in the queued packet list.
  // Queued packet list only contains packets that client wants to be
  // cached. Server may override the cacheable flag of the packets,
  // so even if the server reply is not cacheable we still have to
  // check client requests queue

  const CTLPacketHeader& ctlh = p->ctlHeader() ;

  if ( m_statCollector ) {
    // gather a bit of statistics about the packets
    m_statCollector->addServerPacket ( p, ctlh, false ) ;
  }

  PacketPtr req = m_queuedPackets.findRequestForReply ( ctlh ) ;
  if ( not req ) {

    // nobody expects us here, the request may have been non-cacheable, or maybe multi-part
    PXY_DEBUG ( "PacketDispatcher: no request in the queue, send reply to client" ) ;

    // we don't have to be cached, just send me to the client
    m_clientq.push ( p, ctlh ) ;

  } else {

    // if this is the last packet in the sequence then we'll clear the queue
    // from the requests awaiting it
    bool remove = not ctlh.moreSegments() ;

    // get the list of all requests for whom I'm a good reply
    std::list<PacketPtr> requests ;
    m_queuedPackets.getRequests ( req, requests, remove ) ;
    PXY_DEBUG ( "PacketDispatcher: got " << requests.size() << " requests from queued packets list" ) ;

    // send this reply to every request
    for ( std::list<PacketPtr>::const_iterator it = requests.begin() ; it != requests.end() ; ++ it ) {
      PXY_DEBUG ( "PacketDispatcher: send reply to client, original request: " << *(*it) ) ;
      m_clientq.push ( p, (*it)->ctlHeader() ) ;
    }

    // now if the reply is multi-part add it to the parts collector
    if ( ctlh.segmentNumber() or ctlh.moreSegments() ) {

      std::pair<MultipartCollector::iterator,bool> res = m_multipartCollector.addPacket ( p ) ;
      if ( not res.second ) {

        PXY_ERR ( "PacketDispatcher: failed to add reply part to collector: " << *p ) ;

      } else {

        PXY_DEBUG ( "PacketDispatcher: added reply part to collector: " << *p ) ;

        // if we got the last part them move result to the cache
        if ( remove ) {

          // copy to cache
          const MultipartCollector::PacketPtrList& parts = res.first->second ;
          m_cache.insert ( req, parts ) ;

          // and remove it from collector
          m_multipartCollector.erase( res.first ) ;

        }

      }

    } else {

      // for single-part packet if the reply is cacheable send it to the cache
      if ( p->calHeader().cacheable() ) {
        PXY_DEBUG ( "PacketDispatcher: insert packet into cache: " << *req << " -> " << *p ) ;
        IPacketCache::PacketPtrList replies ( 1, p ) ;
        m_cache.insert ( req, replies ) ;
      }

    }
  }

}

void
PacketDispatcher::processControl ( PacketPtr p )
{
  unsigned opcode = p->calHeader().opcode() ;

  if ( opcode == Packet::ServerShutdown ) {

    // server closed connection, will shutdown all clients too
    PXY_INFO ( "PacketDispatcher : got shutdown packet, clear queued packets list" ) ;
    m_queuedPackets.clear() ;
    m_multipartCollector.clear() ;

    // we also want to make sure that on the next connection to a server
    // a connect packet is sent to the server, so we have to remove all
    // connect packets from a cache
    m_cache.clearConnect() ;

  } else if ( opcode == Packet::ClientDisconnect ) {

    // remove all memory about queued packets coming from this client
    m_queuedPackets.deleteClientId ( p->ctlHeader().clientID() ) ;

  }

  // send it to both client and server queues
  m_serverq.push ( p ) ;
  m_clientq.push ( p, p->ctlHeader() ) ;
}

void
PacketDispatcher::processProxyControl( PacketPtr p )
{
  std::string cmd ( std::string ( (char*)p->data(), p->dataSize() ) ) ;

  PXY_DEBUG ( "PacketDispatcher - got ProxyControl command: \"" << cmd << "\"" ) ;

  if ( cmd.compare( 0, 5, "stat " ) == 0 && not m_statCollector ) {
    // send reply packet
    sendReply ( "FAIL: statistics is not collected", p->ctlHeader() ) ;
    return ;
  }


  if ( cmd == "stat clear" ) {

    // reset statistics
    m_statCollector->clear() ;

    // send reply packet
    sendReply ( "OK", p->ctlHeader() ) ;

  } else if ( cmd.compare( 0, 14, "stat set-size " ) == 0 ) {

    // get the number for the new stat size
    unsigned s ;
    if ( ::str2uint ( std::string(cmd,14), s ) ) {

      m_statCollector->setStatSize ( s ) ;
      // send reply packet
      sendReply ( "OK", p->ctlHeader() ) ;

    } else {

      PXY_ERR ( "PacketDispatcher - invalid number in stat-size: \"" << cmd << "\"" ) ;
      // send reply packet
      sendReply ( "FAIL: invalid number", p->ctlHeader() ) ;

    }

  } else if ( cmd.compare( 0, 20, "stat auto-reset-sec " ) == 0 ) {

    // get the number for the new auto-reset interval
    unsigned s ;
    if ( ::str2uint ( std::string(cmd,20), s ) ) {

      m_statCollector->setAutoResetSec ( s ) ;
      // send reply packet
      sendReply ( "OK", p->ctlHeader() ) ;

    } else {

      PXY_ERR ( "PacketDispatcher - invalid number in auto-reset-sec: \"" << cmd << "\"" ) ;

      // send reply packet
      sendReply ( "FAIL: invalid number", p->ctlHeader() ) ;

    }

  } else if ( cmd == "stat report" ) {

    // dump the stat
    std::ostringstream stream ;
    m_statCollector->printJSON ( stream ) ;

    // send reply packet
    sendReply ( stream.str(), p->ctlHeader() ) ;

  } else {

    PXY_ERR ( "PacketDispatcher - invalid command: \"" << cmd << "\"" ) ;

    // send reply packet
    sendReply ( "invalid command", p->ctlHeader() ) ;
  }

}

void
PacketDispatcher::sendReply( const std::string& res, const CTLPacketHeader& reqHeader )
{
  CALOpcode replyOpcode = CALOpcodes::getCALReplyOK(Packet::ProxyControl & 0x3F) ;
  PacketPtr reply = Packet::buildReply ( replyOpcode, reqHeader, res, false ) ;

  // send the reply to the client queue
  m_clientq.push ( reply, reply->ctlHeader() ) ;
}

} // namespace CoralServerProxy
} // namespace coral
