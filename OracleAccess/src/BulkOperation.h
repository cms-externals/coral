#ifndef ORACLEACCESS_BULKOPERATION_H
#define ORACLEACCESS_BULKOPERATION_H 1

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "RelationalAccess/IBulkOperation.h"

struct OCIStmt;

namespace coral
{

  class AttributeList;

  namespace OracleAccess
  {

    class SessionProperties;
    class StatementStatistics;
    class PolymorphicVector;

    /**
     * Class BulkOperation
     *
     * Implementation of the IBulkOperation interface
     */

    class BulkOperation : virtual public coral::IBulkOperation
    {
    public:
      /// Constructor
      BulkOperation( boost::shared_ptr<const SessionProperties> properties,
                     const std::string& schemaName,
                     const coral::AttributeList& inputBuffer,
                     int cacheSize,
                     const std::string& statement );

      /// Destructor
      virtual ~BulkOperation();

      /**
       * Processes the next iteration
       */
      void processNextIteration();

      /**
       * Flushes the data on the client side to the server.
       */
      void flush();

    private:
      /// Resets the operation and closes the statement handle
      void reset();

    private:

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The schema name for this bulk operation
      const std::string m_schemaName;

      /// A reference to the input data buffer
      const coral::AttributeList& m_inputBuffer;

      /// The cache size
      int m_rowsInCache;

      /// The iterations counter
      int m_rowsUsed;

      /// The OCI statement handle
      OCIStmt* m_ociStmtHandle;

      /// The data cache
      std::vector< PolymorphicVector* > m_dataCache;

      /// The statement statistics
      StatementStatistics* m_statementStatistics;
    };

  }

}

#endif
