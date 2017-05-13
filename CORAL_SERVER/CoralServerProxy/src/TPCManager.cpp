//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: TPCManager.cpp,v 1.2.2.5 2010/12/20 13:23:32 avalassi Exp $
//
// Description:
//	Class TPCManager...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "TPCManager.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <iostream>
#include <cstring>
#include "CoralBase/boost_thread_headers.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "ClientConnManager.h"
#include "ClientWriter.h"
#include "MsgLogger.h"
#include "PacketDispatcher.h"
#include "PacketHeaderQueue.h"
#include "PacketQueue.h"
#include "RoutingTables.h"
#include "ServerReader.h"
#include "ServerWriter.h"
#include "SingleClientReader.h"

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
TPCManager::TPCManager ( const NetEndpointAddress& listenAddress,
                         const NetEndpointAddress& serverAddress,
                         size_t maxQueueSize,
                         unsigned timeout,
                         IPacketCache& cache,
                         StatCollector* statCollector )
  : m_listenAddress( listenAddress )
  , m_serverAddress( serverAddress )
  , m_maxQueueSize( maxQueueSize )
  , m_timeout( timeout )
  , m_cache( cache )
  , m_statCollector( statCollector )
{
  PXY_TRACE ( "TPCManager: listenAddress = " << listenAddress ) ;
  PXY_TRACE ( "TPCManager: serverAddress = " << serverAddress ) ;
}

//--------------
// Destructor --
//--------------
TPCManager::~TPCManager ()
{
}

// this is the "run" method used by the Boost.thread
void
TPCManager::operator() ()
{
  PXY_DEBUG( "TPCManager: starting TPCManager" );

  // we will listen on a specific port and create a new thread
  // for every incoming connection

  // create listening socket
  NetSocket sock = setupListener () ;
  if ( not sock.isOpen() ) return ;

  // setup all three queues for packets
  PacketQueue rcvQueue ( m_maxQueueSize ) ;
  PacketQueue serverQueue ( m_maxQueueSize ) ;
  PacketHeaderQueue clientQueue ( m_maxQueueSize ) ;

  // instantiate routing tables
  RoutingTables routing ;

  // client connection manager
  ServerReaderFactory serverReaderFactory ( m_serverAddress, rcvQueue, m_timeout ) ;
  ClientConnManager connMgr ( serverReaderFactory ) ;

  // start packet dispatcher thread
  PacketDispatcher dispatcher ( rcvQueue, serverQueue, clientQueue, m_cache, m_statCollector ) ;
  boost::thread dispatcherThread( dispatcher ) ;

  // start client writing thread
  boost::thread clientWriterThread( ClientWriter ( clientQueue, routing, connMgr ) ) ;

  // start server writer thread
  boost::thread serverWriterThread( ServerWriter ( serverQueue, connMgr.serverConnection() ) ) ;

  while ( true ) {

    // accept new connections
    NetSocket cli_sock ( sock.accept() ) ;
    if ( not cli_sock.isOpen() ) {
      PXY_ERR ( "TPCManager: accept failed: " << strerror(errno) ) ;
    } else {
      PXY_INFO ( "TPCManager: new client connection " << cli_sock
          << " from " << cli_sock.peer() ) ;
    }

    // add this connection to the connection manager
    // this should happen before we start checking the server connection
    if ( not connMgr.addConnection ( cli_sock ) ) {

      // could not get server connection up, server is probably gone for good.
      // We want to close an incoming connection.
      // We could also send client a special error packet with some
      // specific message or code.

      cli_sock.close() ;

    } else {

      // start new client connection thread
      SingleClientReader reader ( cli_sock, rcvQueue, m_timeout, routing, connMgr ) ;

      // this will spawn the new thread which will run forever.
      // thread object will be destroyed here but the thread itself will
      // live until clientConn() finishes
      PXY_TRACE ( "starting new client thread" ) ;
      boost::thread newClientThread( reader ) ;

    }
  }
  // No path to this line - the loop can only be interrupted by kill or ctrl-c?
  //PXY_DEBUG( "TPCManager: ending TPCManager" );
}

// start listening
NetSocket
TPCManager::setupListener ()
{
  // create socket
  PXY_DEBUG ( "TPCManager: Creating socket" );
  NetSocket sock ( PF_INET, SOCK_STREAM, 0 ) ;
  if ( not sock.isOpen() ) {
    PXY_ERR ( "TPCManager: Failed to create a socket: " << strerror(errno) );
    return sock ;
  }

  // set socket options
  if ( setSocketOptions( sock ) < 0 ) {
    PXY_ERR ( "TPCManager: Failed to set socket options " << sock << ": " << strerror(errno) );
    return NetSocket() ;
  }

  // bind the socket
  PXY_DEBUG( "Binding socket " << sock );
  if ( sock.bind ( m_listenAddress ) < 0 ) {
    PXY_ERR( "TPCManager: Failed to bind the socket: " << strerror(errno) );
    return NetSocket() ;
  }

  // listen on this socket
  PXY_DEBUG( "Listening socket " << sock << ", maxconn = " << SOMAXCONN );
  if ( sock.listen ( SOMAXCONN ) < 0 ) {
    PXY_ERR( "TPCManager: Failed to listen the socket: " << strerror(errno) );
    return NetSocket() ;
  }

  return sock ;
}

int
TPCManager::setSocketOptions( NetSocket& sock )
{
  // reuse the address
  if ( sock.setSocketOptions ( SOL_SOCKET, SO_REUSEADDR, 1 ) < 0 ) {
    return -1 ;
  }
  if ( sock.setSocketOptions ( SOL_SOCKET, SO_KEEPALIVE, 1 ) < 0 ) {
    return -1 ;
  }
  if ( sock.setSocketOptions ( IPPROTO_TCP, TCP_NODELAY, 1 ) < 0 ) {
    return -1 ;
  }
  return 0 ;
}

} // namespace CoralServerProxy
} // namespace coral
