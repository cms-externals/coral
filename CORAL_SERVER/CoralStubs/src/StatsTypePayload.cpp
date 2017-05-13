#include "StatsTypePayload.h"

#include <sstream>
#include <iostream>
#include <limits.h>

namespace coral { namespace CoralStubs {

  StatsTypePayload::StatsTypePayload(std::string opcode, std::string desc)
    : StatsTypeTimer("CStubs_Request_" + opcode, desc)
  {
  }

  StatsTypePayload::~StatsTypePayload()
  {
  }


} }
