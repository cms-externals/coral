// $Id: Table.h,v 1.9.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_TABLE_H
#define CORALACCESS_TABLE_H 1

// Include files
#include "RelationalAccess/ITable.h"

// Local include files
#include "TableDescriptionProxy.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class SessionProperties;
    class TableDataEditor;

    /** @class Table
     *
     *  @author Andrea Valassi
     *  @date   2007-12-05
     */

    class Table : virtual public coral::ITable
    {

    public:

      /// Constructor
      Table( const SessionProperties& sessionProperties,
             const std::string& schemaName,
             const std::string& tableName );

      /// Destructor
      virtual ~Table();

      /**
       * Returns the description of the table.
       */
      const ITableDescription& description() const;

      /**
       * Returns a reference to the schema editor for the table.
       */
      ITableSchemaEditor& schemaEditor();

      /**
       * Returns a reference to the ITableDataEditor object  for the table.
       */
      ITableDataEditor& dataEditor();

      /**
       * Returns a reference to the privilege manager of the table.
       */
      ITablePrivilegeManager& privilegeManager();

      /**
       * Returns a new query object for performing a query involving this table only.
       */
      IQuery* newQuery() const;

    private:

      /// The session properties.
      const SessionProperties& m_sessionProperties;

      /// The schema name.
      /// [NB this is guaranteed to be != ""]
      const std::string m_schemaName;

      /// The table description (owned by this Table).
      TableDescriptionProxy* m_description;

      /// The table data editor (owned by this Table).
      TableDataEditor* m_dataEditor;

    };

  }

}
#endif // CORALACCESS_TABLE_H
