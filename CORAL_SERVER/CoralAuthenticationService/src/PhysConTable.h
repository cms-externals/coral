#ifndef CORAL_PHYSCONTABLE_H
#define CORAL_PHYSCONTABLE_H

#include "RelationalAccess/ISchema.h"

namespace coral {
  class PhysConTable
  {
  public:

    static const std::string defaultTableName()
    { return "PHYS_CON_TBL"; }

    struct columnNames {
      static const std::string physConStr() { return "PHYS_CON_STR"; }
    };

    struct columnTypes {
      typedef std::string physConStr;
    };

    // Standard Constructor
    PhysConTable(ISchema& schema);

    /// Standard Destructor
    virtual ~PhysConTable();

    /// create a connections table.
    static void createTable( ISchema& schema );

    /// drop the connections table
    static void deleteTable( ISchema& schema );

    /// add credentials to the table
    void addPhysCon( const std::string& physConStr );

    /// returns returns true if the connection exists in the table
    bool physConExists( const std::string& physConStr );

    /// delete connection(s) from the table
    /// returns the no of deleted connections
    long delPhysCon( const std::string& physConStr );

  private:

    void generateWhere( AttributeList & whereData, std::string& whereString,
                        const std::string& table,
                        const std::string& physConStr );

    /// the schema in which the table resides
    ISchema &m_schema;

    /// handle to the database table
    ITable &m_table;

  };

}

#endif
