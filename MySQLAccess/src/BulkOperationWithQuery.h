// $Id: BulkOperationWithQuery.h,v 1.3 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_BULK_OPERATION_WITH_QUERY_H
#define MYSQLACCESS_BULK_OPERATION_WITH_QUERY_H

#include <string>
#include "RelationalAccess/IBulkOperationWithQuery.h"

namespace coral
{
  namespace MySQLAccess
  {
    class BulkOperation;
    class QueryDefinition;
    class ISessionProperties;

    /**
     * Class IBulkOperationWithQuery
     * Implementation of the IBulkOperationWithQuery interface
     */
    class BulkOperationWithQuery : virtual public IBulkOperationWithQuery
    {
    public:
      /// Constructor
      BulkOperationWithQuery( const ISessionProperties& properties,
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
      /// A reference to the sessopm properties
      const ISessionProperties&   m_properties;

      /// The cache size
      int m_rowsInCache;

      /// The sql prefix
      std::string m_sqlPrefix;

      /// The query definition object
      QueryDefinition*            m_queryDefinition;

      /// The bulk operation
      BulkOperation*              m_bulkOperation;
    };
  }
}

#endif
