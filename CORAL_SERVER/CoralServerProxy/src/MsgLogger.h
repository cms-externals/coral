#ifndef CORALSERVERPROXY_MSGLOGGER_H
#define CORALSERVERPROXY_MSGLOGGER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: MsgLogger.h,v 1.1.2.3 2010/12/20 11:27:02 avalassi Exp $
//
// Description:
//	Class MsgLogger.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include "CoralBase/boost_thread_headers.h"

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

// User-friendly macros [yuck]
#define PXY_MSG(LVL,MSG) \
    do { if ( std::ostream* str = coral::CoralServerProxy::MsgLogger::stream(coral::CoralServerProxy::MsgLogger::LVL) ) { \
      boost::mutex::scoped_lock qlock_msglogger ( coral::CoralServerProxy::MsgLogger::s_mutex ) ; \
      coral::CoralServerProxy::MsgLogger::header ( *str, coral::CoralServerProxy::MsgLogger::LVL ) ; \
      (*str) << MSG << std::endl ; \
    } } while ( false )
#define PXY_ERR(msg) PXY_MSG(Error,msg)
#define PXY_WARN(msg) PXY_MSG(Warning,msg)
#define PXY_INFO(msg) PXY_MSG(Info,msg)
#define PXY_TRACE(msg) PXY_MSG(Trace,msg)
#define PXY_DEBUG(msg) PXY_MSG(Debug,msg)

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Utility class for doing the message logging. Use one of the
 *  macroses above in the client code. Error and warning messages
 *  are redirected to std::cerr, all other messages to std::cout.
 *  Current logging level is controlled by the member function
 *  setLevel().
 *
 *  @version $Id: MsgLogger.h,v 1.1.2.3 2010/12/20 11:27:02 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class MsgLogger  {
public:

  enum Level { Error = 0, Warning, Info, Trace, Debug } ;

  // set global message level
  static void setLevel ( unsigned int lvl ) ;

  // get global message level
  static unsigned int level () ;

  // get the log stream for the given log level
  static std::ostream* stream( CoralServerProxy::MsgLogger::Level lvl ) ;

  // send the message jheader to the stream
  static void header( std::ostream& o, CoralServerProxy::MsgLogger::Level lvl ) ;

protected:

private:

  // Data members

public:

  static boost::mutex s_mutex ;
};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_MSGLOGGER_H
