#ifndef CORALSERVERBASE_STATSTYPE_THREADS_H
#define CORALSERVERBASE_STATSTYPE_THREADS_H

#include "CoralMonitor/IStatsType.h"
#include "CoralMonitor/StatsStorage.h"

#include <vector>

namespace coral {

  namespace CoralSockets {

    class StatsTypeThreads : public IStatsType {
    public:
      //constructor
      StatsTypeThreads();
      //constructor
      ~StatsTypeThreads();
      //virtual function from IStatsType
      void getFiguresDesc(std::vector<std::string>&);

      void getFiguresData(std::vector<double>&);

      void add();

      void remove();

      void addidle();

      void removeidle();

    private:

      pthread_mutex_t* m_mutex;

      size_t m_amount_t;

      size_t m_max_amount;

      size_t m_total_created;

      size_t m_current_idle;

      size_t m_max_idle;

    };

  }
}

#endif
