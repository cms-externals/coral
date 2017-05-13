#ifndef CORAL_SQLITEACCESS_STATEMENTSTATISTICS_H
#define CORAL_SQLITEACCESS_STATEMENTSTATISTICS_H

#include <string>

namespace coral {

  namespace SQLiteAccess {

    /** @class StatementStatistics StatementStatistics.h
        A simple structure to hold some statistics variables for a statement.
    */
    class StatementStatistics {
    public:
      /// Constructor
      StatementStatistics() : sqlStatement(""),idleTime( 0 ){}
      /// Reset
      void reset(){sqlStatement=""; idleTime=0; }
      /// The actual SQL statement
      std::string sqlStatement;

      /// The total idle time
      double idleTime;
    };

  }

}

#endif
