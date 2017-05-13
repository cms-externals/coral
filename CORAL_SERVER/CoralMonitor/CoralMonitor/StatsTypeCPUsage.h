#ifndef CORALMONITOR_STATSTYPE_CPUSAGE_H
#define CORALMONITOR_STATSTYPE_CPUSAGE_H

#include "CoralMonitor/IStatsType.h"

namespace coral {

  class CPUsageData;

  class StatsTypeCPUsage : public IStatsType {
  public:
    //constructor
    StatsTypeCPUsage();

    virtual ~StatsTypeCPUsage();

    void getFiguresDesc(std::vector<std::string>&);

    void getFiguresData(std::vector<double>&);

  private:

    pthread_mutex_t* m_mutex;

    //would be better to have a static member
    //cannot include CPUusageData.h in public header
    CPUsageData* m_data;

  };

}

#endif
