#ifndef CORALSERVERPROXY_ROUTINGTABLES_H
#define CORALSERVERPROXY_ROUTINGTABLES_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: RoutingTables.h,v 1.1.2.2 2010/12/20 11:27:02 avalassi Exp $
//
// Description:
//	Class RoutingTables.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <vector>
#include <map>
#include "CoralBase/boost_thread_headers.h"

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "NetSocket.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Routing tables for the proxy server. Do translation between
 *  client-side ClientID and server-side ClientID. Unique
 *  client-side ID consists of the client socket plus ClientID of
 *  the packet. Unique server-side ID is just a ClientID of the
 *  packet because we only send to a single server. So these tables
 *  represent a one-to-one mapping:
 *     (Socket,ClientID) [client-side] <-> ClientID [server-side]
 *  New server-side ClientID is generated for every unique
 *  (Socket,ClientID) that appears on client side. When client
 *  connection is closed one has to call delSocket() method
 *  to clear all entries related to that socket.
 *
 *  @see SingleConnectionReader
 *
 *  @version $Id: RoutingTables.h,v 1.1.2.2 2010/12/20 11:27:02 avalassi Exp $
 *
 *  @author Andy salnikov
 */

namespace coral {
namespace CoralServerProxy {

class RoutingTables  {
public:

  // Default constructor
  RoutingTables () ;

  // Destructor
  ~RoutingTables () ;

  // get server-side client id for a given socket+clientId
  // if no such socket+clientId exist yet, then creates new routing entry
  // and returns new server-side id
  unsigned int getServerSideId ( NetSocket cliSock, unsigned int clientId ) ;

  // get client-side ids (socket+clientId) for a given server-side id
  // if no such server-side exist then socket returned will be in closed state.
  std::pair<NetSocket,unsigned int> getClientId ( unsigned int serverId ) const ;

  // het the list of all server-side IDs for given socket
  void getServerSideIds ( NetSocket cliSock, std::vector<unsigned>& result ) const ;

  // delete routing info for specific socket+clientId
  void delClientId ( NetSocket cliSock, unsigned int clientId ) ;

  // delete all routing info related to given socket
  void delSocket ( NetSocket cliSock ) ;

protected:

private:

  typedef NetSocket ClientSocket ;
  typedef unsigned int ClientId ;
  typedef unsigned int ServerSideId ;
  typedef std::pair<NetSocket,ClientId> ClientSideId ;

  // mappings
  typedef std::map<ClientSideId,ServerSideId> Client2Server ;
  typedef std::map<ServerSideId,ClientSideId> Server2Client ;

  // Data members
  Client2Server m_client2server ;
  Server2Client m_server2client ;
  NetSocket m_serverSocket ;
  mutable boost::mutex m_mutex ;

  // Copy constructor and assignment are disabled by default
  RoutingTables ( const RoutingTables& ) ;
  RoutingTables operator = ( const RoutingTables& ) ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_ROUTINGTABLES_H
