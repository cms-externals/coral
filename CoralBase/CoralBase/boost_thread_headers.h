// $Id: boost_thread_headers.h,v 1.3 2010/12/20 13:32:02 avalassi Exp $
#ifndef CORALBASE_BOOSTTHREADHEADERS_H 
#define CORALBASE_BOOSTTHREADHEADERS_H 1

// Disable warnings triggered by the Boost 1.42.0 headers
// See http://wiki.services.openoffice.org/wiki/Writing_warning-free_code
// See also http://www.artima.com/cppsource/codestandards.html
// See also http://gcc.gnu.org/onlinedocs/gcc-4.1.1/cpp/System-Headers.html
// See also http://gcc.gnu.org/ml/gcc-help/2007-01/msg00172.html
#if defined __GNUC__
#pragma GCC system_header
#endif

// Include files
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#endif // CORALBASE_BOOSTTHREADHEADERS_H 
