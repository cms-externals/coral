#ifndef CORALSTUBS_STATSTYPE_PAYLOAD_H
#define CORALSTUBS_STATSTYPE_PAYLOAD_H

#include "CoralMonitor/StatsTypeTimer.h"

namespace coral {

  namespace CoralStubs {

    class StatsTypePayload : public StatsTypeTimer {
    public:
      //constructor
      StatsTypePayload(std::string opcode, std::string desc);
      //constructor
      virtual ~StatsTypePayload();

    };

  }
}

#endif
