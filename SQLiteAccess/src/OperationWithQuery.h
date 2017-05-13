#ifndef SQLITE_OPERATIONWITHQUERY_H
#define SQLITE_OPERATIONWITHQUERY_H

#include "RelationalAccess/IOperationWithQuery.h"
#include <string>
#include <boost/shared_ptr.hpp>

namespace coral {

  namespace SQLiteAccess {

    class QueryDefinition;
    class SessionProperties;

    /**
     * Class OperationWithQuery
     *
     * Simple implementation of the IOperationWithQuery interface for the SQLiteAccess module
     */

    class OperationWithQuery : virtual public coral::IOperationWithQuery
    {
    public:
      /// Constructor
      OperationWithQuery( const std::string& sqlPrefix,
                          boost::shared_ptr<const SessionProperties> properties );

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
      /// The sql statement that is to be executed
      std::string m_statement;
      /// The underlying query definition
      QueryDefinition*  m_queryDefinition;
    };
  }
}

#endif
