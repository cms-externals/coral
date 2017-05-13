#ifndef CORALMONITOR_CSVPLOTTER_H
#define CORALMONITOR_CSVPLOTTER_H

#include <string>

namespace coral
{

  class StatsCSVPlotter {
  public:

    // Constructor
    // rotate_rate < 0 : disabled
    // rotate_rate = 0 : rename once
    // rotate_rate > 0 : rename in seconds
    StatsCSVPlotter(size_t refresh_rate, const std::string& filename, int rotate_rate = -1 );

    ~StatsCSVPlotter();

  private:

    pthread_t m_tid;

  };

}

#endif
