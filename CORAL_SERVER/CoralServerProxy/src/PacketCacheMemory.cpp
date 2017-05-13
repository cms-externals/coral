//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: PacketCacheMemory.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class PacketCacheMemory...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "PacketCacheMemory.h"

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

namespace {

  using namespace coral::CoralServerProxy ;

  unsigned replySize ( const IPacketCache::PacketPtrList& reply )
  {
    unsigned result = 0 ;
    for ( IPacketCache::PacketPtrList::const_iterator i = reply.begin() ; i != reply.end() ; ++ i ) {
      PacketPtr p = *i ;
      result += p->packetSize() ;
    }
    return result ;
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
PacketCacheMemory::PacketCacheMemory ( unsigned maxCacheSizeMB, unsigned maxPackets )
  : IPacketCache()
  , m_maxCacheSizeMB(maxCacheSizeMB)
  , m_maxNumPackets(maxPackets)
  , m_cacheSize(0)
  , m_orderedKeys()
  , m_cacheMap()
{
}

//--------------
// Destructor --
//--------------
PacketCacheMemory::~PacketCacheMemory ()
{
}

// Try to find existing request in cache,
// returns reply sequence for it or null pointer if not found
const IPacketCache::PacketPtrList*
PacketCacheMemory::find ( PacketPtr request ) const
{
  const CacheMap& cache = cacheMap( request ) ;

  CacheMap::const_iterator it = cache.find ( request ) ;
  if ( it != cache.end() ) {
    return &(it->second) ;
  }
  return 0 ;
}

// Add one cache entry, may delete older entries. It's up to
// particular implementation's policy which entries get deleted.
void
PacketCacheMemory::insert ( PacketPtr request, const PacketPtrList& reply )
{
  CacheMap& cache = cacheMap( request ) ;

  // check that there is no cache entry yet
  CacheMap::iterator i = cache.find ( request ) ;
  if ( i != cache.end() ) {
    PXY_ERR ( "PacketCacheMemory::insert: there is an identical entry in the cache already" ) ;
    return ;
  }

  // add it to the map
  cache.insert ( CacheMap::value_type ( request, reply ) ) ;

  // and non-connect packets to the ordered list and calculate new cache size
  if ( request->calHeader().opcode() != CALOpcodes::ConnectRO ) {

    m_orderedKeys.push ( request ) ;

    // calcuate cache size increase
    size_t keySize = request->packetSize() ;
    size_t resSize = ::replySize ( reply ) ;

    m_cacheSize += keySize + resSize ;

    PXY_DEBUG ( "PacketCacheMemory::insert: new cache size: " << m_cacheMap.size() ) ;
    PXY_DEBUG ( "PacketCacheMemory::insert: new cache volume: " << m_cacheSize/(1024*1024) << "MB" ) ;

    // might need to purge older stuff
    purge() ;

  }
}

// Clear the cache, completely remove all entries.
void
PacketCacheMemory::clear()
{

  // first delete all cache entries
  m_cacheMap.clear() ;
  m_connectCacheMap.clear() ;
  m_cacheSize = 0 ;

  // and all ordered stuff too
  while ( not m_orderedKeys.empty() ) {
    m_orderedKeys.pop() ;
  }

}

// Remove all cache entries with the Connect opcode
void
PacketCacheMemory::clearConnect()
{
  m_connectCacheMap.clear() ;
}

// remove over-limits stuff
void
PacketCacheMemory::purge()
{
  while ( m_cacheMap.size() > m_maxNumPackets ) {

    if ( m_orderedKeys.empty() ) {
      PXY_ERR ( "PacketCacheMemory::purge: inconsistent data -- cache size="
                << m_cacheMap.size() << " while keys are empty" ) ;
      break ;
    }

    // remove one entry
    purgeOne() ;

  }

  while ( not m_cacheMap.empty() and m_cacheSize > m_maxCacheSizeMB*(1024*1024) ) {

    if ( m_orderedKeys.empty() ) {
      PXY_ERR ( "PacketCacheMemory::purge: inconsistent data -- cache size="
                << m_cacheMap.size() << " while keys are empty" ) ;
      break ;
    }

    // remove one entry
    purgeOne() ;

  }
}

void
PacketCacheMemory::purgeOne()
{
  // this will be the key to remove
  PacketPtr key = m_orderedKeys.front() ;
  m_orderedKeys.pop() ;

  // find key in a map
  CacheMap::iterator i = m_cacheMap.find ( key ) ;
  if ( i == m_cacheMap.end() ) {
    PXY_ERR ( "PacketCacheMemory::purge: inconsistent data -- key is not in the cache" );
    return ;
  }

  // result size
  key = i->first ;
  const PacketPtrList& res = i->second ;
  size_t keySize = key->packetSize() ;
  size_t resSize = ::replySize ( res ) ;

  // remove the cache entry
  m_cacheMap.erase ( i ) ;

  // reduce the cached size of the cache
  m_cacheSize -= std::min ( (unsigned int)keySize, m_cacheSize ) ;
  m_cacheSize -= std::min ( (unsigned int)resSize, m_cacheSize ) ;

  PXY_ERR ( "PacketCacheMemory::purge: new cache size: " << m_cacheMap.size() ) ;
  PXY_ERR ( "PacketCacheMemory::purge: new cache volume: " << m_cacheSize/(1024*1024) << "MB" ) ;

}

// get the cache map for the request
const PacketCacheMemory::CacheMap&
PacketCacheMemory::cacheMap ( PacketPtr request ) const
{
  if ( request->calHeader().opcode() == CALOpcodes::ConnectRO ) {
    return m_connectCacheMap ;
  } else {
    return m_cacheMap ;
  }
}

PacketCacheMemory::CacheMap&
PacketCacheMemory::cacheMap ( PacketPtr request )
{
  if ( request->calHeader().opcode() == CALOpcodes::ConnectRO ) {
    return m_connectCacheMap ;
  } else {
    return m_cacheMap ;
  }
}


} // namespace CoralServerProxy
} // namespace coral
