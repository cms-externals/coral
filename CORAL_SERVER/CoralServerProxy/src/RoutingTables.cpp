//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: RoutingTables.cpp,v 1.3.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class RoutingTables...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "RoutingTables.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  //unsigned int lastConnId = 0 ;
  unsigned int lastServerId = 0 ;

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace coral {
namespace CoralServerProxy {

//----------------
// Constructors --
//----------------
RoutingTables::RoutingTables ()
  : m_client2server()
  , m_server2client()
  , m_mutex()
{
}

//--------------
// Destructor --
//--------------
RoutingTables::~RoutingTables ()
{
}

// get server-side client id for a given socket+clientId
// if no such socket+clientId exist yet, then creates new routing entry
// and returns new server-side id
unsigned int
RoutingTables::getServerSideId ( NetSocket cliSock, unsigned int clientId )
{
  boost::mutex::scoped_lock slock ( m_mutex ) ;

  // look for existing mapping first
  ClientSideId cid ( cliSock, clientId ) ;
  Client2Server::const_iterator i = m_client2server.find ( cid ) ;
  if ( i != m_client2server.end() ) {
    // already there
    return i->second ;
  } else {
    // make a new entry
    ServerSideId sid = ++ ::lastServerId ;
    m_client2server.insert ( Client2Server::value_type ( cid, sid ) ) ;
    m_server2client.insert ( Server2Client::value_type ( sid, cid ) ) ;
    return sid ;
  }
}

// get client-side ids (socket+clientId) for a given server-side id
// if no such server-side exist then socket returned will be in closed state.
std::pair<NetSocket,unsigned int>
RoutingTables::getClientId ( unsigned int sid ) const
{
  boost::mutex::scoped_lock slock ( m_mutex ) ;

  // try to find it in the mapping
  Server2Client::const_iterator i = m_server2client.find ( sid ) ;
  if ( i != m_server2client.end() ) {
    // found OK
    return i->second ;
  }

  // not found, return non-existing id pair
  return std::pair<NetSocket,unsigned int>() ;
}

// het the list of all server-side IDs for given socket
void
RoutingTables::getServerSideIds ( NetSocket cliSock, std::vector<unsigned>& result ) const
{
  boost::mutex::scoped_lock slock ( m_mutex ) ;

  // clear it first
  result.clear() ;

  // reserve enough space, maybe more tahn enough
  result.reserve ( m_client2server.size() ) ;

  // copy all matching ids
  for ( Client2Server::const_iterator i = m_client2server.begin() ; i != m_client2server.end() ; ++ i ) {
    const ClientSideId& cid = i->first ;
    if ( cid.first == cliSock ) {
      result.push_back( i->second ) ;
    }
  }

}

// delete routing info for specific socket+clientId
void
RoutingTables::delClientId ( NetSocket cliSock, unsigned int clientId )
{
  boost::mutex::scoped_lock slock ( m_mutex ) ;

  // look for existing mapping
  ClientSideId cid ( cliSock, clientId ) ;
  Client2Server::iterator i = m_client2server.find ( cid ) ;
  if ( i != m_client2server.end() ) {
    // get it
    ServerSideId sid = i->second ;

    // destroy reverse mapping
    m_server2client.erase ( sid ) ;

    // destroy direct mapping
    m_client2server.erase ( i ) ;
  }
}

// delete socket and all routing info related to it
void
RoutingTables::delSocket ( NetSocket cliSock )
{
  boost::mutex::scoped_lock slock ( m_mutex ) ;

  // delete client -> server id mapping
  for ( Client2Server::iterator i = m_client2server.begin() ; i != m_client2server.end() ; ) {
    const ClientSideId& cid = i->first ;
    if ( cid.first == cliSock ) {
      Client2Server::iterator j = i ;
      ++ j ;
      m_client2server.erase(i) ;
      i = j ;
    } else {
      ++ i ;
    }
  }

  // delete server -> client id mapping
  for ( Server2Client::iterator i = m_server2client.begin() ; i != m_server2client.end() ; ) {
    const ClientSideId& cid = i->second ;
    if ( cid.first == cliSock ) {
      Server2Client::iterator j = i ;
      ++ j ;
      m_server2client.erase(i) ;
      i = j ;
    } else {
      ++ i ;
    }
  }

}


} // namespace CoralServerProxy
} // namespace coral
