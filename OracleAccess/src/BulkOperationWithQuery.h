#ifndef ORACLEACCESS_BULKOPERATIONWITHQUERY_H
#define ORACLEACCESS_BULKOPERATIONWITHQUERY_H 1

#include <string>
#include <boost/shared_ptr.hpp>
#include "RelationalAccess/IBulkOperationWithQuery.h"

namespace coral
{

  namespace OracleAccess
  {

    class BulkOperation;
    class QueryDefinition;
    class SessionProperties;

    /**
     * Class IBulkOperationWithQuery
     * Implementation of the IBulkOperationWithQuery interface
     */
    class BulkOperationWithQuery : virtual public IBulkOperationWithQuery
    {
    public:
      /// Constructor
      BulkOperationWithQuery( boost::shared_ptr<const SessionProperties> properties,
                              const std::string& schemaName,
                              int cacheSize,
                              const std::string& statement );

      /// Destructor
      virtual ~BulkOperationWithQuery();

      /**
       * Returns a reference to the underlying query definition,
       * so that it can be filled-in by the client.
       */
      coral::IQueryDefinition& query();

      /**
       * Processes the next iteration
       */
      void processNextIteration();

      /**
       * Flushes the data on the client side to the server.
       */
      void flush();

    private:
      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The schema name for this bulk operation
      const std::string m_schemaName;

      /// The cache size
      int m_rowsInCache;

      /// The sql prefix
      std::string m_sqlPrefix;

      /// The query definition object
      QueryDefinition* m_queryDefinition;

      /// The bulk operation
      BulkOperation* m_bulkOperation;
    };
  }
}

#endif
