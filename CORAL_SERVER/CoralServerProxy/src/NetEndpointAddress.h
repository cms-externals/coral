#ifndef CORALSERVERPROXY_NETENDPOINTADDRESS_H
#define CORALSERVERPROXY_NETENDPOINTADDRESS_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: NetEndpointAddress.h,v 1.1.2.2 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class NetEndpointAddress.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <iostream>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "NetAddress.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Address/port pair as a single object.
 *
 *  @see AdditionalClass
 *
 *  @version $Id: NetEndpointAddress.h,v 1.1.2.2 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class NetEndpointAddress  {
public:

  // Default constructor
  NetEndpointAddress () : m_address(), m_port(-1) {}

  // makes endpoint with address INADDR_ANY and a given port number
  explicit NetEndpointAddress ( int port ) : m_address(), m_port(port) {}

  // makes endpoint with a given address and a given port number
  NetEndpointAddress ( const std::string& host, int port )
    : m_address(host), m_port(port) {}

  // makes endpoint with a given address and a given port number
  NetEndpointAddress ( NetAddress host, int port )
    : m_address(host), m_port(port) {}

  // Destructor
  ~NetEndpointAddress () {}

  // get host address
  unsigned int address() const { return m_address.address() ; }

  // get port number
  int port() const { return m_port ; }

  // printing op
  void print ( std::ostream& out ) const { out << m_address << ':' << m_port ; }

protected:

private:

  // Data members
  NetAddress m_address ;
  int m_port;

};

// printing of the address in a nice format
inline
std::ostream&
operator<< ( std::ostream& out, const NetEndpointAddress& a )
{
  a.print ( out ) ;
  return out ;
}

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_NETENDPOINTADDRESS_H
