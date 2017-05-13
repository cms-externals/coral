#ifndef LFCREPLICASERVICE_REPLICAPERMISSION_H
#define LFCREPLICASERVICE_REPLICAPERMISSION_H

namespace coral {

  namespace LFCReplicaService {

    typedef enum { NoPerm=0, RPerm=1, WPerm=2, RWPerm=3 } ReplicaPermission;

    int permissionMode(ReplicaPermission permission);

  }

}

#endif
