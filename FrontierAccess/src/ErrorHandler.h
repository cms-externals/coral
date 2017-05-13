#ifndef FRONTIER_ACCESS_FRONTIERERRORHANDLER_H
#define FRONTIER_ACCESS_FRONTIERERRORHANDLER_H

#include <string>

namespace coral {

  namespace FrontierAccess {

    /**
       @class ErrorHandler ErrorHandler.h

       Helper class to analyze an error from an error handle
    */
    class ErrorHandler
    {
    public:
      /// Constructor
      explicit ErrorHandler();

      /// Destructor
      ~ErrorHandler();

      /// handles the error
      void handleCase( int status, const std::string& context );

      /// Checks if an error has been issued
      bool isError() const;

      /// Returns the message
      const std::string message() const;

      /// Returns the last error code
      int lastErrorCode() const;

    private:
      bool m_error;
      std::string m_lastMessage;
      int m_lastErrorCode;
    };

  }
}

#endif
