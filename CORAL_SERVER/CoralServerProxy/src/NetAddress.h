#ifndef CORALSERVERPROXY_NETADDRESS_H
#define CORALSERVERPROXY_NETADDRESS_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: NetAddress.h,v 1.1.2.2 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class NetAddress.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <iosfwd>

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
 *  Representation of network node address. Does conversion
 *  from host name into IP address in constructor.
 *
 *  @see AdditionalClass
 *
 *  @version $Id: NetAddress.h,v 1.1.2.2 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class NetAddress  {
public:

  // Default constructor makes INADDR_ANY address
  NetAddress () ;

  // Make and address from the host name string,
  // throws std::sxception if can't find the host name.
  explicit NetAddress ( const std::string& name ) ;

  // Make and address from the IPv4 address
  explicit NetAddress ( unsigned int addr ) : m_addr(addr) {}

  // Destructor
  ~NetAddress () {}

  // get the address
  unsigned int address() const { return m_addr ; }

protected:

private:

  // Data members

  unsigned int m_addr;  // private members start with m_

};

// printing of the address in a nice format
std::ostream&
operator<< ( std::ostream&  out, const NetAddress& a ) ;

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_NETADDRESS_H
