#ifndef CORALSERVERPROXY_CLIENTCONNMANAGER_H
#define CORALSERVERPROXY_CLIENTCONNMANAGER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ClientConnManager.h,v 1.1.2.2 2010/12/20 11:25:41 avalassi Exp $
//
// Description:
//	Class ClientConnManager.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <set>
#include "CoralBase/boost_thread_headers.h"

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "NetSocket.h"
#include "ServerReaderFactory.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Client Connection manager class. Tracks all active client
 *  connections. Thread-safe.
 *
 *  @see ClientWriter, SingleClientReader
 *
 *  @version $Id: ClientConnManager.h,v 1.1.2.2 2010/12/20 11:25:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class ClientConnManager  {
public:

  // Default constructor
  ClientConnManager ( const ServerReaderFactory& serverReaderFactory ) ;

  // Destructor
  ~ClientConnManager () ;

  // add one active connection
  bool addConnection ( NetSocket s ) ;

  // remove one connection
  void removeConnection ( NetSocket s ) ;

  // get server socket
  NetSocket& serverConnection() { return m_serverConn ; }

  // set server socket
  void setServerConnection( NetSocket sock ) { m_serverConn = sock; }

  // close all connections
  void closeAllConnections () ;

protected:

private:

  typedef std::set<NetSocket> Connections ;

  // Data members
  ServerReaderFactory m_serverReaderFactory ;  // upstream server address
  NetSocket m_serverConn ;
  Connections m_connections ;
  boost::mutex m_mutex ;

  // Copy constructor and assignment are disabled by default
  ClientConnManager ( const ClientConnManager& ) ;
  ClientConnManager operator = ( const ClientConnManager& ) ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_CLIENTCONNMANAGER_H
