#ifndef CORALMONITOR_CPUSAGEDATA_H
#define CORALMONITOR_CPUSAGEDATA_H

#include <string>

namespace coral {

  struct CPUsageContainer {

    double system;

    double user;

    double nice;

  };

  class CPUsageData {
  public:

    CPUsageData();

    ~CPUsageData();

    void getUsage(CPUsageContainer&);

  private:

    void getDataLinux();

    size_t _user;

    size_t _system;

    size_t _nice;

    size_t _idle;

    size_t _last_user;

    size_t _last_system;

    size_t _last_nice;

    size_t _last_idle;

  };

}

#endif
