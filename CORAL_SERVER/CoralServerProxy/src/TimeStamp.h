#ifndef CORALSERVERPROXY_TIMESTAMP_H
#define CORALSERVERPROXY_TIMESTAMP_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: TimeStamp.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class TimeStamp.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <iostream>
#include <time.h>

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
 *  Simple timestamp class.
 *
 *  @version $Id: TimeStamp.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class TimeStamp  {
public:

  // Default constructor
  TimeStamp () { m_timespec.tv_sec = 0 ; m_timespec.tv_nsec = 0 ; }
  TimeStamp ( const struct timespec& tmspec ) : m_timespec(tmspec) {}

  // Destructor
  ~TimeStamp () {}

  // make a timestamp with the current time
  static TimeStamp now() ;

  // print the timestamp according to a format given, format
  // is the same as for strftime with an addition of '%f' field
  // replaced by the number of milliseconds
  void print ( std::ostream& out, std::string fmt ) const ;
  
  // print the timestamp in JSON format
  void printJSON ( std::ostream& out ) const ;
  
protected:

private:

  // Data members
  struct timespec m_timespec ;

};

std::ostream& 
operator << ( std::ostream& out, const TimeStamp& t ) ;

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_TIMESTAMP_H
