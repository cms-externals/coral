// $Id: StopTimer.h,v 1.2.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALSERVERBASE_STOPTIMER_H
#define CORALSERVERBASE_STOPTIMER_H 1

// Include files
#include <string>
#include <sys/times.h>

#include "StatsTypeTimer.h"

namespace coral {
  /**
   * Usage:
   *
   * StopTimer myTimer;
   * myTimer.start();
   * // performance critical part
   * myTimer.stop("package::class::performancePart1")
   *
   * The times will be automatically added to the report
   *
   * Alternatively you can use the scoped timer, which is very nice for
   * functions:
   *
   * void class::doSomething()
   * {
   *   ScopedTimer timer("package::class::doSomething");
   *   // do something
   * }
   *
   *
   */

  /// globaly enable/disable timings
  /// by default the timings are disabled, and can be enabled
  /// by setting the environment variable CORALSERVER_TIMING
  void enableTimings( bool enabled=true );

  /// prints times of all registered timers
  //void printTimers();

  /// deletes all so far collected statistics
  void deleteTimings();

  class StopTimer {
  public:
    /// default constructor
    StopTimer();

    virtual ~StopTimer() {};

    /// start timer
    virtual void start();

    /// stop timer and add to report
    virtual void stop();

    /// get user time in seconds
    virtual double getUserTime() const;

    /// get system time in seconds
    virtual double getSystemTime() const;

    /// get the real time in seconds
    virtual double getRealTime() const;

  private:
    /// is the watch running?
    bool m_isRunning;

    /// how much time elapsed
    struct tms m_elapsedTime;

    /// starting time of the watch
    struct tms m_startTime;

    /// elapsed real time
    clock_t m_elapsedReal;

    /// starting real time
    clock_t m_startReal;

#ifdef __linux
    unsigned long m_startUserLinux;
    unsigned long m_startSystemLinux;
    unsigned long m_elapsedUserLinux;
    unsigned long m_elapsedSystemLinux;
#endif
  };

  class ScopedTimer {
  public:

    ScopedTimer()
      : m_stopTimer()
    {
      m_stopTimer.start();
    };

    ~ScopedTimer()
    {
      m_stopTimer.stop();
    };

  private:

    StopTimer m_stopTimer;
  };

  class ScopedTimerStats {
  public:

    ScopedTimerStats(StatsTypeTimer& statst)
      : m_stat( statst )
    {
      m_stopTimer.start();
    };

    ~ScopedTimerStats()
    {
      m_stopTimer.stop();
      m_stat.add(m_stopTimer.getUserTime(), m_stopTimer.getSystemTime(), m_stopTimer.getRealTime());
    };

  private:

    StatsTypeTimer& m_stat;

    StopTimer m_stopTimer;

  };

}

#endif
