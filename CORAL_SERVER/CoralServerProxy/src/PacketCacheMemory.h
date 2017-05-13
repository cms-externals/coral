#ifndef CORALSERVERPROXY_PACKETCACHEMEMORY_H
#define CORALSERVERPROXY_PACKETCACHEMEMORY_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: PacketCacheMemory.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class PacketCacheMemory.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <queue>
#include <map>

//----------------------
// Base Class Headers --
//----------------------
#include "IPacketCache.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Memory-based packet cache. Keeps all cached data in memory.
 *  Has two parameters which control memory use - maximum total
 *  data volume and number of entries in the cache. This class
 *  is not thread-safe.
 *
 *  @see IPacketCache
 *
 *  @version $Id: PacketCacheMemory.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class PacketCacheMemory : public IPacketCache {
public:

  // Default constructor
  PacketCacheMemory ( unsigned maxCacheSizeMB, unsigned maxPackets ) ;

  // Destructor
  virtual ~PacketCacheMemory () ;

  // Try to find existing request in cache,
  // returns reply sequence for it or null pointer if not found
  virtual const PacketPtrList* find ( PacketPtr request ) const ;

  // Add one cache entry, may delete older entries. It's up to
  // particular implementation's policy which entries get deleted.
  virtual void insert ( PacketPtr request, const PacketPtrList& reply ) ;

  // Clear the cache, completely remove all entries.
  virtual void clear() ;

  // Remove all cache entries with the Connect opcode
  virtual void clearConnect () ;

protected:

  // remove over-limits stuff
  virtual void purge() ;
  virtual void purgeOne() ;

private:

  // functor types for comparison
  struct LessPacketPtr {
    bool operator() ( const PacketPtr& p1, const PacketPtr& p2 ) const {
      return *p1 < *p2 ;
    }
  };

  typedef std::queue<PacketPtr> OrderedKeys ;
  typedef std::map<PacketPtr,PacketPtrList,LessPacketPtr> CacheMap ;

  // Data members
  unsigned m_maxCacheSizeMB ;
  unsigned m_maxNumPackets ;
  unsigned m_cacheSize ;
  OrderedKeys m_orderedKeys ; // keys (request packets) in the order of insertion
  CacheMap m_cacheMap ;       // cache mapping
  CacheMap m_connectCacheMap ; // cache mapping for Connect packets

  // get the cache map for the request
  const CacheMap& cacheMap ( PacketPtr request ) const ;
  CacheMap& cacheMap ( PacketPtr request ) ;

  // Copy constructor and assignment are disabled by default
  PacketCacheMemory ( const PacketCacheMemory& ) ;
  PacketCacheMemory operator = ( const PacketCacheMemory& ) ;
};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_PACKETCACHEMEMORY_H
