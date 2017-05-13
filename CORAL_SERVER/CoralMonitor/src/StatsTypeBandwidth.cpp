#include "CoralMonitor/StatsTypeBandwidth.h"
#include "CoralMonitor/StatsStorage.h"

#include <sstream>
#include <iostream>

namespace coral {

  StatsTypeBandwidth::StatsTypeBandwidth(const std::string& id, const std::string& desc)
    : IStatsType( id )
    , m_mutex( new pthread_mutex_t )
    , m_desc( desc )
    , m_last_timestamp( time(0) )
    , m_last_data( 0 )
    , m_last_counter( 0 )
    , m_current_data( 0 )
    , m_current_counter( 0 )
  {
    //initialise the mutex
    pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
    *m_mutex = semaphore_mutex;
#ifdef CORALSERVERBASE_STATSSTORAGE_H
    StatsStorageInstance().add( this );
#endif
  }

  StatsTypeBandwidth::~StatsTypeBandwidth()
  {
    delete m_mutex;
  }

  void
  StatsTypeBandwidth::getFiguresDesc(std::vector<std::string>& desc)
  {
    desc.push_back(m_desc + " in amount/s");
    desc.push_back(m_desc + " in bytes/s");
  }

  void
  StatsTypeBandwidth::getFiguresData(std::vector<double>& data)
  {
    pthread_mutex_lock(m_mutex);

    //calculate the difference between last and now
    time_t current = time(0);

    size_t seconds = current - m_last_timestamp;
    if(seconds)
    {
      m_last_timestamp = current;
      //calculate the current bandwith
      m_current_data = m_last_data / seconds;
      //reset all data
      m_last_data = 0;

      m_current_counter = m_last_counter / seconds;

      m_last_counter = 0;
    }

    data.push_back(m_current_counter);
    data.push_back(m_current_data);

    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypeBandwidth::add(unsigned long long data)
  {
    pthread_mutex_lock(m_mutex);

    m_last_data = m_last_data + data;

    m_last_counter++;

    pthread_mutex_unlock(m_mutex);
  }


}
