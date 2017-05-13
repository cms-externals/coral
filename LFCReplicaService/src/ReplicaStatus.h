#ifndef LFCREPLICASERVICE_REPLICASTATUS_H
#define LFCREPLICASERVICE_REPLICASTATUS_H

#include <string>

namespace coral {

  namespace LFCReplicaService {

    typedef enum { Off, On } ReplicaStatus;

    static const std::string ON("1");
    static const std::string OFF("0");

    std::string statusCode( ReplicaStatus status );

  }

}

#endif
