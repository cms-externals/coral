// $Id: Table.h,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_TABLE_H
#define CORAL_MYSQLACCESS_TABLE_H 1

#include <string>
#include "RelationalAccess/ITable.h"

namespace coral
{
  namespace MySQLAccess
  {
    class ISessionProperties;
    class ISchemaProperties;
    class SchemaProperties;
    class DataEditor;
    class PrivilegeManager;
    class TableSchemaEditor;

    /**
     * Class Table
     * Provides means of accessing and manipulating the data and the description of a relational table.
     */
    class Table : virtual public coral::ITable
    {
    public:
      /**
       *
       */
      Table( const std::string& tableName, const ISessionProperties& sessionProperties, ISchemaProperties& schemaProperties );

      virtual ~Table();

      /**
       * Returns the description of the table.
       */
      virtual const ITableDescription& description() const;

      /**
       * Returns a reference to the schema editor for the table.
       */
      virtual ITableSchemaEditor& schemaEditor();

      /**
       * Returns a reference to the ITableDataEditor object  for the table.
       */
      virtual ITableDataEditor& dataEditor();

      /**
       * Returns a reference to the privilege manager of the table.
       */
      virtual ITablePrivilegeManager& privilegeManager();

      /**
       * Returns a new query object for performing a query involving this table only.
       */
      virtual IQuery* newQuery() const;

    private:
      /// Table name
      std::string m_tableName;
      const coral::MySQLAccess::ISessionProperties& m_sessionProperties;
      coral::MySQLAccess::ISchemaProperties&        m_schemaProperties;
      coral::MySQLAccess::TableSchemaEditor*        m_tableSchemaEditor;
      coral::MySQLAccess::DataEditor*               m_tableDataEditor;
      coral::MySQLAccess::PrivilegeManager*         m_tablePrivilegeManager;
    };
  }
}

#endif // CORAL_MYSQLACCESS_TABLE_H
