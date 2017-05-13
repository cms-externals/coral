#include "CoralMonitor/StatsTypeCounter.h"
#include "CoralMonitor/StatsStorage.h"

#include <sstream>
#include <iostream>

namespace coral {

  StatsTypeCounter::StatsTypeCounter(const std::string& id, const std::string& desc)
    : IStatsType( id )
    , m_mutex( new pthread_mutex_t )
    , m_desc( desc )
    , m_counter(0)
  {
    //initialise the mutex
    pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
    *m_mutex = semaphore_mutex;
#ifdef CORALSERVERBASE_STATSSTORAGE_H
    StatsStorageInstance().add( this );
#endif
  }

  StatsTypeCounter::~StatsTypeCounter()
  {
    delete m_mutex;
  }

  void
  StatsTypeCounter::getFiguresDesc(std::vector<std::string>& desc)
  {
    desc.push_back(m_desc + " counter");
  }

  void
  StatsTypeCounter::getFiguresData(std::vector<double>& data)
  {
    pthread_mutex_lock(m_mutex);

    data.push_back(m_counter);

    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypeCounter::increase()
  {
    pthread_mutex_lock(m_mutex);

    m_counter++;

    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypeCounter::decrease()
  {
    pthread_mutex_lock(m_mutex);

    if(m_counter > 0)
      m_counter--;
    //else print some error

    pthread_mutex_unlock(m_mutex);
  }


}
