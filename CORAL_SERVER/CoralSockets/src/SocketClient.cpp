// $Id: SocketClient.cpp,v 1.7.2.1 2010/12/20 09:10:11 avalassi Exp $

// Include files
#include "CoralSockets/SocketClient.h"


#include "SocketRequestHandler.h"
#include "TcpSocket.h"
#ifdef HAVE_OPENSSL
#include "SslSocket.h"
#endif

// Namespace
using namespace coral::CoralSockets;

#define LOGGER_NAME "CoralSockets::SocketClient"
#include "logger.h"

SocketClient::SocketClient( const std::string& host, int port )
{
  INFO("Connect to host " << host << " and port " << port );

  PacketSocketPtr socket = PacketSocketPtr(
#ifdef HAVE_OPENSSL
                                           new PacketSocket( SslSocket::connectTo( host, port ) )
#else
                                           new PacketSocket( TcpSocket::connectTo( host, port ) )
#endif
                                           );
  INFO("Connection established");

  m_handler =  new SocketRequestHandler( socket );
}

SocketClient::~SocketClient()
{
  INFO("SocketClient destructor");
  delete m_handler;
}

coral::IRequestHandler* SocketClient::getRequestHandler()
{
  return m_handler;
}
