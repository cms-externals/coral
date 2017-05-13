//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: QueuedPacketList.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class QueuedPacketList...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "QueuedPacketList.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger.h"

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
QueuedPacketList::QueuedPacketList ()
  : m_queuedPackets()
{
}

//--------------
// Destructor --
//--------------
QueuedPacketList::~QueuedPacketList ()
{
}

// For the given request check if the is an identical request sitting
// already in the queued packet list, return first found
PacketPtr
QueuedPacketList::findIdenticalRequest ( PacketPtr request ) const
{
  for ( QueuedPackets::const_iterator it = m_queuedPackets.begin() ; it != m_queuedPackets.end() ; ++it ) {
    if ( *(*it) == *request ) return *it ;
  }
  return PacketPtr() ;
}

// For the given reply header try to find the corresponding request in
// the list of the queued packets. Returns request packet or empty pointer.
PacketPtr
QueuedPacketList::findRequestForReply ( const CTLPacketHeader& replyHeader ) const
{
  for ( QueuedPackets::const_iterator it = m_queuedPackets.begin() ; it != m_queuedPackets.end() ; ++it ) {
    const CTLPacketHeader& reqHeader = (*it)->ctlHeader() ;
    if ( replyHeader.clientID() == reqHeader.clientID() and
         replyHeader.requestID() == reqHeader.requestID() ) {
      return *it ;
    }
  }
  return PacketPtr() ;
}

// For the given request find all identical requests in the queued packet list,
// remove the from that list and return to the caller
void
QueuedPacketList::getRequests ( PacketPtr request, std::list<PacketPtr>& result, bool remove )
{
  unsigned count = 0 ;
  for ( QueuedPackets::iterator it = m_queuedPackets.begin() ; it != m_queuedPackets.end() ;  ) {
    if ( *(*it) == *request ) {
      QueuedPackets::iterator nxt = it ;
      ++ nxt ;
      result.push_back ( *it ) ;
      if ( remove ) {
        m_queuedPackets.erase ( it ) ;
        ++count ;
      }
      it = nxt ;
    } else {
      ++it ;
    }
  }
  if ( remove ) {
    PXY_DEBUG ( "QueuedPacketList::getRequests: removed " << count
                << " entries, new size = " << m_queuedPackets.size() ) ;
  }
}

// Add request to the list of queued packet
void
QueuedPacketList::add ( PacketPtr request )
{
  m_queuedPackets.push_back( request ) ;
  PXY_DEBUG ( "QueuedPacketList::add: new size = " << m_queuedPackets.size() ) ;
}

// delete all packets coming from the specific client ID
void
QueuedPacketList::deleteClientId ( unsigned id )
{
  PXY_DEBUG ( "QueuedPacketList::deleteClientId: id = " << id ) ;
  unsigned count = 0 ;
  for ( QueuedPackets::iterator it = m_queuedPackets.begin() ; it != m_queuedPackets.end() ;  ) {
    if ( (*it)->ctlHeader().clientID() == id ) {
      QueuedPackets::iterator nxt = it ;
      ++ nxt ;
      m_queuedPackets.erase ( it ) ;
      ++ count ;
      it = nxt ;
    } else {
      ++it ;
    }
  }
  PXY_DEBUG ( "QueuedPacketList::deleteClientId: deleted " << count
              << " entries, new size = " << m_queuedPackets.size() ) ;
}

// clear all queue
void
QueuedPacketList::clear()
{
  m_queuedPackets.clear() ;
}

} // namespace CoralServerProxy
} // namespace coral
