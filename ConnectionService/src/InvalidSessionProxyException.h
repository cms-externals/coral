#ifndef CONNECTIONSERVICE_INVALIDSESSIONPROXY_EXCEPTION_H
#define CONNECTIONSERVICE_INVALIDSESSIONPROXY_EXCEPTION_H 1

#include "RelationalAccess/ConnectionServiceException.h"

namespace coral {

  namespace ConnectionService {

    /// Exception thrown when no replica is available for a logical connection string
    class InvalidSessionProxyException : public coral::ConnectionServiceException {
    public:
      /// Constructor
      InvalidSessionProxyException( const std::string& connectionString,
                                    std::string methodName ) :
        coral::ConnectionServiceException( "Session Proxy for connection "+ connectionString +
                                           "\" is invalid, because the underlying service has been deleted.",
                                           methodName, "CORAL/Services/ConnectionService" )
      {}

      /// Destructor
      virtual ~InvalidSessionProxyException() throw() {}
    };

  }

}

#endif
