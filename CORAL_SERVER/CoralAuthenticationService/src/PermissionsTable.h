#ifndef CORAL_PERMISSIONSTABLE_H
#define CORAL_PERMISSIONSTABLE_H

#include "CoralKernel/Service.h"
#include "RelationalAccess/ISchema.h"

namespace coral {
  class PermissionsTable
  {
  public:

    static const std::string defaultTableName()
    { return "PERMISSIONS_TBL"; }

    struct columnNames {
      static const std::string physConStr() { return "PHYS_CON_STR"; }
      static const std::string dbRole() { return "DB_ROLE"; }
      static const std::string distinguishedName() { return "DN"; }
      static const std::string vomsFQAN() { return "VOMS_FQAN"; }
    };

    struct columnTypes {
      typedef std::string physConStr;
      typedef std::string dbRole;
      typedef std::string distinguishedName;
      typedef std::string vomsFQAN;
    };

    // Standard Constructor
    PermissionsTable(ISchema& schema);

    /// Standard Destructor
    virtual ~PermissionsTable();

    /// create a database table.
    /// the table should not exist already and it is asummed that
    /// an update transaction has already been started
    static void createTable( ISchema& schema );

    /// drop the database table
    static void deleteTable( ISchema& schema );

    void addPermission(
                       const columnTypes::physConStr& physConStr,
                       const columnTypes::dbRole& dbRole,
                       const columnTypes::distinguishedName &dn,
                       const columnTypes::vomsFQAN &vomsFQAN );

    int delPermission(
                      const columnTypes::physConStr& physConStr,
                      const columnTypes::dbRole& dbRole,
                      const columnTypes::distinguishedName &dn,
                      const columnTypes::vomsFQAN &vomsFQAN );

  private:
    void generateWhere( AttributeList & whereData, std::string& whereString,
                        bool wildcards, const std::string& table,
                        const std::string& physConStr, const std::string& dbRole,
                        const std::string& dn, const std::string& vomsFQAN );

    /// the schema in which the table resides
    ISchema &m_schema;

    /// handle to the permissions table
    ITable &m_table;

  };

}

#endif
