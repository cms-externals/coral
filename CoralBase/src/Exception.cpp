#include "CoralBase/Exception.h"


coral::Exception::Exception( const std::string& message,
                             const std::string& methodName,
                             const std::string& moduleName ) :
  m_message( message + " ( CORAL : \"" + methodName + "\" from \"" + moduleName + "\" )" )
{
}

void coral::Exception::setMessage(const std::string& message)
{
  m_message = message;
}
