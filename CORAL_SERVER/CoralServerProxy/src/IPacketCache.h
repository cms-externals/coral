#ifndef CORALSERVERPROXY_IPACKETCACHE_H
#define CORALSERVERPROXY_IPACKETCACHE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: IPacketCache.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class IPacketCache.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <vector>

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
 *  Abstract interface for the packet cache implementations.
 *  Packet cache is basically a mapping from request packet
 *  to a sequence of reply packets (we currently support
 *  multi-packet replies, but no multi-packet requests).
 *  Generally the cache is not thread-safe, client code needs
 *  to protect it in case cache is accessed from more than one
 *  thread. Particlar implementation could be thread-safe,
 *  consult the documentation for implementation classes.
 *
 *  @see PacketDispatcher
 *
 *  @version $Id: IPacketCache.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class IPacketCache  {
public:

  // typedef for the sequence of packets
  typedef std::vector<PacketPtr> PacketPtrList ;

  // Destructor
  virtual ~IPacketCache () ;

  // Try to find existing request in cache,
  // returns reply sequence for it or null pointer if not found
  virtual const PacketPtrList* find ( PacketPtr request ) const = 0 ;

  // Add one cache entry, may delete older entries. It's up to
  // particular implementation's policy which entries get deleted.
  virtual void insert ( PacketPtr request, const PacketPtrList& reply ) = 0 ;

  // Clear the cache, completely remove all entries.
  virtual void clear() = 0 ;

  // Remove all cache entries with the Connect opcode
  virtual void clearConnect () = 0 ;

protected:

  // Default constructor
  IPacketCache () {}

private:

  // Data members

  // Copy constructor and assignment are disabled by default
  IPacketCache ( const IPacketCache& ) ;
  IPacketCache& operator = ( const IPacketCache& ) ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_IPACKETCACHE_H
