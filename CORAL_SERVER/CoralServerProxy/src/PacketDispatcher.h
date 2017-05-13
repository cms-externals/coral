#ifndef CORALSERVERPROXY_PACKETDISPATCHER_H
#define CORALSERVERPROXY_PACKETDISPATCHER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: PacketDispatcher.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class PacketDispatcher.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MultipartCollector.h"
#include "QueuedPacketList.h"
#include "Packet.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Class which is responsible for dispatching of all incoming
 *  packets from the input queue of the thread-per-connection server.
 *  It is supposed to run in a separate thread.
 *
 *  @see TPCManager
 *
 *  @version $Id: PacketDispatcher.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class IPacketCache;
class Packet ;
class PacketQueue ;
class PacketHeaderQueue ;
class NetSocket ;
class StatCollector ;

class PacketDispatcher  {
public:

  // Default constructor
  PacketDispatcher ( PacketQueue& inputq,
                     PacketQueue& serverq,
                     PacketHeaderQueue& clientq,
                     IPacketCache& cache,
                     StatCollector* statCollector ) ;

  // Destructor
  ~PacketDispatcher () ;

  // this is the "run" method used by the Boost.thread
  void operator() () ;

protected:

  // process single request packet
  void processRequest ( PacketPtr p ) ;

  // process single server reply packet
  void processReply ( PacketPtr p ) ;

  // process internal control packet
  void processControl ( PacketPtr p ) ;

  // process client control command
  void processProxyControl ( PacketPtr p ) ;

  // send a string back to client as a reply
  void sendReply( const std::string& res, const CTLPacketHeader& reqHeader ) ;

private:

  // Data members

  PacketQueue& m_inputq;  // input packet queue
  PacketQueue& m_serverq;  // send to server packet queue
  PacketHeaderQueue& m_clientq;  // send to client packet queue
  QueuedPacketList m_queuedPackets ;  // list of packets currently sent to server
  MultipartCollector m_multipartCollector ;  // packet parts collector
  IPacketCache& m_cache;   // packet cache
  StatCollector* m_statCollector ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_PACKETDISPATCHER_H
