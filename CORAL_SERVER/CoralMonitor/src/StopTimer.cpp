#include <cmath>
#include <cstdio> // For sprintf on gcc46 (bug #89762)
#include <map>
#include <iostream>
#include <sys/syscall.h> // For SYS_gettid
#include <string.h>
#include <memory.h>

#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/Exception.h"
#include "CoralMonitor/StopTimer.h"

using namespace coral;

#ifdef __linux

#include <fcntl.h>
#include <sys/types.h>
#include <linux/unistd.h>

#undef CLK_TCK
#define CLK_TCK 100

#endif

//-----------------------------------------------------------------------------

class calcMean {
public:
  /// constructor
  calcMean() : m_no(0), m_SumSq(0), m_Sum(0), m_Min(0), m_Max(0)
  {};

  virtual ~calcMean() {};

  /// add a number to the dataset
  void newValue( double number )
  {
    if ( m_no == 0 ) {
      m_Min=number;
      m_Max=number;
    }
    m_no++;
    m_Sum+=number;
    m_SumSq+=(number*number);
    if ( m_Min > number )
      m_Min = number;
    if ( m_Max < number )
      m_Max = number;
  };

  /// returns the number of data points
  int getNoOfPoints() const
  {
    return m_no;
  };

  /// returns the mean value
  double getMean() const
  {
    if ( m_no == 0 )
      return nan("");

    return m_Sum/(double) m_no;
  };

  /// returns the standard deviation
  double getDeviation() const
  {
    if ( m_no == 0)
      return nan("");

    double mean=getMean();
    return sqrt(m_SumSq/(double)m_no - mean*mean);
  };

  /// returns the sum of all values
  double getTotal() const
  {
    return m_Sum;
  };

  /// returns the minimal value
  double getMin() const
  {
    return m_Min;
  };

  /// returns the maximum value
  double getMax() const
  {
    return m_Max;
  };

  void toOutputStream( std::ostream& os ) const
  {
    os.setf(std::ostream::fixed, std::ostream::floatfield );
    os.precision(4);
    os << m_Min << " < " << getMean() << "+-" << getDeviation()
       << " < " << m_Max ;
  };


private:
  /// no of data points
  int m_no;

  /// sum of squares
  double m_SumSq;

  /// sum
  double m_Sum;

  /// min
  double m_Min;

  /// max
  double m_Max;
};

//-----------------------------------------------------------------------------

class TimerStat {
public:
  TimerStat() {};

  void addValues( const StopTimer& timer )
  {
    m_user.newValue( timer.getUserTime() );
    m_system.newValue( timer.getSystemTime() );
    m_real.newValue( timer.getRealTime() );
  };

  int getNoOfPoints() const
  {
    return m_user.getNoOfPoints();
  }

  void toOutputStream( std::ostream& os ) const
  {
    os << "user time   : ";
    m_user.toOutputStream( os );
    os << std::endl;
    os << "system time : ";
    m_system.toOutputStream( os );
    os << std::endl;
    os << "real time   : ";
    m_real.toOutputStream( os );
    os << std::endl;
  };

private:
  calcMean m_user;
  calcMean m_system;
  calcMean m_real;
};

//-----------------------------------------------------------------------------

std::ostream& operator << (std::ostream& os, const TimerStat& x)
{
  x.toOutputStream(os);
  return os;
}

//-----------------------------------------------------------------------------

class TimerManager {
public:
  TimerManager();

  virtual ~TimerManager() {};

  /// adds the value of the timer to the statistics with name
  void addTimer( const StopTimer& timer, const std::string& name );

  /// print results of all timers
  void printAll( std::ostream & os);

  /// returns true if the collecting of timining information is enabled
  inline bool isTimingEnabled();

  /// enable/disable timings
  void enableTimings( bool enable );

  /// deletes all collected statistic
  void deleteTimings();

private:
  typedef std::map< std::string, TimerStat > TimerMap;

  boost::mutex m_mutex;

  TimerMap m_timers;

  bool m_isTimingEnabled;

};

//-----------------------------------------------------------------------------

TimerManager::TimerManager()
  : m_timers(), m_isTimingEnabled(0)
{
  if ( getenv( "CORALSERVER_TIMING" ) != 0 ) {
    m_isTimingEnabled=true;
    std::cout << "CORAL Server timings enabled" << std::endl;
  };
}

//-----------------------------------------------------------------------------

bool TimerManager::isTimingEnabled()
{
  return m_isTimingEnabled;
}

//-----------------------------------------------------------------------------

void TimerManager::enableTimings( bool enable )
{
  m_isTimingEnabled = enable;
}

//-----------------------------------------------------------------------------

void TimerManager::deleteTimings()
{
  boost::mutex::scoped_lock lock(m_mutex);
  m_timers.clear();
}

//-----------------------------------------------------------------------------

void TimerManager::addTimer( const StopTimer& timer, const std::string& name )
{
  boost::mutex::scoped_lock lock(m_mutex);
  m_timers[name].addValues( timer );
}

//-----------------------------------------------------------------------------

void TimerManager::printAll( std::ostream& os)
{
  if ( !m_isTimingEnabled)
    return;

  os << "Printing timers:" << std::endl;
  boost::mutex::scoped_lock lock(m_mutex);
  for ( TimerMap::iterator it = m_timers.begin();
        it != m_timers.end(); ++it )
  {
    os << "----- " << it->first << " (n=" << it->second.getNoOfPoints()
       << ") in seconds" << std::endl;
    it->second.toOutputStream( os );
  }
}

//-----------------------------------------------------------------------------

TimerManager& TimerMgr()
{
  static TimerManager* timerMgr = new TimerManager();
  return *timerMgr;
}

//-----------------------------------------------------------------------------

/*
void coral::printTimers()
{
  TimerMgr().printAll( std::cout );
}
*/

 //-----------------------------------------------------------------------------

void coral::enableTimings( bool enable )
{
  TimerMgr().enableTimings( enable );
}

//-----------------------------------------------------------------------------

void coral::deleteTimings()
{
  TimerMgr().deleteTimings();
}

//-----------------------------------------------------------------------------
#if 0
class StopTimer {
public:
  /// default constructor
  StopTimer();

  virtual ~StopTimer() {};

  /// start timer (doesn't reset the times)
  void start();

  /// stop timer
  void stop();

  /// get user time in seconds
  double getUserTime();

  /// get system time in seconds
  double getSystemTime();

  /// get the real time in seconds
  double getRealTime();

private:
  /// is the watch running?
  bool m_isRunning;

  /// how much time elapsed
  struct tms m_elapsedTime;

  /// starting time of the watch
  struct tms m_startTime;

  /// elapsed real time
  clock_t m_startReal;

  /// starting real time
  clock_t m_startReal;
}
#endif

/*
    taken from
    http://www.linuxforums.org/forum/linux-programming-scripting/101371-pthreads-thread-time-vs-process-time.html
*/

#ifdef __linux
  void getThreadTimes( unsigned long& jiffies_user, unsigned long& jiffies_system)
{
  char procFilename[256];
  char buffer[1024];

  pid_t pid = ::getpid();
  //pid_t tid = ::gettid();
  pid_t tid = ::syscall( SYS_gettid );

  sprintf(procFilename, "/proc/%d/task/%d/stat",pid,tid) ;
  int fd, num_read;
  fd = open(procFilename, O_RDONLY, 0);
  num_read = read(fd, buffer, 1023);
  close(fd);
  buffer[num_read] = '\0';

  char* ptrUsr = strrchr(buffer, ')') + 1;
  for(int i = 3 ; i != 14 ; ++i) ptrUsr = strchr(ptrUsr+1, ' ');

  ptrUsr++;
  jiffies_user = atol(ptrUsr);
  jiffies_system = atol(strchr(ptrUsr,' ') + 1);

}
#endif

//-----------------------------------------------------------------------------

StopTimer::StopTimer()
  : m_isRunning( false )
{
  memset( &m_elapsedTime, 0, sizeof( m_elapsedTime ) );
  memset( &m_startTime, 0, sizeof( m_startTime ) );
}

//-----------------------------------------------------------------------------

void StopTimer::start()
{
  if ( !TimerMgr().isTimingEnabled() )
    return;

  if (m_isRunning)
    throw coral::Exception("Timer is already running!",
                           "StopTimer::start",
                           "coral::CoralServerBase");

  m_startReal = times( &m_startTime );
#ifdef __linux
  getThreadTimes( m_startUserLinux, m_startSystemLinux);
#endif
  m_isRunning=true;
}

//-----------------------------------------------------------------------------

void StopTimer::stop()
{
  if ( !TimerMgr().isTimingEnabled() )
    return;

  if (!m_isRunning)
    throw coral::Exception("Timer is not running!",
                           "StopTimer::stop",
                           "coral::CoralServerBase");

  m_elapsedReal = times( &m_elapsedTime );

  m_elapsedReal -=m_startReal;
  m_elapsedTime.tms_utime -= m_startTime.tms_utime;
  m_elapsedTime.tms_stime -= m_startTime.tms_stime;
  m_isRunning=false;
#ifdef __linux
  getThreadTimes( m_elapsedUserLinux, m_elapsedSystemLinux);
  m_elapsedUserLinux-=m_startUserLinux;
  m_elapsedSystemLinux-=m_startSystemLinux;
#endif

}

//-----------------------------------------------------------------------------

double StopTimer::getUserTime() const
{
  if (m_isRunning)
    throw coral::Exception("Timer is still running",
                           "StopTimer::start",
                           "coral::CoralServerBase");
#ifdef __linux
  return (double)m_elapsedUserLinux / (double) ( CLK_TCK );
#else
  return (double)m_elapsedTime.tms_utime / (double) ( CLK_TCK );
#endif
}

//-----------------------------------------------------------------------------

double StopTimer::getSystemTime() const
{
  if (m_isRunning)
    throw coral::Exception("Timer is still running",
                           "StopTimer::start",
                           "coral::CoralServerBase");

  return (double)m_elapsedTime.tms_stime / (double) ( CLK_TCK );
}

//-----------------------------------------------------------------------------

double StopTimer::getRealTime() const
{
  if (m_isRunning)
    throw coral::Exception("Timer is still running",
                           "StopTimer::start",
                           "coral::CoralServerBase");

  return (double)m_elapsedReal / (double) ( CLK_TCK );
}
