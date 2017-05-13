#ifndef SQLITEACCESS_TABLE_H
#define SQLITEACCESS_TABLE_H

#include "RelationalAccess/ITable.h"
#include <string>
#include <boost/shared_ptr.hpp>

//class sqlite3;
namespace seal {
  class Service;
}

namespace coral {

  namespace SQLiteAccess {

    class SessionProperties;
    class TableDescriptionProxy;
    class PrivilegeManager;
    class DataEditor;
    /**
     * @class Table Table.h
     *
     * Implementation of the ITable interface for the SQLiteAccess plugin
     @author Zhen Xie
    */

    class Table : virtual public coral::ITable
    {
    public:
      /// Constructor
      Table( boost::shared_ptr<const SessionProperties> properties,
             const std::string& tableName);
      /// Destructor
      virtual ~Table();
      /// Methods from ITable
      /**
       * Returns the name of the table.
       */
      //const std::string& name() const;
      /**
       * Returns the description of the table.
       */
      const coral::ITableDescription& description() const;
      /**
       * Returns a reference to the object which allows the altering of the schema of the table.
       */
      coral::ITableSchemaEditor& schemaEditor();
      /**
       * Returns the object which controls the privileges for this table
       */
      coral::ITablePrivilegeManager& privilegeManager();
      /**
       * Returns the number of rows in this table. WARNING : THIS MAY BE A VERY SLOW OPERATION !!!
       */
      //long numberOfRows() const;
      /**
       * Returns a reference to the object which allows to add, delete and updates rows in the table.
       */
      coral::ITableDataEditor& dataEditor();
      /**
       * Creates an object for queries in the table.
       */
      coral::IQuery* newQuery() const;
    private:
      /// The session properties
      boost::shared_ptr<const SessionProperties> m_properties;

      /// The proxy to the table description
      TableDescriptionProxy*    m_descriptionProxy;
      /// The privilege manager
      PrivilegeManager*         m_privilegeManager;
      /// The data editor
      DataEditor*               m_dataEditor;
    };
  } //ns SQLiteAccess
} //ns coral
#endif
