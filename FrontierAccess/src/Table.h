#ifndef FRONTIER_ACCESS_TABLE_H
#define FRONTIER_ACCESS_TABLE_H

#include "RelationalAccess/ITable.h"
#include <string>

namespace coral {

  namespace FrontierAccess {

    class ISessionProperties;
    class TableDescriptionProxy;

    /**
     * Class Table
     *
     * Implementation of the ITable interface for the FrontierAccess package
     */
    class Table : virtual public coral::ITable
    {
    public:
      /// Constructor
      Table( const ISessionProperties& sessionProperties, const std::string& tableName );

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
      const ISessionProperties& m_sessionProperties;

      /// The proxy to the table description
      TableDescriptionProxy*    m_descriptionProxy;

    };

  }

}

#endif
