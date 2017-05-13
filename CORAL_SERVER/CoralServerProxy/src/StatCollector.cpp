//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: StatCollector.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class StatCollector...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "StatCollector.h"

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


ClientRequestStat::ClientRequestStat ( PacketPtr p )
  : m_timestamp(p->recvTime())
  , m_packetSize(p->packetSize())
  , m_clientId()
  , m_reqId()
  , m_seqId()
  , m_opcode()
  , m_cacheable()
{
  const CTLPacketHeader& ctlh = p->ctlHeader() ;
  const CALPacketHeader& calh = p->calHeader() ;
  m_clientId = ctlh.clientID() ;
  m_reqId = ctlh.requestID() ;
  m_seqId = ctlh.segmentNumber() ;
  m_opcode = calh.opcode() ;
  m_cacheable = calh.cacheable() ;
}


void
ClientRequestStat::printJSON ( std::ostream& out ) const
{
  out << "[ " ;
  m_timestamp.printJSON ( out ) ;
  out << ", " << m_packetSize ;
  out << ", " << m_clientId ;
  out << ", " << m_reqId ;
  out << ", " << m_seqId ;
  out << ", " << unsigned(m_opcode) ;
  out << ", " << ( m_cacheable ? "true" : "false" ) ;
  out << " ]" ;
}

ServerReplyStat::ServerReplyStat ( PacketPtr p, const CTLPacketHeader& header, bool cached )
  : m_timestamp(p->recvTime())
  , m_packetSize(p->packetSize())
  , m_clientId()
  , m_reqId()
  , m_seqId()
  , m_cached(cached)
{
  const CTLPacketHeader& replyHeader = p->ctlHeader() ;
  m_clientId = header.clientID() ;
  m_reqId = header.requestID() ;
  m_seqId = replyHeader.segmentNumber() ;
}


void
ServerReplyStat::printJSON ( std::ostream& out ) const
{
  out << "[ " ;
  m_timestamp.printJSON ( out ) ;
  out << ", " << m_packetSize ;
  out << ", " << m_clientId ;
  out << ", " << m_reqId ;
  out << ", " << m_seqId ;
  out << ", " << ( m_cached ? "true" : "false" ) ;
  out << " ]" ;
}


//----------------
// Constructors --
//----------------
StatCollector::StatCollector ( unsigned statSize, unsigned autoResetSec )
  : m_statSize( statSize )
  , m_autoResetSec( autoResetSec )
  , m_lastUpdateSec( 0 )
  , m_clientPacketSize()
  , m_serverPacketSize()
  , m_cachedPacketSize()
  , m_nonCachedPacketSize()
  , m_clientRequests()
  , m_serverReplies()
{
}

//--------------
// Destructor --
//--------------
StatCollector::~StatCollector ()
{
}


// change sampling size
void
StatCollector::setStatSize ( unsigned statSize )
{
  m_statSize = statSize ;
}

// change auto-reset interval
void
StatCollector::setAutoResetSec ( unsigned autoResetSec )
{
  m_autoResetSec = autoResetSec ;
}

// reset all collected statistics
void
StatCollector::clear()
{
  m_clientPacketSize.reset() ;
  m_serverPacketSize.reset() ;
  m_clientRequests.clear() ;
  m_serverReplies.clear() ;
}

// add info about one more client request packet
void
StatCollector::addClientPacket ( PacketPtr p )
{
  // auto-expire if necessary
  checkResetTime() ;

  m_clientPacketSize.accumulate ( p->packetSize() ) ;

  while ( m_clientRequests.size() >= m_statSize ) {
    m_clientRequests.pop_front() ;
  }
  m_clientRequests.push_back( ClientRequestStat(p) ) ;
}

// add info about one more server reply packet
void
StatCollector::addServerPacket ( PacketPtr p, const CTLPacketHeader& header, bool cached )
{
  // auto-expire if necessary
  checkResetTime() ;

  unsigned s = p->packetSize() ;
  m_serverPacketSize.accumulate ( s ) ;
  if ( cached ) {
    m_cachedPacketSize.accumulate ( s ) ;
  } else {
    m_nonCachedPacketSize.accumulate ( s ) ;
  }

  while ( m_serverReplies.size() >= m_statSize ) {
    m_serverReplies.pop_front() ;
  }
  m_serverReplies.push_back( ServerReplyStat ( p, header, cached ) ) ;
}

// dump whole stat in JSON format
void
StatCollector::printJSON ( std::ostream& out ) const
{
  out << "{\n" ;
  out << "  \"clientPackets\" : { \"size\" : " << m_clientPacketSize.mean()
    << ", \"count\" : " << m_clientPacketSize.count() << " },\n" ;
  out << "  \"serverPackets\" : { \"size\" : " << m_serverPacketSize.mean()
    << ", \"count\" : " << m_serverPacketSize.count() << " },\n" ;
  out << "  \"cachedPackets\" : { \"size\" : " << m_cachedPacketSize.mean()
    << ", \"count\" : " << m_cachedPacketSize.count() << " },\n" ;
  out << "  \"nonCachedPackets\" : { \"size\" : " << m_nonCachedPacketSize.mean()
    << ", \"count\" : " << m_nonCachedPacketSize.count() << " },\n" ;

  if ( not m_clientRequests.empty() ) {
    out << "  \"clientPackets\" : [" ;
    char sep = ' ' ;
    for ( ClientRequestStatList::const_iterator i = m_clientRequests.begin() ; i != m_clientRequests.end() ; ++ i ) {
      out << sep << "\n    " ;
      i->printJSON(out) ;
      sep = ',' ;
    }
    out << "\n  ],\n" ;
  }

  if ( not m_serverReplies.empty() ) {
    out << "  \"serverPackets\" : [" ;
    char sep = ' ' ;
    for ( ServerReplyStatList::const_iterator i = m_serverReplies.begin() ; i != m_serverReplies.end() ; ++ i ) {
      out << sep << "\n    " ;
      i->printJSON(out) ;
      sep = ',' ;
    }
    out << "\n  ],\n" ;
  }

  out << "}\n" ;
}

// Compare current time with the last update time and reset
// whole shebang if necessary. Should be called before every
// update operation
void
StatCollector::checkResetTime()
{
  if ( m_autoResetSec && m_lastUpdateSec ) {

    time_t nowSec = ::time(0) ;
    if ( time_t(m_lastUpdateSec + m_autoResetSec) < nowSec ) {
      // expire everything
      clear() ;
    }

    // prepare for next run
    m_lastUpdateSec = ::time(0) ;
  }
}

} // namespace CoralServerProxy
} // namespace coral
