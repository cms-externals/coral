#ifndef TESTCORE_TESTENV_EXCEPTIONS_H
#define TESTCORE_TESTENV_EXCEPTIONS_H

#include "CoralBase/Exception.h"

namespace coral {

  class TestEnvException : public coral::Exception
  {
  public:
    TestEnvException(const std::string& message, const std::string& methodName)
      : Exception( message, methodName, "TestEnv" )
    {
    }

  };

}

#endif
