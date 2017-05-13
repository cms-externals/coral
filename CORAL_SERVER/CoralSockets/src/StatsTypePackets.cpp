#include "StatsTypePackets.h"

#include <sstream>
#include <iostream>

namespace coral { namespace CoralSockets {

  StatsTypePackets::StatsTypePackets(const std::string& id, const std::string& desc)
    : IStatsType(id)
    , m_mutex( new pthread_mutex_t )
    , m_desc( desc )
    , m_last_timestamp( time(0) )
    , m_last_data_dh( 0 )
    , m_last_data_dp( 0 )
    , m_last_counter( 0 )
    , m_current_data_dh( 0 )
    , m_current_data_dp( 0 )
    , m_current_counter( 0 )
    , m_amount_writep(0)
    , m_amount_write_dh(0)
    , m_amount_write_dp(0)
  {
    //initialise the mutex
    pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
    *m_mutex = semaphore_mutex;
#ifdef CORALSERVERBASE_STATSSTORAGE_H
    coral::StatsStorageInstance().add( this );
#endif
  }

  StatsTypePackets::~StatsTypePackets()
  {
    delete m_mutex;
  }

  void
  StatsTypePackets::getFiguresDesc(std::vector<std::string>& desc)
  {
    desc.push_back(m_desc + " in amount/s");
    desc.push_back(m_desc + " header data in bytes/s");
    desc.push_back(m_desc + " payload data in bytes/s");
    desc.push_back(m_desc + " total amount");
    desc.push_back(m_desc + " total header data");
    desc.push_back(m_desc + " total payload data");
  }

  void
  StatsTypePackets::getFiguresData(std::vector<double>& data)
  {
    pthread_mutex_lock(m_mutex);

    //calculate the difference between last and now
    time_t current = time(0);

    size_t seconds = current - m_last_timestamp;
    if(seconds)
    {
      m_last_timestamp = current;
      //calculate the current bandwith
      m_current_data_dh = m_last_data_dh / seconds;
      m_current_data_dp = m_last_data_dp / seconds;
      //reset all data
      m_last_data_dh = 0;
      m_last_data_dp = 0;

      m_current_counter = m_last_counter / seconds;

      m_last_counter = 0;
    }

    data.push_back(m_current_counter);
    data.push_back(m_current_data_dh);
    data.push_back(m_current_data_dp);

    data.push_back(m_amount_writep);
    data.push_back(m_amount_write_dh);
    data.push_back(m_amount_write_dp);

    pthread_mutex_unlock(m_mutex);
  }

  void
  StatsTypePackets::add(size_t header, size_t payload)
  {
    pthread_mutex_lock(m_mutex);

    m_last_data_dh = m_last_data_dh + header;
    m_last_data_dp = m_last_data_dp + payload;

    m_last_counter++;

    m_amount_writep++;
    m_amount_write_dh = m_amount_write_dh + header;
    m_amount_write_dp = m_amount_write_dp + payload;

    pthread_mutex_unlock(m_mutex);
  }

} }
