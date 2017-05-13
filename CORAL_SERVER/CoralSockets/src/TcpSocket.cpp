// $Id: TcpSocket.cpp,v 1.19.2.8 2011/09/16 13:29:04 avalassi Exp $

// Include files
#include <cstring>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> /* for TCP_NODELAY */
#include <sys/errno.h>
#include <sys/socket.h>
#include "CoralBase/boost_thread_headers.h"
#include "CoralMonitor/StatsTypeCounter.h"
#include "CoralSockets/GenericSocketException.h"
#include "CoralSockets/NonFatalSocketException.h"

// Local include files
#include "TcpSocket.h"

// Logger
#define LOGGER_NAME "CoralSockets::TcpSocket"
#include "logger.h"

// Debug
#undef DEBUG
#define DEBUG( out )

// Namespace
using namespace coral::CoralSockets;

//-----------------------------------------------------------------------------

coral::StatsTypeCounter myTcpSocketConnectionCounter( "CSockets_Connections",
                                                      "connection" );

//-----------------------------------------------------------------------------

TcpSocket::TcpSocket( int fd, const std::string& desc )
  : m_fd( fd )
  , m_desc( desc )
  , m_sendBytes( 0 )
  , m_recBytes( 0 )
{
  INFO( "TcpSocket constructor '" << m_desc << "'" );
  configureSocket();
}

//---------------------------------------------------------------------------

TcpSocket::~TcpSocket()
{
  INFO( "TcpSocket destructor '" << m_desc << "'" );
  if ( isOpen() ) {
    DEBUG( "Closing socket in destructor" );
    close();
  };
  INFO( "TcpSocket '" << m_desc
        << "' rec: " << (float)m_recBytes /1024/1024 << "MB "
        << " send: " << (float)m_sendBytes /1024/1024 << "MB " );
  if ( getenv("CORALSERVER_STATS") )
    std::cout << "TcpSocket '" << m_desc
              << "' rec: " << (float)m_recBytes /1024/1024 << "MB "
              << " send: " << (float)m_sendBytes /1024/1024 << "MB "
              << std::endl;
}

//---------------------------------------------------------------------------

const std::string& TcpSocket::desc() const
{
  return m_desc;
}

//---------------------------------------------------------------------------

void TcpSocket::setSockOpt( int level, int option, int value )
{
  if ( setsockopt( m_fd, level, option, &value, sizeof( value ) ) < 0 )
    throw GenericSocketException( "Error setting socket option.",
                                  "TcpSocket::setSockOpt" );
}

//---------------------------------------------------------------------------

void TcpSocket::setFdFlags( int getCMD, int setCMD, int flags )
{
  int oflags = fcntl( m_fd, getCMD, 0 );
  if (oflags < 0 )
    throw GenericSocketException( "Error getting fd flags.",
                                  "TcpSocket::setFdFlags" );
  if ( fcntl( m_fd, setCMD, oflags | flags ) < 0 )
    throw GenericSocketException( "Error setting fd flags.",
                                  "TcpSocket::setFdFlags" );
}

//---------------------------------------------------------------------------

void TcpSocket::clearFdFlags( int getCMD, int setCMD, int flags )
{
  int oflags = fcntl( m_fd, getCMD, 0 );
  if (oflags < 0 )
    throw GenericSocketException( "Error getting fd flags.",
                                  "TcpSocket::clearFdFlags" );
  if ( fcntl( m_fd, setCMD, oflags & ~flags ) < 0 )
    throw GenericSocketException( "Error setting fd flags.",
                                  "TcpSocket::clearFdFlags" );
}

//-------------------------------------------------------------------------

void TcpSocket::setNonBlocking()
{
  setFdFlags( F_GETFL, F_SETFL, O_NONBLOCK );
}

//-------------------------------------------------------------------------

void TcpSocket::unsetNonBlocking()
{
  clearFdFlags( F_GETFL, F_SETFL, O_NONBLOCK );
}

//-------------------------------------------------------------------------

void TcpSocket::configureSocket()
{
  /*
  // now already done in listenOn
  // fast reuse enable
  setSockOpt( SOL_SOCKET, SO_REUSEADDR, 1 );
  */
  // keepalive enable
  setSockOpt( SOL_SOCKET, SO_KEEPALIVE, 1 );
#ifdef __APPLE__
  // don't send broken pipe signal
  setSockOpt( SOL_SOCKET, SO_NOSIGPIPE, 1 );
  // For Linux this is done on per message basis ( see read_write() )
#endif // __APPLE__
  // close on exec
  setFdFlags( F_GETFD, F_SETFD, FD_CLOEXEC );
  // non-blocking mode
  setNonBlocking();
#ifdef __APPLE__
  // set no delay option, cork/uncork is only supported on Linux
  setSockOpt( IPPROTO_TCP, TCP_NODELAY, 1 );
#endif // __APPLE__
  //int value = 1;
  //::setsockopt( m_cSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value) );
}

//-------------------------------------------------------------------------

void TcpSocket::cork()
{
#ifndef __APPLE__
  setSockOpt( SOL_TCP, TCP_CORK, 1);
#endif
}

//-------------------------------------------------------------------------

void TcpSocket::uncork()
{
#ifndef __APPLE__
  setSockOpt( SOL_TCP, TCP_CORK, 0);
#endif
}

//-------------------------------------------------------------------------

void TcpSocket::fillAddr( const std::string& hostName,
                          int port,
                          sockaddr_in& addr )
{
  struct addrinfo hints, *res0;
  int error;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo( hostName.c_str(), NULL , &hints, &res0);
  if (error) {
    throw GenericSocketException( gai_strerror(error), "TcpSocket::fillAddr" );
  }
  if ( res0 == 0 )
    throw GenericSocketException("Failed to resolve"
                                 " name (gethostbyname( '" + hostName + "'))",
                                 "TcpSocket::fillAddr" );
  memcpy(&addr, res0->ai_addr, sizeof( sockaddr_in ) );
  addr.sin_port = htons(port);
  freeaddrinfo(res0);
#if 0
  memset(&addr, 0, sizeof(addr)); // Zero out address structure
  addr.sin_family = AF_INET; // Internet address
  hostent* host; // Resolve name
  static boost::mutex gethostbynameMutex;
  boost::mutex::scoped_lock lock(gethostbynameMutex);
  if ( ( host = gethostbyname(hostName.c_str()) ) == NULL ) {
    throw GenericSocketException( "Failed to resolve"
                                  " name (gethostbyname( '" + hostName + "'))",
                                  "TcpSocket::fillAddr" );
  }
  addr.sin_addr.s_addr = *( (unsigned long*)host->h_addr_list[0] );
  addr.sin_port = htons(port); // Assign port in network byte order
#endif
}

//--------------------------------------------------------------------------

int TcpSocket::connectTo( std::string& desc,
                          const std::string& host,
                          unsigned int port )
{
  sockaddr_in addr;
  fillAddr( host, port, addr);
  int fd = socket( PF_INET, SOCK_STREAM, 0 );
  if ( fd < 0 )
    throw GenericSocketException( "Error creating socket.",
                                  "TcpSocket::connectTo" );
  if ( ::connect( fd, (sockaddr*)&addr, sizeof( addr ) ) < 0 )
  {
    std::stringstream str;
    str << "Connection failed: " << strerror( errno );
#ifdef WIN32
    ::closesocket( fd );
#else
    ::close( fd );
#endif
    throw GenericSocketException( str.str(), "TcpSocket::connectTo" );
  }
  sockaddr_in laddr;
  memset(&laddr, 0, sizeof( laddr ));
  socklen_t llen=sizeof( laddr );
  if ( getsockname( fd, (sockaddr*)&laddr, &llen ) < 0 )
    ERROR( "Could not get local port!" );
  std::stringstream desc_str;
  desc_str << "connectTo: "<< host <<":"<< port << " from port "
           << ntohs(laddr.sin_port);
  desc=desc_str.str();
  return fd;
}

//----------------------------------------------------------------------

TcpSocketPtr TcpSocket::connectTo( const std::string& host, unsigned int port )
{
  std::string desc;
  int fd=connectTo( desc, host, port );
  return TcpSocketPtr( new TcpSocket( fd, desc ) );
}

//----------------------------------------------------------------------

int TcpSocket::listenOn( std::string& desc,
                         const std::string& ip,
                         unsigned int port )
{
  sockaddr_in addr;
  fillAddr( ip, port, addr);
  int fd = socket( PF_INET, SOCK_STREAM, 0 );
  if ( fd < 0 )
    throw GenericSocketException( "Error creating socket.",
                                  "TcpSocket::listenOn" );
  int value = 1;
  if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value) ) < 0 )
    throw GenericSocketException( "Error setting SO_REUSEADDR on socket!",
                                  "TcpSocket::listenOn" );
  memset( addr.sin_zero,'\0', sizeof( addr.sin_zero ) );
  if ( bind( fd, (struct sockaddr*) &addr, sizeof( addr ) ) < 0 )
  {
    std::stringstream str;
    str << "Error binding socket: " << strerror( errno );
#ifdef WIN32
    ::closesocket( fd );
#else
    ::close( fd );
#endif
    throw GenericSocketException( str.str(), "TcpSocket::listenOn" );
  }
  int backlog=100;
  if ( listen( fd, backlog ) < 0 )
  {
    std::stringstream str;
    str << "Listen on socket failed: " << strerror( errno );
#ifdef WIN32
    ::closesocket( fd );
#else
    ::close( fd );
#endif
    throw GenericSocketException( str.str(), "TcpSocket::listenOn" );
  }
  std::stringstream desc_str;
  desc_str << "listenOn: "<< ip <<":"<< port;
  desc=desc_str.str();
  return fd;
}

//----------------------------------------------------------------------

TcpSocketPtr TcpSocket::listenOn( const std::string& ip, unsigned int port )
{
  std::string desc;
  int fd = listenOn( desc, ip, port );
  return TcpSocketPtr( new TcpSocket( fd, desc ) );
}

//----------------------------------------------------------------------

int TcpSocket::accept( std::string& desc )
{
  sockaddr_in cAddrStruct;
#ifdef WIN32
  int cAddrStructSize = sizeof(cAddrStruct);
#else
  unsigned int cAddrStructSize = sizeof(cAddrStruct);
#endif
  int cSocket;
  cSocket = ::accept( m_fd,
                      (struct sockaddr*)& cAddrStruct,
                      &cAddrStructSize );
  if ( cSocket < 0 )
  {
    switch (errno)
    {
      // non fatal errors (emitt warning and otherwise ignore)
    case EAGAIN:
      throw NonFatalSocketException( "Warning accept failed with non fatal error" );
      // TCP Errors on accept, the man page suggests to treat them
      // like EAGAIN
    case ENETDOWN:
    case EPROTO:
    case ENOPROTOOPT:
    case EHOSTDOWN:
      //case ENONET:
    case EHOSTUNREACH:
    case EOPNOTSUPP:
    case ENETUNREACH:
      throw NonFatalSocketException( "Warning accept failed with non fatal tcp error" );
    default:
      std::ostringstream str;
      str <<  "Client socket accept() failed :" << strerror( errno ) ;
      throw GenericSocketException( str.str() ,
                                    "TcpSocket::accept" );
    }
  }
  // A client has succesfully connected
  std::string cHost = inet_ntoa( cAddrStruct.sin_addr );
  int cPort = ntohs( cAddrStruct.sin_port );
  INFO( "New client connection from "
        << "( " << cHost << ", " << cPort << " )" );
  std::stringstream desc_str;
  desc_str << "accept from " << cHost << ":" << cPort;
  desc=desc_str.str();
  return cSocket;
}

//----------------------------------------------------------------------

TcpSocketPtr TcpSocket::accept()
{
  std::string desc;
  int fd=accept( desc );
  myTcpSocketConnectionCounter.increase();
  return TcpSocketPtr( new TcpSocket( fd, desc ) );
}

//----------------------------------------------------------------------

void TcpSocket::readAll( unsigned char* buf, size_t len )
{
  size_t res = read_write( P_READ, buf, len, -1 );
  if ( res != len )
    throw GenericSocketException( "Panic! readAll() could not read all bytes!" );
}

//----------------------------------------------------------------------

void TcpSocket::writeAll( const unsigned char* buf, size_t len )
{
  size_t res = read_write( P_WRITE, const_cast<unsigned char*>(buf), len, -1 );
  if ( res != len )
    throw GenericSocketException( "Panic! writedAll() could not write all bytes!" );
}

//----------------------------------------------------------------------

size_t TcpSocket::read( unsigned char* buf, size_t len, int timeout )
{
  return read_write( P_READ, buf, len, timeout );
}

//----------------------------------------------------------------------

size_t TcpSocket::write( const unsigned char* buf, size_t len, int timeout )
{
  return read_write( P_WRITE, const_cast<unsigned char*>(buf), len, timeout );
}

//----------------------------------------------------------------------

size_t TcpSocket::read_write( PMode mode,
                              unsigned char* buf,
                              size_t len,
                              int timeout )
{
  if ( mode != P_READ && mode != P_WRITE )
    throw GenericSocketException( "Panic! Wrong mode in read_write!" );
  const char* label = (mode==P_READ) ? "TcpSocket::read" : "TcpSocket::write";
  DEBUG( label << ":" << m_desc << " enter read_write timeout " << timeout);
  if ( m_fd < 0 ) throw SocketClosedException( label );
#ifndef __APPLE__
  static const int flags = MSG_NOSIGNAL;
#else
  static const int flags = 0;
  // flag SO_NOSIGPIPE gives stranges results on Mac Os
#endif
  unsigned char* p = buf;
  size_t read = 0;
  while ( len > read )
  {
    DEBUG( label << ":" << m_desc << " len " << len
           << " read " << read <<" p " << (int)p );
    if( timeout != 0 )
    {
      // wait until we can read...
      if (timeout < 0 )
        // no timeout set, we sleep until data is available,
        // but don't return unless we got all data
        poll( mode, 200 );
      else if ( !poll( mode, timeout ) )
      {
        DEBUG( label << ": " << m_desc << " timeout" );
        // timeout occured, return
        break;
      }
    }
    // has the socket been closed?
    if ( m_fd < 0 ) throw SocketClosedException( label );
    ssize_t nResult=0;
    if ( mode == P_READ)
      nResult = ::recv( m_fd, p, len-read, flags);
    else
      nResult = ::send( m_fd, p, len-read, flags);
    if ( nResult < 0 )
    {
      DEBUG( label << ":" << m_desc << " nResult < 0 "
             << errno << " " << strerror(errno) );
      // something is wrong, find out what
      if ( errno == EINTR )
        // try again on interrupt
        continue;
      if ( errno == EAGAIN )
      {
        DEBUG( label << ":" << m_desc <<" EAGAIN this should not happen?"  );
        // no data available
        // as long as we poll, this should not happen...
        if (timeout == 0) break;
        continue;
      }
      if ( errno == EPIPE || errno == ENOTCONN || errno == ECONNRESET )
      {
        DEBUG( label << ":" << m_desc <<"Error : " << strerror( errno ) );
        close();
        // connection closed
        throw SocketClosedException( label );
      }
      if ( errno == EBADF )
      {
        // someone else seems to have closed the descriptor. Hmm...
        INFO( label << " the socket for " << m_desc
              << " is invalid. Should not happen... " );
        close();
        throw SocketClosedException( label );
      }
      // hmm, seems to be something serious
      throw GenericSocketException( strerror(errno) , label);
    }
    if ( nResult == 0 )
    {
      // connection lost, close connection
      close();
      // in case we already read something return what we have
      if (read!=0) break;
      DEBUG( label << ":" << m_desc << " nrecv return 0 " );
      throw SocketClosedException( label );
    }
    read+=nResult;
    p+=nResult;
    if (mode == P_READ)
      m_recBytes+=nResult;
    else
      m_sendBytes+=nResult;
  }
  DEBUG( label << ":" << m_desc << " leave read_write return:" << read );
  return (size_t) read;
}

//--------------------------------------------------------------------------

bool TcpSocket::poll( PMode mode, int timeout )
{
  struct pollfd fds;
  fds.fd = m_fd;
  if (mode == P_READ )
    fds.events = POLLIN;
  else if ( mode == P_WRITE )
    fds.events = POLLOUT;
  else if ( mode == P_READ_WRITE )
    fds.events = POLLOUT | POLLIN;
  // has socket been closed?
  if ( m_fd < 0 )
    throw SocketClosedException( "TcpSocket::poll" );
  int ret=::poll( &fds, 1, timeout);
  if ( ret > 0 )
    return true;
  if ( ret == -1 )
  {
    // this is an error
    if (errno == EINTR)
      // no fatal error
      return false;
    INFO( "Error from poll: " << strerror(errno) );
    // we return true here, a subsequent read/write will handle the error
    return true;
    // hmm, seems to be something serious
    //throw GenericSocketException( strerror(errno) , "TcpSocket::poll" );
  }
  // nothing new
  return false;
}

//--------------------------------------------------------------------------

void TcpSocket::close()
{
  if ( m_fd < 0 )
    // already closed
    return;
  if ( ::shutdown( m_fd, SHUT_RDWR ) < 0 )
  {
    // FIXME log something? already closed?
  }
#ifdef WIN32
  ::closesocket( m_fd );
#else
  ::close( m_fd );
#endif
  m_fd = -1;
  //  myTcpSocketMonitorConnections.close();
}

//---------------------------------------------------------------------------

bool TcpSocket::isOpen() const
{
  return m_fd >0;
}

//---------------------------------------------------------------------------
