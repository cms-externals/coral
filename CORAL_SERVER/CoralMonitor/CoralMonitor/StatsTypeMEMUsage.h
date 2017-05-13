#ifndef CORALMONITOR_STATSTYPE_MEMUSAGE_H
#define CORALMONITOR_STATSTYPE_MEMUSAGE_H

#include "CoralMonitor/IStatsType.h"

namespace coral {

  class StatsTypeMEMUsage : public IStatsType {
  public:
    //constructor
    StatsTypeMEMUsage();

    virtual ~StatsTypeMEMUsage();

    void getFiguresDesc(std::vector<std::string>&);

    void getFiguresData(std::vector<double>&);

  private:

    pthread_mutex_t* m_mutex;

    //would be better to have a static member
    //cannot include CPUusageData.h in public header
    unsigned long long m_total;

    unsigned long long m_resident;

    unsigned long long m_share;

    unsigned long long m_text;

    unsigned long long m_lib;

    unsigned long long m_data;

  };

}

#endif
