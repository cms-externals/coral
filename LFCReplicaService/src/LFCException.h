#ifndef LFCREPLICASERVICE_LFCEXCEPTION_H
#define LFCREPLICASERVICE_LFCEXCEPTION_H

#include "CoralBase/Exception.h"

namespace coral {

  namespace LFCReplicaService {

    class LFCException : public coral::Exception {

    public:

      explicit LFCException(const std::string& contextMessage);

      /// Constructors
      LFCException(const std::string& contextMessage,
                   const std::string& lfcFunctionName);

      LFCException(const std::string& contextMessage,
                   const std::string& lfcFunctionName,
                   int errorCode);

      /// Destructor
      virtual ~LFCException() throw() {}

      const char* what() const throw() { return m_message.c_str(); }

      static std::string resolveCode(int code);

    private:

      std::string m_message;
    };
  }
}

#endif
