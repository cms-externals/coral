#ifndef ORACLEACCESS_ORACLEERRORHANDLER_H
#define ORACLEACCESS_ORACLEERRORHANDLER_H

#include <string>
#include "oratypes.h"

struct OCIError;

namespace coral {

  namespace OracleAccess {

    /**
       @class OracleErrorHandler OracleErrorHandler.h

       Helper class to analyze an error from an error handle
    */
    class OracleErrorHandler
    {
    public:
      /// Constructor
      explicit OracleErrorHandler( OCIError* ociErrorHandle );

      /// Destructor
      ~OracleErrorHandler();


#ifdef CORAL240CN
      /// Handles the error (and prints error msg to log if printToLog==true)
      void handleCase( sword status,
                       const std::string& context,
                       bool printToLog = false );
#else
      /// Handles the error
      void handleCase( sword status,
                       const std::string& context );
#endif

      /// Checks if an error has been issued
      bool isError() const;

      /// Returns the message
      const std::string& message() const;

      /// Returns the last error code
      sb4 lastErrorCode() const;

    private:

      OCIError*   m_ociErrorHandle;
      bool m_error;
      std::string m_lastMessage;
      sb4 m_lastErrorCode;

    };

  }
}
#endif
