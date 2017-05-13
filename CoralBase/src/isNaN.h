// $Id: isNaN.h,v 1.2 2011/03/22 11:31:07 avalassi Exp $
#ifndef CORALBASE_ISNAN_H
#define CORALBASE_ISNAN_H 1

// Include files
#include <cmath>
#ifdef _WIN32
#include <float.h>
#elif defined(__ICC)
#include <mathimf.h>
#endif

namespace coral
{

  //--------------------------------------------------------------------------

  inline bool isNaN( float value )
  {
#ifdef _WIN32
    return ( _isnan( value ) != 0 );
#elif defined(__ICC)
    return isnan( value );
#else
    return std::isnan( value );
#endif
  }

  //--------------------------------------------------------------------------

  inline bool isNaN( double value )
  {
#ifdef _WIN32
    return ( _isnan( value ) != 0 );
#elif defined(__ICC)
    return isnan( value );
#else
    return std::isnan( value );
#endif
  }

  //--------------------------------------------------------------------------

}
#endif // CORALBASE_ISNAN_H
