#include "StatsTypeThreads.h"

#include <sstream>
#include <iostream>

namespace coral { namespace CoralSockets {

  StatsTypeThreads::StatsTypeThreads()
    : IStatsType("CSocket_Threads")
    , m_mutex( new pthread_mutex_t )
    , m_amount_t(0)
    , m_max_amount(0)
    , m_total_created(0)
    , m_current_idle(0)
    , m_max_idle(0)
  {
    //initialise the mutex
    pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
    *m_mutex = semaphore_mutex;
#ifdef CORALSERVERBASE_STATSSTORAGE_H
    StatsStorageInstance().add( this );
#endif
  }

  StatsTypeThreads::~StatsTypeThreads()
  {
    delete m_mutex;
  }

  void
  StatsTypeThreads::getFiguresDesc(std::vector<std::string>& desc)
  {
    desc.push_back("current threads active");
    desc.push_back("max of concurent threads");
    desc.push_back("total threads created");
    desc.push_back("current idle threads");
    desc.push_back("max idle threads");
  }

  void
  StatsTypeThreads::getFiguresData(std::vector<double>& data)
  {
    data.push_back(m_amount_t);
    data.push_back(m_max_amount);
    data.push_back(m_total_created);
    data.push_back(m_current_idle);
    data.push_back(m_max_idle);
  }

  void
  StatsTypeThreads::add()
  {
    pthread_mutex_lock(m_mutex);

    m_amount_t++;

    if(m_amount_t > m_max_amount)
      m_max_amount = m_amount_t;

    m_total_created++;

    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypeThreads::remove()
  {
    pthread_mutex_lock(m_mutex);

    m_amount_t--;
    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypeThreads::addidle()
  {
    pthread_mutex_lock(m_mutex);
    m_current_idle++;

    if(m_current_idle > m_max_idle)
      m_max_idle = m_current_idle;

    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypeThreads::removeidle()
  {
    pthread_mutex_lock(m_mutex);
    m_current_idle--;
    pthread_mutex_unlock(m_mutex);
  }



} }
