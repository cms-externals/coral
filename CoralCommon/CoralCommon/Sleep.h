// -*-C++-*-

#ifndef CORAL_COMMON_SYSTEM_H
#define CORAL_COMMON_SYSTEM_H 1

namespace coral
{
  namespace sys
  {
    typedef long long int NanoSecs;

    void nanosleep (NanoSecs nanosecs);

    /** Take a high-precision nap, ignoring interruptions.  Like
        #nanosleep() except time is in microseconds.  */
    void usleep (NanoSecs usecs);

    /** Take a high-precision nap, ignoring interruptions.  Like
        #nanosleep() except time is in milliseconds.  */
    void msleep (NanoSecs msecs);

    /** Take a high-precision nap, ignoring interruptions.  Like
        #nanosleep() except time is in seconds.  */
    void sleep (NanoSecs secs);
  }
}

#endif // CORAL_COMMON_SYSTEM_H
