#include "CoralCommon/Utilities.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void
coral::sleepSeconds( unsigned int secondsToSleep )
{
#ifdef WIN32
  ::Sleep( 1000 * secondsToSleep );
#else
  ::sleep( secondsToSleep );
#endif
}
