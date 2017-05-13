#ifndef CORALMONITOR_STATSTYPE_TIMER_H
#define CORALMONITOR_STATSTYPE_TIMER_H

#include "CoralMonitor/IStatsType.h"

namespace coral {

  class StatsTypeTimer : public IStatsType {
  public:
    //constructor
    StatsTypeTimer(const std::string& id, const std::string& desc);
    //constructor
    virtual ~StatsTypeTimer();
    //virtual function from IStatsType
    void getFiguresDesc(std::vector<std::string>&);

    void getFiguresData(std::vector<double>&);

    void add(double usert, double syst, double realt);

  private:

    pthread_mutex_t* m_mutex;

    std::string m_desc;

    size_t m_counter;

    size_t m_counter_lastt;

    size_t m_current_counter;

    double m_total_usertime;

    double m_total_systemtime;

    double m_total_realtime;

    double m_last_usertime;

    double m_last_systemtime;

    double m_last_realtime;

    double m_current_usertime;

    double m_current_systemtime;

    double m_current_realtime;

    time_t m_lastfetch;

  };

}

#endif
