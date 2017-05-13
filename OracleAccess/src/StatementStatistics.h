#ifndef CORAL_ORACLEACCESS_STATEMENTSTATISTICS_H
#define CORAL_ORACLEACCESS_STATEMENTSTATISTICS_H

#include <string>

namespace coral {

  namespace OracleAccess {

    /** @class StatementStatistics StatementStatistics.h
        A simple structure to hold some statistics variables for a statement.
    */
    class StatementStatistics {
    public:
      /// Constructor
      explicit StatementStatistics( const std::string& _sqlStatement ) :
        sqlStatement( _sqlStatement ), idleTime( 0 )
      {}

      /// The actual SQL statement
      std::string sqlStatement;

      /// The total idle time
      double idleTime;
    };

  }

}

#endif
