#ifndef CORALMONITOR_STATS_SIMPLE_BUFFER_H
#define CORALMONITOR_STATS_SIMPLE_BUFFER_H

#include <string>
#include <list>
#include <vector>
#include <map>

namespace coral
{

  struct StatsSimpleBufferPlot {

    std::string name;

    std::vector<std::string> stats;

    std::vector< std::pair<std::string, size_t> > gnuplot;

  };

  class StatsSimpleBuffer {
  public:

    StatsSimpleBuffer(size_t refresh_rate /* in seconds */, size_t maxsize);

    ~StatsSimpleBuffer();
    //fills up the buffer with new data
    void refresh();

    const std::list<std::string>& data() { return m_databuffer; };

    const std::map<std::string, StatsSimpleBufferPlot*>& plots() { return m_plots; };

    void startDatPlotter(const std::string& filename, const std::string& plotdir, size_t refresh_rate);

    void stopDatPlotter();

    StatsSimpleBufferPlot& addPlot(const std::string& name);

  private:
    //buffer for data
    std::list<std::string> m_databuffer;

    std::map<std::string, StatsSimpleBufferPlot*> m_plots;

    size_t m_refresh_rate;

    size_t m_maxsize;

    time_t m_last_refresh;

    pthread_t m_dat_tid;

    pthread_attr_t m_dat_attr;


  };


}

#endif
