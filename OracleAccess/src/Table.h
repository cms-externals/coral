#ifndef ORACLEACCESS_TABLE_H
#define ORACLEACCESS_TABLE_H 1

#include <string>
#include <boost/shared_ptr.hpp>
#include "RelationalAccess/ITable.h"

struct OCIDescribe;

namespace coral
{

  namespace OracleAccess
  {

    class DataEditor;
    class PrivilegeManager;
    class SessionProperties;
    class TableDescriptionProxy;

    /**
     * Class Table
     *
     * Implementation of the ITable interface for the OracleAccess package
     */
    class Table : virtual public coral::ITable
    {
    public:
      /// Constructor
      Table( boost::shared_ptr<const SessionProperties> properties,
             const std::string& schemaName,
             const std::string& tableName,
             OCIDescribe* descriptor );

      /// Destructor
      virtual ~Table();

      /**
       * Returns the description of the table.
       */
      const coral::ITableDescription& description() const;

      /**
       * Returns a reference to the schema editor for the table.
       */
      coral::ITableSchemaEditor& schemaEditor();

      /**
       * Returns a reference to the ITableDataEditor object  for the table.
       */
      coral::ITableDataEditor& dataEditor();

      /**
       * Returns a reference to the privilege manager of the table.
       */
      coral::ITablePrivilegeManager& privilegeManager();

      /**
       * Returns a new query object for performing a query involving this table only.
       */
      coral::IQuery* newQuery() const;

    private:

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The proxy to the table description
      TableDescriptionProxy* m_descriptionProxy;

      /// The privilege manager
      PrivilegeManager* m_privilegeManager;

      /// The data editor
      DataEditor* m_dataEditor;
    };

  }

}

#endif
