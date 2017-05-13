#ifndef CORALSERVERPROXY_STATCOLLECTOR_H
#define CORALSERVERPROXY_STATCOLLECTOR_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: StatCollector.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class StatCollector.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <deque>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "Packet.h"
#include "TimeStamp.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Class which collects statistics about data processed by the proxy
 *
 *  @version $Id: StatCollector.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

//
// Helper class for statistics counting
//
template <typename T>
struct StatCollectorBin {

  explicit StatCollectorBin( T v = 0, unsigned c = 0 ) : m_sum(v), m_count(c) {}

  void reset( T v = 0, unsigned c = 0 ) { m_sum = v ; m_count = c ; }

  void accumulate( T v, unsigned c = 1 ) { m_sum += v ; m_count += c ; }

  T mean() const { return m_count == 0 ? T(0) : m_sum/m_count ; }
  double meanDouble() const { return m_count == 0 ? 0.0 : double(m_sum)/m_count ; }
  unsigned count() const { return m_count ; }

private:

  T m_sum ;
  unsigned m_count ;

} ;

struct ClientRequestStat {

  ClientRequestStat ( PacketPtr p ) ;

  void printJSON ( std::ostream& out ) const ;

private:

  TimeStamp m_timestamp ;
  uint32_t m_packetSize ;
  uint32_t m_clientId ;
  uint32_t m_reqId ;
  uint32_t m_seqId ;
  uint8_t m_opcode ;
  bool m_cacheable ;
};

struct ServerReplyStat {

  ServerReplyStat ( PacketPtr p, const CTLPacketHeader& header, bool cached ) ;

  void printJSON ( std::ostream& out ) const ;

private:

  TimeStamp m_timestamp ;
  uint32_t m_packetSize ;
  uint32_t m_clientId ;
  uint32_t m_reqId ;
  uint32_t m_seqId ;
  bool m_cached ;
};


class StatCollector  {
public:

  // Default constructor
  explicit StatCollector ( unsigned statSize = 100000, unsigned autoResetSec = 0 ) ;

  // Destructor
  ~StatCollector () ;

  // change sampling size
  void setStatSize ( unsigned statSize ) ;

  // change auto-reset interval
  void setAutoResetSec ( unsigned autoResetSec ) ;

  // reset all collected statistics
  void clear() ;

  // add info about one more client request packet
  void addClientPacket ( PacketPtr p ) ;

  // add info about one more server reply packet
  void addServerPacket ( PacketPtr p, const CTLPacketHeader& header, bool cached ) ;

  // dump whole stat in JSON format
  void printJSON ( std::ostream& out ) const ;

protected:

  // Compare current time with the last update time and reset
  // whole shebang if necessary. Should be called before every
  // update operation
  void checkResetTime() ;

private:

  typedef std::deque<ClientRequestStat> ClientRequestStatList ;
  typedef std::deque<ServerReplyStat> ServerReplyStatList ;

  // Data members
  unsigned m_statSize ;
  unsigned m_autoResetSec ;
  time_t m_lastUpdateSec ;

  StatCollectorBin<double> m_clientPacketSize ;
  StatCollectorBin<double> m_serverPacketSize ;
  StatCollectorBin<double> m_cachedPacketSize ;
  StatCollectorBin<double> m_nonCachedPacketSize ;

  ClientRequestStatList m_clientRequests ;
  ServerReplyStatList m_serverReplies ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_STATCOLLECTOR_H
