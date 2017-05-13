// $Id: boost_tuple_headers.h,v 1.2 2011/03/22 11:22:33 avalassi Exp $
#ifndef CORALCOMMON_BOOSTTUPLEHEADERS_H
#define CORALCOMMON_BOOSTTUPLEHEADERS_H 1

// Disable warnings triggered by the Boost 1.42.0 headers
// See http://wiki.services.openoffice.org/wiki/Writing_warning-free_code
// See also http://www.artima.com/cppsource/codestandards.html
// See also http://gcc.gnu.org/onlinedocs/gcc-4.1.1/cpp/System-Headers.html
// See also http://gcc.gnu.org/ml/gcc-help/2007-01/msg00172.html
#if defined __GNUC__
#pragma GCC system_header
#endif

// Include files
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#endif // CORALCOMMON_BOOSTTUPLEHEADERS_H
