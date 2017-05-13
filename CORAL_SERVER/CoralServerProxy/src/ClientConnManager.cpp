//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ClientConnManager.cpp,v 1.3.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ClientConnManager...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ClientConnManager.h"

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
ClientConnManager::ClientConnManager ( const ServerReaderFactory& serverReaderFactory )
  : m_serverReaderFactory(serverReaderFactory)
  , m_serverConn()
  , m_connections()
  , m_mutex()
{
}

//--------------
// Destructor --
//--------------
ClientConnManager::~ClientConnManager ()
{
}

// add one active connection
bool
ClientConnManager::addConnection ( NetSocket s )
{
  PXY_DEBUG ( "ClientConnManager: adding new connection: " << s ) ;

  // lock the whole thing
  boost::mutex::scoped_lock qlock ( m_mutex ) ;

  // if we are not connected to server yet then do connect and
  // spawn server reader thread
  if ( not m_serverConn.isOpen() ) {

    // open connection to server
    m_serverReaderFactory.serverConnect( *this ) ;
    if ( not m_serverConn.isOpen() ) {
      return false ;
    }

  }

  // just store it in the active connections set
  m_connections.insert( s ) ;

  return true ;
}

// remove one connection
void
ClientConnManager::removeConnection ( NetSocket s )
{
  PXY_DEBUG ( "ClientConnManager: removing connection: " << s ) ;

  // lock the whole thing
  boost::mutex::scoped_lock qlock ( m_mutex ) ;

  // just remove it from the active connections set
  m_connections.erase( s ) ;

  // when all clients disconnect shutdown server connection too
  if ( m_connections.empty() and m_serverConn.isOpen() ) {
    //PXY_DEBUG ( "ClientConnManager: closing server socket" ) ;
    //m_serverConn.shutdown() ;
  }
}

// close all connections
void
ClientConnManager::closeAllConnections ()
{
  // lock the whole thing
  boost::mutex::scoped_lock qlock ( m_mutex ) ;

  // close every socket
  for ( Connections::iterator i = m_connections.begin() ; i != m_connections.end() ; ++ i ) {
    NetSocket s = *i ;
    PXY_DEBUG ( "ClientConnManager: closing connection: " << s ) ;
    s.shutdown() ;
  }
  m_connections.clear() ;

  // shutdown server connection too
  if ( m_serverConn.isOpen() ) {
    PXY_DEBUG ( "ClientConnManager: closing server socket" ) ;
    m_serverConn.shutdown() ;
  }
}

} // namespace CoralServerProxy
} // namespace coral
