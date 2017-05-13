#ifndef CORAL_CORALSTUBS_EXCEPTIONS_H
#define CORAL_CORALSTUBS_EXCEPTIONS_H

#include <stdexcept>
#include <exception>

#include "CoralBase/Exception.h"


namespace coral {

  namespace CoralStubs {

    // Base exception class for CoralStubs
    class StubsException : public std::exception
    {
    public:
      // Constructor
      StubsException( const std::string& message );
      // Destructor
      virtual ~StubsException() throw() {}
      // The error reporting method
      virtual const char* what() const throw() { return m_message.c_str(); }

    private:
      // The name of the module which threw the exception
      std::string m_message;
    };

    class DBProxyException : public std::exception
    {
    public:
      // Constructor
      DBProxyException( const std::string& message );
      // Destructor
      virtual ~DBProxyException() throw() {}
      // The error reporting method
      virtual const char* what() const throw() { return m_message.c_str(); }

    private:
      // The name of the module which threw the exception
      std::string m_message;
    };

    class InvalidMessageException : public coral::Exception
    {
    public:
      // Constructor
      InvalidMessageException( const std::string& message, const std::string& location );
      // Destructor
      virtual ~InvalidMessageException() throw() {}
    };

    class ReplyIteratorException : public coral::Exception
    {
    public:
      // Constructor
      ReplyIteratorException( const std::string& message, const std::string& location );
      // Destructor
      virtual ~ReplyIteratorException() throw() {}
    };

    class StreamBufferException : public coral::Exception
    {
    public:
      // Constructor
      StreamBufferException( const std::string& message, const std::string& location );
      // Destructor
      virtual ~StreamBufferException() throw() {}
    };

  }

}

#endif
