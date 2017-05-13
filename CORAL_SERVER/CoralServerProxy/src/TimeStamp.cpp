//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: TimeStamp.cpp,v 1.4.2.2 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class TimeStamp...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "TimeStamp.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <iomanip>
#include <cstring> // For strlen
#include <cstdio> // For snprintf
#ifdef __APPLE__
#include <sys/time.h>
#include <time.h>
#endif

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------


namespace coral {
namespace CoralServerProxy {

// make a timestamp with the current time
TimeStamp
TimeStamp::now()
{
  // get seconds/nanoseconds
  struct timespec ts;
#ifndef __APPLE__
  clock_gettime( CLOCK_REALTIME, &ts );
#else
  // http://www.wanderinghill.com/wp/?p=29
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ts.tv_sec = tv.tv_sec;
  ts.tv_nsec = tv.tv_usec*1000;
#endif
  return TimeStamp ( ts ) ;
}

void
TimeStamp::print ( std::ostream& out, std::string fmt ) const
{
  // convert to break-down time
  struct tm tms ;
  localtime_r( &m_timespec.tv_sec, &tms );

  // replace %f in the format string with miliseconds
  std::string::size_type n = fmt.find("%f") ;
  if ( n != std::string::npos ) {
    char subs[4] ;
    snprintf ( subs, 4, "%03d", int(m_timespec.tv_nsec/1000000) ) ;
    while ( n != std::string::npos ) {
      fmt.replace ( n, 2, subs ) ;
      n = fmt.find("%f") ;
    }
  }

  char buf[128] ;
  strftime(buf, 128, fmt.c_str(), &tms );
  out.write ( buf, strlen(buf) ) ;

}

// print the timestamp in JSON format
void
TimeStamp::printJSON ( std::ostream& out ) const
{
  out << m_timespec.tv_sec << "."
      << std::setfill('0') << std::setw(6)
      << m_timespec.tv_nsec/1000
      << std::setfill(' ') ;
}

std::ostream&
operator << ( std::ostream& out, const TimeStamp& t )
{
  t.print ( out, "%Y-%m-%d %H:%M:%S.%f" ) ;
  return out ;
}

} // namespace CoralServerProxy
} // namespace coral
