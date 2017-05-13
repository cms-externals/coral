#ifndef FRONTIER_OPERATION_WITH_QUERY_H
#define FRONTIER_OPERATION_WITH_QUERY_H

#include "RelationalAccess/IOperationWithQuery.h"
#include <string>

namespace coral {

  namespace FrontierAccess {

    class QueryDefinition;
    class ISessionProperties;

    /**
     * Class OperationWithQuery
     *
     * Simple implementation of the IOperationWithQuery interface for the FrontierAccess module
     */

    class OperationWithQuery : virtual public coral::IOperationWithQuery
    {
    public:
      /// Constructor
      OperationWithQuery( const std::string& sqlPrefix,
                          const ISessionProperties& properties );

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
      /// The underlying query definition
      QueryDefinition*  m_queryDefinition;

      /// The sql statement that is to be executed
      std::string m_statement;
    };
  }
}

#endif
