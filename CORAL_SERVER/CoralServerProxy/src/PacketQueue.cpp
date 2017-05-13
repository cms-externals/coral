//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: PacketQueue.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class PacketQueue...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "PacketQueue.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

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
PacketQueue::PacketQueue ( size_t maxSize )
  : m_maxSize ( maxSize )
  , m_queue()
  , m_mutex()
  , m_condFull()
  , m_condEmpty()
{
}

//--------------
// Destructor --
//--------------
PacketQueue::~PacketQueue ()
{
  clear () ;
}

// add one more packet to the queue, if the queue
// is full already then wait until somebody calls pop()
void
PacketQueue::push ( PacketPtr packet )
{
  boost::mutex::scoped_lock qlock ( m_mutex ) ;

  // wait unil we have an empty slot
  while ( m_queue.size() >= m_maxSize ) {
    m_condFull.wait( qlock ) ;
  }

  // store the packet
  m_queue.push ( packet ) ;

  // tell anybody waiting for queue to become non-empty
  m_condEmpty.notify_one () ;

}

// add one more packet to the queue, if the queue
// is full already then wait until somebody calls pop() or timeout
// is expired. If timeout is expeired return false.
bool
PacketQueue::timed_push ( PacketPtr packet, unsigned timeoutSec )
{
  boost::mutex::scoped_lock qlock ( m_mutex ) ;

  // wait unil we have an empty slot
  while ( m_queue.size() >= m_maxSize ) {
    boost::xtime xt;
    boost::xtime_get ( &xt, boost::TIME_UTC ) ;
    xt.sec += timeoutSec;
    if ( not m_condFull.timed_wait( qlock, xt ) ) {
      return false ;
    }
  }

  // store the packet
  m_queue.push ( packet ) ;

  // tell anybody waiting for queue to become non-empty
  m_condEmpty.notify_one () ;

  return true ;
}

// get one packet from the head of the queue, if the queue is
// empty then wait until somebody calls push()
PacketPtr
PacketQueue::pop()
{
  boost::mutex::scoped_lock qlock ( m_mutex ) ;

  // wait unil we have something in the queue
  while ( m_queue.empty() ) {
    m_condEmpty.wait( qlock ) ;
  }

  // get a packet
  PacketPtr p = m_queue.front() ;
  m_queue.pop() ;

  // tell anybody waiting for queue to become non-full
  m_condFull.notify_one () ;

  return p ;
}

// clear the queue
void
PacketQueue::clear()
{
  boost::mutex::scoped_lock qlock ( m_mutex ) ;

  while ( not m_queue.empty() ) {
    m_queue.pop() ;
  }
}

// get current queue size
size_t
PacketQueue::size()
{
  boost::mutex::scoped_lock qlock ( m_mutex ) ;
  return m_queue.size() ;
}

// check queue size
bool
PacketQueue::empty()
{
  boost::mutex::scoped_lock qlock ( m_mutex ) ;
  return m_queue.empty() ;
}

} // namespace CoralServerProxy
} // namespace coral
