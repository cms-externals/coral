#ifndef CORALSERVERPROXY_NETSOCKET_H
#define CORALSERVERPROXY_NETSOCKET_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: NetSocket.h,v 1.1.2.2 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class NetSocket.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <sys/types.h>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Socket class. This code is thread-safe and has to be thread-safe
 *  because other parts of the system rely on this. Socket is just a
 *  file descriptor with a bunch of associated methods.
 *
 *  @see AdditionalClass
 *
 *  @version $Id: NetSocket.h,v 1.1.2.2 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author
 */

namespace coral {
namespace CoralServerProxy {

class NetEndpointAddress ;


class NetSocket  {
public:

  // default constructor - non-functional socket
  NetSocket () ;

  // constructor from socket() arguments
  NetSocket ( int domain, int type, int protocol ) ;

  // Destructor
  ~NetSocket () ;

  // socket status, even if it returns true it does not mean
  // that socket is actually open
  bool isOpen() const { return m_fd >= 0 ; }

  // set socket option
  int setSocketOptions ( int level, int option, int value ) ;

  // bind to address
  int bind ( const NetEndpointAddress& addr ) ;

  // listen for incoming connections
  int listen ( int nmax ) ;

  // accept incoming connection
  NetSocket accept () ;

  // connect to address
  int connect ( const NetEndpointAddress& addr ) ;

  // read the data from socket, only available data, don't wait,
  // returns number of bytes read, or -1 on errors
  int recv ( void* buf, size_t len ) const ;

  // read the data from socket, try to read as much a possible,
  // returns number of bytes read, or -1 on errors
  int read ( void* buf, size_t len ) const ;

  // write the data to socket, try to write as much a possible,
  // returns number of bytes written, or -1 on errors
  int write ( const void* buf, size_t len ) const ;

  // shutdown the communications
  int shutdown() ;

  // close connection
  int close() ;

  // get the peer address
  NetEndpointAddress peer() const ;

  // print me
  void print( std::ostream& out ) const ;

  // compare two sockets, can be used as a key in std::map or set
  bool operator < ( NetSocket other ) const { return m_fd < other.m_fd ; }

  // compare two sockets
  bool operator == ( NetSocket other ) const { return m_fd == other.m_fd ; }

protected:

  // make a socket from file descriptor
  NetSocket ( int fd ) ;

private:

  // Data members
  int m_fd;

};

inline
std::ostream&
operator<< ( std::ostream& o, const NetSocket& s ) {
  s.print ( o ) ;
  return o ;
}

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_NETSOCKET_H
