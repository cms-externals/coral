// $Id: SocketServer.cpp,v 1.12.2.1 2010/12/20 09:10:11 avalassi Exp $

// Include files
#ifndef WIN32
#include <iostream>
#endif
#include <sstream>

#include "CoralSockets/SocketServer.h"
#include "CoralSockets/NonFatalSocketException.h"

// Local include files
#include "ThreadManager.h"
#include "SocketThread.h"
#include "TcpSocket.h"
#ifdef HAVE_OPENSSL
#include "SslSocket.h"
#endif
#include "PacketSocket.h"

// Namespace
using namespace coral::CoralSockets;

#define LOGGER_NAME "CoralSockets::SocketServer"
#include "logger.h"

//-----------------------------------------------------------------------------

SocketServer::SocketServer( coral::IRequestHandlerFactory& handlerFactory,
                            const std::string& host,
                            int port,
                            int nHandlerThreadsPerSocket )
  : m_isActive(true)
  , m_handlerFactory( handlerFactory )
  , m_host( host )
  , m_port( port )
  , m_nHandlerThreadsPerSocket( nHandlerThreadsPerSocket )
  , m_thrManager( new ThreadManager() )
{
  INFO("Create SocketServer on host " << host << " and port " << port);
  if (!nHandlerThreadsPerSocket)
    throw GenericSocketException( "Invalid # handler threads per socket" );
}

//-----------------------------------------------------------------------------

SocketServer::~SocketServer()
{
  std::cout << "Delete SocketServer" << std::endl;
}

//-----------------------------------------------------------------------------

const coral::IThreadMonitor* SocketServer::threadMonitor() const
{
  return m_thrManager.get();
}

//-----------------------------------------------------------------------------

void SocketServer::stopServer()
{
  m_isActive=false;
}

//-----------------------------------------------------------------------------

void SocketServer::run( int timeout ) {
  INFO("Open listening  socket on host " << m_host << " and port " << m_port);
#ifdef HAVE_OPENSSL
  SslSocketPtr listen=SslSocket::listenOn( m_host, m_port );
#else
  TcpSocketPtr listen=TcpSocket::listenOn( m_host, m_port );
#endif

  int tries=0;
  while ( m_isActive && ( timeout < 0  || tries < timeout*2 ) ) {
    m_thrManager->housekeeping();
    // poll if any new clients want to connect
    if ( !listen->poll( TcpSocket::P_READ, 500) ) {
      tries++;
      continue;
    };
    try {
      tries=0;
      PacketSocketPtr cSocket( new PacketSocket( listen->accept() ) );
      SocketThread *cThread =
        new SocketThread( m_handlerFactory.newRequestHandler(), cSocket,
                          m_nHandlerThreadsPerSocket );
      m_thrManager->addThread( cThread );
    }
    catch (NonFatalSocketException &e) {
      INFO("Caught non fatal exception during accept: " << e.what());
    }
    catch (GenericSocketException &e) {
      ERROR("Caught exception during accept: " << e.what());
    };
  };
  INFO( "Close the server socket");
  listen->close();

  INFO("Waiting for client connections to terminate:");

  m_thrManager->joinAll();

  INFO("All client connections are closed");
  INFO("Shutdown the server");
}

//-----------------------------------------------------------------------------
