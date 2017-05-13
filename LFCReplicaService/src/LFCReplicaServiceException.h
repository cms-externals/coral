#ifndef LFCREPLICASERVICE_LFCREPLICASERVICEEXCEPTION_H
#define LFCREPLICASERVICE_LFCREPLICASERVICEEXCEPTION_H

#include "CoralBase/Exception.h"

namespace coral {

  namespace LFCReplicaService {

    class LFCReplicaServiceException : public coral::Exception {

    public:

      /// Constructors
      LFCReplicaServiceException(const std::string& message,
                                 const std::string& methodName,
                                 std::string moduleName = "CORAL/Services/LFCReplicaService" ) :
        Exception( message, methodName, moduleName ){}


      /// Destructor
      virtual ~LFCReplicaServiceException() throw() {}

    };
  }
}

#endif
