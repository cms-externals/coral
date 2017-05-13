#include "ReplicaStatus.h"

namespace coral { namespace LFCReplicaService {

  std::string
  statusCode( ReplicaStatus status )
  {
    return status == On ? ON : OFF;
  }

} }
