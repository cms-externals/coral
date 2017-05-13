#ifndef CORALSOCKETS_GENERICSOCKETEXCEPTION_H
#define CORALSOCKETS_GENERICSOCKETEXCEPTION_H

#include "CoralBase/Exception.h"

namespace coral {

  namespace CoralSockets {

    class GenericSocketException : public Exception
    {

    public:

      /// Constructor
      GenericSocketException( const std::string& message,
                              const std::string& methodName = "" )
        : Exception( message, methodName, "coral::CoralSockets" ) {}

      /// Destructor
      virtual ~GenericSocketException() throw() {}

    };

  }

}
#endif
