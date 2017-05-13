// -*- C++ -*-
#ifndef CORAL_COMMON_SIMPLE_TIMER_H
#define CORAL_COMMON_SIMPLE_TIMER_H 1

#include "CoralBase/boost_thread_headers.h"

namespace coral
{
  /**
   * @class SimpleTimer SimpleTimer.h CoralCommon/SimpleTimer.h
   * A class defining simple timer type using microsecond precision
   * or whatever a platform precision is and is implemented
   * using the boost::posix_time::ptime class
   * This is not a high precision linear time growing timer class.
   * Heavily depends on the BOOST installation compiled-in options,
   * platform and compiler.
   * Not thread-safe, e.g. should not be used by 2 or more threads
   * in parallel.
   */
  class SimpleTimer
  {
  public:
    typedef long long int ValueType;

  public:
    /// Constructor
    SimpleTimer();
    /// Destructor
    ~SimpleTimer();
    /// Start measuring a time period taking the starting snapshot in time
    void start();
    /// Re-set any measurement done so far
    void restart();
    /// Return the time duration since the time a measurement has started
    /// The intermmediate duration is updated
    ValueType sample();
    /// Stop measuring a time, duration is saved,
    /// one has to re-start after this call
    void stop();
    /// Return the total measured time duration
    /// since the timer has been start()-ed, timer must be stop()-ed
    /// in order to get a total measured time duration
    ValueType total();

  private:
    boost::posix_time::ptime m_start;
    boost::posix_time::time_duration m_duration;
    bool m_running;
  };
}

#endif // CORAL_COMMON_SIMPLE_TIMER_H
