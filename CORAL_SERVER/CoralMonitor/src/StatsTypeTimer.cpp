#include "CoralMonitor/StatsTypeTimer.h"
#include "CoralMonitor/StatsStorage.h"

#include <sstream>
#include <iostream>
#include <limits.h>

namespace coral {

  StatsTypeTimer::StatsTypeTimer(const std::string& id, const std::string& desc)
    : IStatsType( id )
    , m_mutex( new pthread_mutex_t )
    , m_desc( desc )
    , m_counter(0)
    , m_counter_lastt(0)
    , m_current_counter()
    , m_total_usertime(0)
    , m_total_systemtime(0)
    , m_total_realtime(0)
    , m_last_usertime(0)
    , m_last_systemtime(0)
    , m_last_realtime(0)
    , m_current_usertime(0)
    , m_current_systemtime(0)
    , m_current_realtime(0)
    , m_lastfetch( time(0) )
  {
    //initialise the mutex
    pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
    *m_mutex = semaphore_mutex;
#ifdef CORALSERVERBASE_STATSSTORAGE_H
    StatsStorageInstance().add( this );
#endif
  }

  StatsTypeTimer::~StatsTypeTimer()
  {
    delete m_mutex;
  }

  void
  StatsTypeTimer::getFiguresDesc(std::vector<std::string>& desc)
  {
    desc.push_back(m_desc + " total counter");
    desc.push_back(m_desc + " total user time");
    desc.push_back(m_desc + " total system time");
    desc.push_back(m_desc + " total real time");
    desc.push_back(m_desc + " current counter");
    desc.push_back(m_desc + " current user time");
    desc.push_back(m_desc + " current system time");
    desc.push_back(m_desc + " current real time");
  }

  void
  StatsTypeTimer::getFiguresData(std::vector<double>& data)
  {
    pthread_mutex_lock(m_mutex);

    data.push_back(m_counter);
    data.push_back(m_total_usertime);
    data.push_back(m_total_systemtime);
    data.push_back(m_total_realtime);
    //get the current time
    time_t current = time(0);
    //compare the last fetch-time with now
    if(m_lastfetch < current)
    {
      //set the fetch time to now
      m_lastfetch = current;
      //calculate the difference between last and now
      m_current_counter = m_counter_lastt;
      m_counter_lastt = 0;

      m_current_usertime = m_last_usertime;
      m_last_usertime = 0;

      m_current_systemtime = m_last_systemtime;
      m_last_systemtime = 0;

      m_current_realtime = m_last_realtime;
      m_last_realtime = 0;
    }

    data.push_back(m_current_counter);
    data.push_back(m_current_usertime);
    data.push_back(m_current_systemtime);
    data.push_back(m_current_realtime);

    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypeTimer::add(double usert, double syst, double realt)
  {
    pthread_mutex_lock(m_mutex);

    m_counter++;
    m_counter_lastt++;

    m_total_usertime = m_total_usertime + usert;
    m_total_systemtime = m_total_systemtime + syst;
    m_total_realtime = m_total_realtime + realt;

    m_last_usertime = m_last_usertime + usert;
    m_last_systemtime = m_last_systemtime + syst;
    m_last_realtime = m_last_realtime + realt;

    pthread_mutex_unlock(m_mutex);
  }

}
