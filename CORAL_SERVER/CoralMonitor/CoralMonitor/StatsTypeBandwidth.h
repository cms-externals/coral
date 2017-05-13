#ifndef CORALMONITOR_STATSTYPE_BANDWIDTH_H
#define CORALMONITOR_STATSTYPE_BANDWIDTH_H

#include "CoralMonitor/IStatsType.h"

namespace coral {

  class StatsTypeBandwidth : public IStatsType {
  public:

    StatsTypeBandwidth(const std::string& id, const std::string& desc);

    virtual ~StatsTypeBandwidth();
    //virtual function from IStatsType
    void getFiguresDesc(std::vector<std::string>&);

    void getFiguresData(std::vector<double>&);

    void add(unsigned long long data);

  private:

    pthread_mutex_t* m_mutex;

    std::string m_desc;

    time_t m_last_timestamp;

    unsigned long long m_last_data;

    unsigned long long m_last_counter;

    double m_current_data;

    double m_current_counter;

  };

}

#endif
