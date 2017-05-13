#include "CoralCommon/Sleep.h"

# ifdef _WIN32
#  include <windows.h>
# else
#  include <unistd.h>
#  include <time.h>
#  include <sys/time.h> /* Hope for the best the U**x dialect has this with sys/select.h included TO BE FIXED */
# endif

#include <cerrno>

namespace coral
{
  namespace sys
  {
    void nanosleep (NanoSecs nanosecs)
    {
#     ifdef _WIN32
      ::SleepEx (DWORD (nanosecs / 1e6), false);
#     elif defined( __USE_POSIX199309 ) // Should be defined on a Linux system if _GNU_SOURCE is in use
      struct timespec duration;
      struct timespec rem;
      duration.tv_sec = int (nanosecs / 1e9);
      duration.tv_nsec = int (nanosecs - duration.tv_sec * 1e9);
      while (::nanosleep (&duration, &rem) == -1 && errno == EINTR)
        duration = rem;
#     else
      // NB: Not all selects report how much time is left if the call is
      // interrupted.  So ignore errors and hope for the best...
      struct timeval tv;
      tv.tv_sec = int (nanosecs / 1e9);
      tv.tv_usec = int (nanosecs - tv.tv_sec * 1e9) / 1000;
      ::select (0, 0, 0, 0, &tv);
#     endif
    }

    void usleep (NanoSecs usecs)
    { nanosleep (usecs * 1000); }

    void msleep (NanoSecs msecs)
    { nanosleep (msecs * 1000000); }

    void sleep (NanoSecs secs)
    { nanosleep (secs * 1000000000); }
  }
}
