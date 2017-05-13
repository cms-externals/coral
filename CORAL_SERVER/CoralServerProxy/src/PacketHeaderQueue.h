#ifndef CORALSERVERPROXY_PACKETHEADERQUEUE_H
#define CORALSERVERPROXY_PACKETHEADERQUEUE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: PacketHeaderQueue.h,v 1.1.2.3 2010/12/20 13:23:32 avalassi Exp $
//
// Description:
//	Class PacketHeaderQueue.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <queue>
#include <unistd.h>
#include "CoralBase/boost_thread_headers.h"

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "Packet.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  This class represents synchronized packet+header queue.
 *  Used in cases when you want to send the packet but need
 *  and alternative header for it.
 *
 *  @see ClientWriter
 *
 *  @version $Id: PacketHeaderQueue.h,v 1.1.2.3 2010/12/20 13:23:32 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class PacketHeaderQueue  {
public:

  typedef CTLPacketHeader HeaderType ;
  typedef std::pair<PacketPtr,HeaderType> value_type ;

  // Default constructor
  PacketHeaderQueue ( size_t maxSize ) ;

  // Destructor
  ~PacketHeaderQueue () ;

  // add one more packet to the queue, if the queue
  // is full already then wait until somebody calls pop()
  void push ( PacketPtr packet, const HeaderType& header ) ;

  // add one more packet to the queue, if the queue
  // is full already then wait until somebody calls pop() or timeout
  // is expired. If timeout is expeired return false.
  bool timed_push ( PacketPtr packet, const HeaderType& header, unsigned timeoutSec ) ;

  // get one packet from the head of the queue, if the queue is
  // empty then wait until somebody calls push()
  value_type pop() ;

  // clear the queue
  void clear() ;

  // get current queue size
  size_t size() ;

  // check queue size
  bool empty() ;

protected:

private:


  // Data members
  size_t m_maxSize ;
  std::queue<value_type> m_queue ;
  boost::mutex m_mutex ;
  boost::condition m_condFull ;
  boost::condition m_condEmpty ;

  // Copy constructor and assignment are disabled by default
  PacketHeaderQueue ( const PacketHeaderQueue& ) ;
  PacketHeaderQueue operator = ( const PacketHeaderQueue& ) ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_PACKETHEADERQUEUE_H
