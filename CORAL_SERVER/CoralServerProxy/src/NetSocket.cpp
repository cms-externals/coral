//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: NetSocket.cpp,v 1.3.2.2 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class NetSocket...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "NetSocket.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "NetEndpointAddress.h"

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
NetSocket::NetSocket ()
  : m_fd ( -1 )
{
}

// make a socket from file descriptor
NetSocket::NetSocket ( int fd )
  : m_fd ( fd )
{
}

// constructor from socket() arguments
NetSocket::NetSocket ( int domain, int type, int protocol )
  : m_fd ( ::socket( domain, type, protocol ) )
{
}

//--------------
// Destructor --
//--------------
NetSocket::~NetSocket ()
{
}

// set socket option
int
NetSocket::setSocketOptions ( int level, int option, int value )
{
  return ::setsockopt ( m_fd, level, option, &value, sizeof(value) ) ;
}

// bind to address
int
NetSocket::bind ( const NetEndpointAddress& addr )
{
  // build socket address
  sockaddr_in saddr ;
  bzero ( &saddr, sizeof saddr );
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl( addr.address() ) ;
  saddr.sin_port = htons( addr.port() ) ;

  // bind the socket
  return ::bind ( m_fd, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr) ) ;
}

// listen for incoming connections
int
NetSocket::listen ( int nmax )
{
  return ::listen ( m_fd, nmax ) ;
}

// accept incoming connection
NetSocket
NetSocket::accept ()
{
  sockaddr_in cliaddr ;
  bzero ( &cliaddr, sizeof cliaddr );
  socklen_t len = sizeof(sockaddr_in) ;
  return NetSocket ( ::accept( m_fd, reinterpret_cast<sockaddr*>(&cliaddr), &len ) ) ;
}

// connect to address
int
NetSocket::connect ( const NetEndpointAddress& addr )
{
  // build socket address
  sockaddr_in saddr ;
  bzero ( &saddr, sizeof saddr );
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl( addr.address() ) ;
  saddr.sin_port = htons( addr.port() ) ;

  return ::connect ( m_fd, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr) ) ;
}

int
NetSocket::shutdown ()
{
  int r = ::shutdown ( m_fd, SHUT_RDWR ) ;
  return r ;
}

int
NetSocket::close ()
{
  int r = ::close ( m_fd ) ;
  m_fd = -1 ;
  return r ;
}

// read the data from socket, only available data, don't wait,
// returns number of bytes read, or -1 on errors
int
NetSocket::recv ( void* buf, size_t len ) const
{
  do {
    ssize_t s = ::recv( m_fd, buf, len, 0 ) ;
    if ( s < 0 and errno == EINTR ) {
      // if signal-terminated then try again
      continue ;
    } else {
      return s ;
    }
  } while ( true ) ;
}

// read the data from socket, try to read as much a possible,
// returns number of bytes read, or -1 on errors
int
NetSocket::read ( void* buf, size_t len ) const
{
  size_t size_read = 0 ;  // number of bytes read already
  while ( size_read < len ) {

    void* p = (char*)buf + size_read ;
    size_t size_to_read = len - size_read ;

    errno = 0 ;
    ssize_t s = ::recv( m_fd, p, size_to_read, MSG_WAITALL ) ;
    if ( s < 0 ) {
      // error
      if ( errno == EINTR ) {
        // if signal-terminated then try again
        continue ;
      } else {
        break ;
      }
    } else if ( s == 0 ) {
      // closed connection
      break ;
    } else {
      size_read += s ;
    }

  }

  return size_read ;
}

// write the data to socket, try to write as much a possible,
// returns number of bytes written, or -1 on errors
int
NetSocket::write ( const void* buf, size_t len ) const
{
  size_t size_sent = 0 ;  // number of bytes sent already
  while ( size_sent < len ) {

    void* p = (char*)buf + size_sent ;
    size_t size_to_send = len - size_sent ;

    errno = 0 ;
#ifdef __APPLE__
    // http://bugs.freepascal.org/view.php?id=9401,
    // http://www.mail-archive.com/fpc-devel@lists.freepascal.org/msg12176.html
    // http://lists.apple.com/archives/macnetworkprog/2002/Dec/msg00091.html
    ssize_t s = ::send( m_fd, p, size_to_send, MSG_DONTROUTE | SO_NOSIGPIPE ) ;
#else
    ssize_t s = ::send( m_fd, p, size_to_send, MSG_DONTROUTE | MSG_NOSIGNAL ) ;
#endif
    if ( s < 0 ) {
      // error
      if ( errno == EINTR ) {
        // if signal-terminated then try again
        continue ;
      } else {
        break ;
      }
    } else if ( s == 0 ) {
      // closed connection
      break ;
    } else {
      size_sent += s ;
    }

  }

  return size_sent ;
}

// get the peer address
NetEndpointAddress
NetSocket::peer() const
{
  sockaddr_in saddr ;
  bzero ( &saddr, sizeof saddr );
  socklen_t addr_len = sizeof(saddr) ;
  int rc = getpeername ( m_fd, reinterpret_cast<sockaddr*>(&saddr), &addr_len ) ;
  if ( rc != 0 ) {
    return NetEndpointAddress() ;
  }

  NetAddress na ( (unsigned int)ntohl(saddr.sin_addr.s_addr) ) ;
  return NetEndpointAddress ( na, ntohs(saddr.sin_port) ) ;
}

// print me
void
NetSocket::print( std::ostream& out ) const
{
  out << "<socket:" ;
  if ( m_fd >= 0 ) {
    out << m_fd ;
  } else {
    out << "closed" ;
  }
  out << ">" ;
}

} // namespace CoralServerProxy
} // namespace coral
