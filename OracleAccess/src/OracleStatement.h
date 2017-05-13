#ifndef ORACLEACCESS_ORACLESTATEMENT_H
#define ORACLEACCESS_ORACLESTATEMENT_H 1

#include "oratypes.h"
#include <string>
#include <typeinfo>
#include <vector>
#include <boost/shared_ptr.hpp>

struct OCIDate;
struct OCIDateTime;
struct OCILobLocator;
struct OCINumber;
struct OCIStmt;

namespace coral
{

  class AttributeList;
  class Date;
  class TimeStamp;

  namespace OracleAccess
  {

    class IOutputVariableHolder;
    class SessionProperties;
    class StatementStatistics;

    /**
     * @class OracleStatement OracleStatement.h
     *
     * Class responsible for the execution of SQL statements
     * and for the retrieval of its results.
     *
     */

    class OracleStatement
    {
    public:
      /**
       * Constructor
       *
       */
      OracleStatement( boost::shared_ptr<const SessionProperties> properties,
                       const std::string& schemaName,
                       const std::string& sqlStatement  );

      /// Destructor
      ~OracleStatement();

      /**
       * Prepares and executes the statement.
       * @param inputData The input bind data.
       * Returns false in case of any error, true otherwise.
       * If an errorCodePtr is provided as input, the pointed *errorCodePtr
       * will contain the Oracle error code if an error occurs.
       *
       */
      bool execute( const coral::AttributeList& inputData,
                    sb4* errorCodePtr = 0 );

      /**
       * Returns the C++ type for an output column [0,..]
       *
       * @param columnId The column identifier
       *
       * @param typeConverter The SQL to C++ type converter
       *
       */
      const std::type_info* typeForOutputColumn( int columnId ) const;

      /**
       * Defines the output. It assumes that the privided output AttributeList
       * matches exactly the actual output from the statement.
       * Throws in case of any error (fix bug #80099).
       *
       * @param outputData The buffer where the output data must be delivered.
       *
       */
      void defineOutput( coral::AttributeList& outputData );

      /**
       * Returns the number of rows processed.
       *
       */
      long numberOfRowsProcessed() const;

      /**
       * Sets the number of prefetched rows
       *
       * @param numberOfRows The number of rows to be prefetched.
       *
       */
      bool setNumberOfPrefetchedRows( unsigned int numberOfRows );

      /**
       * Sets the size of the cache for prefetched rows
       *
       * @param sizeInMB The size of the cache in MBytes.
       *
       */
      bool setCacheSize( unsigned int sizeInMB );

      /**
       * Fetches the next row. Returns false in case no more rows are there.
       *
       */
      bool fetchNext();

      /**
       * Retrieves the current row number
       *
       */
      unsigned int currentRowNumber() const;

    private:

      /// Resets the internal structures.
      void reset();

      /// Copies the data from the buffers to the defined output
      bool copyOutput();

    private:

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The schema name for this statement
      const std::string m_schemaName;

      /// The OCI statement handle
      OCIStmt* m_ociStmtHandle;

      /// Indicator array
      std::vector< sb2 > m_indicators;

      /// The output data
      coral::AttributeList* m_outputData;

      /// The buffer holding the output variables
      std::vector< IOutputVariableHolder* > m_outputVariables;

      // The statistics for the statement
      StatementStatistics* m_statementStatistics;

      /// The SQL statement text (for debugging purposes only)
      std::string m_sqlStatement;

    };

  }
}
#endif
