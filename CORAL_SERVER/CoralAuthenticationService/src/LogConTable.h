#ifndef CORAL_LOGCONTABLE_H
#define CORAL_LOGCONTABLE_H

#include "RelationalAccess/ISchema.h"

namespace coral {
  class LogConTable
  {
  public:

    static const std::string defaultTableName()
    { return "LOG_CON_TBL"; }

    struct columnNames {
      static const std::string logConStr() { return "LOG_CON_STR"; }
    };

    struct columnTypes {
      typedef std::string logConStr;
    };

    // Standard Constructor
    LogConTable(ISchema& schema);

    /// Standard Destructor
    virtual ~LogConTable();

    /// create a connections table.
    static void createTable( ISchema& schema );

    /// drop the connections table
    static void deleteTable( ISchema& schema );

    /// add credentials to the table
    void addLogCon( const std::string& logConStr );

    /// returns true if the logical connection is in the table
    bool logConExists( const std::string& logConStr );

    /// delete connection(s) from the table
    /// returns the no of deleted connections
    long delLogCon( const std::string& logConStr );

  private:

    void generateWhere(
                       AttributeList & whereData, std::string& whereString,
                       const std::string& table,
                       const std::string& logConStr );

    /// the schema in which the table resides
    ISchema &m_schema;

    /// handle to the database table
    ITable &m_table;

  };

}

#endif
