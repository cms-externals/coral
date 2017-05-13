#ifndef CORALSOCKETS_NONFATALSOCKETEXCEPTION_H
#define CORALSOCKETS_NONFATALSOCKETEXCEPTION_H

#include "CoralBase/Exception.h"

namespace coral {

  namespace CoralSockets {

    class NonFatalSocketException : public Exception
    {

    public:

      /// Constructor
      NonFatalSocketException( const std::string& message,
                               const std::string& methodName = "" )
        : Exception( message, methodName, "coral::CoralSockets" ) {}

      /// Destructor
      virtual ~NonFatalSocketException() throw() {}

    };

  }

}
#endif
