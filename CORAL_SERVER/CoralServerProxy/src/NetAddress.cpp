//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: NetAddress.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class NetAddress...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "NetAddress.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <exception>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  class NetAddressException : public std::exception {
  public:
    NetAddressException ( const std::string& msg ) : _msg(msg) {
      _msg += ": " ;
      _msg += hstrerror( h_errno );
    }
    virtual ~NetAddressException() throw() {}
    virtual const char* what() const throw() { return _msg.c_str() ; }
  private:
    std::string _msg ;
  };

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace coral {
namespace CoralServerProxy {

//----------------
// Constructors --
//----------------
NetAddress::NetAddress ()
  : m_addr( INADDR_ANY )
{
}

NetAddress::NetAddress ( const std::string& name )
  : m_addr( INADDR_ANY )
{
  hostent* h = gethostbyname( name.c_str() ) ;
  if ( h && h->h_addr_list[0] ) {
    m_addr = ntohl(*reinterpret_cast<unsigned*>(h->h_addr_list[0]));
  } else {
    throw NetAddressException ( "gethostbyname() failed for name " + name );
  }
}

// printing of the address in a nice format
std::ostream&
operator<< ( std::ostream&  out, const NetAddress& addr )
{
  unsigned a = addr.address() ;
  return out << ( ( a >> 24 ) & 0xFF ) << '.'
             << ( ( a >> 16 ) & 0xFF ) << '.'
             << ( ( a >> 8 ) & 0xFF ) << '.'
             << ( a & 0xFF ) ;
}

} // namespace CoralServerProxy
} // namespace coral
