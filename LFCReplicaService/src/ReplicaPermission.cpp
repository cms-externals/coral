#include "ReplicaPermission.h"

namespace coral { namespace LFCReplicaService {

  int
  permissionMode(ReplicaPermission permission)
  {
    int mode = 0;
    switch (permission) {
    case NoPerm:
      break;
    case RPerm:
      mode = 4;
      break;
    case WPerm:
      mode = 2;
      break;
    case RWPerm:
      mode = 6;
      break;
    };
    return mode;
  }

} }
