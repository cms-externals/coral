#ifndef CORALMONITOR_STATSTYPE_COUNTER_H
#define CORALMONITOR_STATSTYPE_COUNTER_H

#include "CoralMonitor/IStatsType.h"

namespace coral {

  class StatsTypeCounter : public IStatsType {
  public:

    StatsTypeCounter(const std::string& id, const std::string& desc);

    virtual ~StatsTypeCounter();
    //virtual function from IStatsType
    void getFiguresDesc(std::vector<std::string>&);

    void getFiguresData(std::vector<double>&);

    void increase();

    void decrease();

  private:

    pthread_mutex_t* m_mutex;

    std::string m_desc;

    size_t m_counter;

  };

}

#endif
