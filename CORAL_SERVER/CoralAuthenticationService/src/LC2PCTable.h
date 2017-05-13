#ifndef CORAL_LC2PCTABLE_H
#define CORAL_LC2PCTABLE_H

#include "CoralKernel/Service.h"
#include "CoralCommon/DatabaseServiceSet.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/AccessMode.h"

#include "boost/shared_array.hpp"

namespace coral {
  class LC2PCTable
  {
  public:

    static const std::string defaultTableName()
    { return "LC2PC_TBL"; }

    struct columnNames {
      static const std::string logConStr() { return "LOG_CON_STR"; }
      static const std::string physConStr() { return "PHYS_CON_STR"; }
      static const std::string order() { return "ORDER"; }
      static const std::string accessMode() { return "ACCESS_MODE"; }
      static const std::string authenticationMechanism() { return "AUTH_MECH"; }
    };

    struct columnTypes {
      typedef std::string logConStr;
      typedef std::string physConStr;
      typedef unsigned int order;
      typedef std::string accessMode;
      typedef std::string authenticationMechanism;
    };

    // Standard Constructor
    LC2PCTable(ISchema& schema);

    /// Standard Destructor
    virtual ~LC2PCTable();

    /// create a connections table.
    static void createTable( ISchema& schema );

    /// drop the connections table
    static void deleteTable( ISchema& schema );

    const std::string& AccessMode2Str(AccessMode mode) const;
    AccessMode Str2AccessMode( const std::string& mode ) const;

    /// add locigal to physical connection mapping to the table
    void addLC2PC( const std::string& logConStr,
                   const std::string& physConStr,
                   AccessMode accessmode,
                   const std::string& authenticationMechanism );

    /// returns true if the mapping exists
    /// if wildcards is true, empty parameters will match everything
    bool LC2PCExists( bool wildcards,
                      const std::string& logConStr="",
                      const std::string& physConStr="",
                      const std::string& accessmode="",
                      const std::string& authenticationMechanism="" );

    bool LC2PCExists( bool wildcards,
                      const std::string& logConStr="",
                      const std::string& physConStr="",
                      AccessMode accessmode=coral::Update,
                      const std::string& authenticationMechanism="" )
    {
      return LC2PCExists( wildcards, logConStr, physConStr, AccessMode2Str( accessmode),
                          authenticationMechanism );
    };

    /// delete connection(s) from the table
    /// returns the no of deleted connections
    long delLC2PC( const std::string& logConStr="",
                   const std::string& physConStr="",
                   const std::string& accessmode="",
                   const std::string& authenticationMechanism="" );

    /// returns DatabaseServiceSet with all physical connections matching the serach parameters
    DatabaseServiceSet* lookup( const std::string& logicalName,
                                AccessMode accessMode, const std::string& authenticationMechanism );

  private:
    void generateWhere(
                       AttributeList& whereData, std::string& whereString,
                       bool wildcards,
                       const std::string& table,
                       const std::string& logConStr,
                       const std::string& physConStr,
                       const std::string& accessmode,
                       const std::string& authenticationMechanism );

    /// returns the next available order number
    unsigned int getNextOrderNo( const std::string& logConStr);

    /// the schema in which the table resides
    ISchema &m_schema;

    /// handle to the database table
    ITable &m_table;

  };

}

#endif
