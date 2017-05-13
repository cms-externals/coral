#ifndef ORACLEACCESS_OPERATIONWITHQUERY_H
#define ORACLEACCESS_OPERATIONWITHQUERY_H 1

#include <string>
#include <boost/shared_ptr.hpp>
#include "RelationalAccess/IOperationWithQuery.h"

namespace coral
{

  namespace OracleAccess
  {

    class QueryDefinition;
    class SessionProperties;

    /**
     * Class OperationWithQuery
     *
     * Simple implementation of the IOperationWithQuery interface for the OracleAccess module
     */

    class OperationWithQuery : virtual public coral::IOperationWithQuery
    {
    public:
      /// Constructor
      OperationWithQuery( const std::string& sqlPrefix,
                          boost::shared_ptr<const SessionProperties> properties,
                          const std::string& schemaName );

      /// Destructor
      virtual ~OperationWithQuery();

      /**
       * Returns a reference to the underlying query definition,
       * so that it can be filled-in by the client.
       */
      coral::IQueryDefinition& query();

      /**
       * Executes the operation and returns the number of rows affected.
       */
      long execute();

    private:
      /// The schema name for this operation
      const std::string m_schemaName;

      /// The underlying query definition
      QueryDefinition* m_queryDefinition;

      /// The sql statement that is to be executed
      std::string m_statement;
    };
  }
}

#endif
