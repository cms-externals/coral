// $Id: SchemaEditor.h,v 1.5 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_SCHEMAEDITOR_H
#define CORAL_MYSQLACCESS_SCHEMAEDITOR_H 1

#include <set>
#include <string>

namespace coral
{
  namespace MySQLAccess
  {
    class ISessionProperties;
    class ISchemaProperties;

    /** The class allows to physically perform schema ("database" in MySQL jargon) inspection */
    struct SchemaEditor
    {
      /// Constructor
      SchemaEditor( const ISessionProperties& sessionProperties, ISchemaProperties& schemaProperties );
      /// Get the list of table for the current schema from the db server
      std::set<std::string> listTables();
      /// Wipe out all the rows in the table in the current schema
      void truncateTable( const std::string& );
      /// Drop the table in the current schema
      void dropTable( const std::string& );
      /// Create the table in the current schema
      void createTable( const std::string&, const std::string& );

      const ISessionProperties& m_sessionProperties;
      ISchemaProperties&  m_schemaProperties;
    };
  }
}

#endif // CORAL_MYSQLACCESS_SCHEMAEDITOR_H
