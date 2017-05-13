#ifndef CORALSERVERBASE_STATSTYPE_THREADS_H
#define CORALSERVERBASE_STATSTYPE_THREADS_H

#include "CoralMonitor/IStatsType.h"
#include "CoralMonitor/StatsStorage.h"

#include <vector>

namespace coral {

  namespace CoralSockets {

    class StatsTypePackets : public IStatsType {
    public:
      //constructor
      StatsTypePackets(const std::string& id, const std::string& desc);
      //constructor
      ~StatsTypePackets();
      //virtual function from IStatsType
      void getFiguresDesc(std::vector<std::string>&);

      void getFiguresData(std::vector<double>&);

      void add(size_t header, size_t payload);

    private:

      pthread_mutex_t* m_mutex;

      std::string m_desc;

      time_t m_last_timestamp;

      unsigned long long m_last_data_dh;

      unsigned long long m_last_data_dp;

      unsigned long long m_last_counter;

      double m_current_data_dh;

      double m_current_data_dp;

      double m_current_counter;

      unsigned long long m_amount_writep;

      unsigned long long m_amount_write_dh;

      unsigned long long m_amount_write_dp;

    };

  }
}

#endif
