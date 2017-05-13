//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: MsgLogger.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class MsgLogger...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "MsgLogger.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "TimeStamp.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  unsigned int msgLevel = coral::CoralServerProxy::MsgLogger::Info ;

}

namespace coral {
namespace CoralServerProxy {

boost::mutex MsgLogger::s_mutex ;


//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

// set global message level
void
MsgLogger::setLevel ( unsigned int lvl )
{
  ::msgLevel = lvl ;
}

// get global message level
unsigned int
MsgLogger::level ()
{
  return ::msgLevel ;
}

// get the log stream for the given log level
std::ostream*
MsgLogger::stream( Level lvl )
{
  std::ostream* stream = 0 ;
  if ( (unsigned int)lvl <= ::msgLevel ) {
    switch ( lvl ) {
    case Error:
    case Warning:
      stream = &std::cerr ;
      break ;
    default :
      stream = &std::cout ;
      break ;
    }
  }
  return stream ;

}

// send the message jheader to the stream
void
MsgLogger::header( std::ostream& o, Level lvl )
{
  // print current time
  o << TimeStamp::now() ;

  // print message log level
  const char* l = " [???] " ;
  switch ( lvl ) {
  case Error:
    l = " [ERR] " ;
    break ;
  case Warning:
    l = " [WRN] " ;
    break ;
  case Info:
    l = " [INF] " ;
    break ;
  case Trace:
    l = " [TRC] " ;
    break ;
  case Debug:
    l = " [DBG] " ;
    break ;
  }
  o.write ( l, 7 ) ;
}


} // namespace CoralServerProxy
} // namespace coral
