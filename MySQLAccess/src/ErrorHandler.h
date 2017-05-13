#ifndef MYSQLACCESS_ERRORHANDLER_H
#define MYSQLACCESS_ERRORHANDLER_H 1

#include "MySQL_headers.h"

#include <string>

namespace coral
{
  namespace MySQLAccess
  {

    /**
       @class MySQLAccess::ErrorHandler ErrorHandler.h

       Helper class to analyze an error from an error handle
    */
    class ErrorHandler
    {
    public:
      /// Constructor
      ErrorHandler();

      /// Destructor
      virtual ~ErrorHandler();

      /// handles the error
      void handleCase( MYSQL* connection, const std::string& context );
      void handleCase( MYSQL* connection, const std::string& context, const char*, unsigned int );

      /// Checks if an error has been issued
      bool isError() const;

      /// Returns the message
      std::string message() const;

      /// Returns the last error code
      unsigned int lastErrorCode() const;

#if (  MYSQL_VERSION_ID > 40100 )
      void handleCase( MYSQL_STMT* stmt, const std::string& context );
      void handleCase( MYSQL_STMT* stmt, const std::string& context, const char*, unsigned int );
#endif

      /// Checks if an statement error has been issued
      bool isStmtError() const;

      /// Returns the statement error message
      std::string stmtMessage() const;

      /// Returns the last statement  error code
      unsigned int stmtLastErrorCode() const;

    private:
      bool m_error;
      std::string m_lastMessage;
      unsigned int m_lastErrorCode;
      bool m_stmtError;
      std::string m_stmtLastMessage;
      unsigned int m_stmtLastErrorCode;
    };
  }
}

// Define macro to include file and line information
#define MYSQL_HANDLE_ERR(handler,connection,context) (handler.handleCase((connection),(context),__FILE__,__LINE__))

#if (  MYSQL_VERSION_ID > 40100 )
#define MYSQL_HANDLE_STMT_ERR(handler,stmt,context) (handler.handleCase((stmt),(context),__FILE__,__LINE__))
#endif

#endif // MYSQLACCESS_ERRORHANDLER_H
