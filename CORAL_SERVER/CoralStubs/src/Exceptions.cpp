#include "Exceptions.h"

namespace coral { namespace CoralStubs {

  StubsException::StubsException( const std::string& message )
    : m_message( message )
  {
  }

  DBProxyException::DBProxyException( const std::string& message )
    : m_message( "Exception from the DBProxy ( " + message + " )" )
  {
  }

  InvalidMessageException::InvalidMessageException( const std::string& message, const std::string& location )
    : Exception( message , location , "CoralStubs" )
  {
  }

  ReplyIteratorException::ReplyIteratorException( const std::string& message, const std::string& location )
    : Exception( message , location , "CoralStubs" )
  {
  }

  StreamBufferException::StreamBufferException( const std::string& message, const std::string& location )
    : Exception( message , location , "CoralStubs" )
  {
  }

} }
