// $Id: SchemaProperties.h,v 1.5 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_SCHEMAPROPERTIES_H
#define MYSQLACCESS_SCHEMAPROPERTIES_H 1

#include "ISchemaProperties.h"

namespace coral
{
  namespace MySQLAccess
  {
    class ISessionProperties;
    class Schema;

    /** Class coral::MySQLAccess::SchemaProperties
     *  Implements the registry of schema properties and its schema objects
     */
    class SchemaProperties : virtual public ISchemaProperties
    {
    public:
      SchemaProperties( const ISessionProperties&, const std::string&, Schema& );
      virtual ~SchemaProperties();

      /// Return the schema name
      virtual std::string schemaName() const;
      /// Return cached names of the tables
      virtual std::set<std::string>& tableNames();
      /// Return cached names of the views
      virtual std::set<std::string>& viewNames();
      /// Return the registry of table descriptions
      virtual TableDescriptionRegistry& tableDescriptionRegistry();
      /// Return the registry of tables
      virtual TableRegistry& tableRegistry();
      /// Return table description by table name
      virtual coral::TableDescription& tableDescription( const std::string& );
      /// Return table description by table name
      virtual const coral::ITableDescription& tableDescription( const std::string& ) const;

    private:
      SchemaProperties();
      SchemaProperties( const SchemaProperties& );

    private:
      /// Current session properties
      const ISessionProperties& m_sessprops;
      /// Current schema name
      std::string m_schemaName;
      /// Current schema
      Schema&                   m_schema;
      /// Cached table names
      std::set<std::string>     m_tableNames;
      /// Cached view names
      std::set<std::string>     m_viewNames;
      /// Currently opened table descriptions
      TableDescriptionRegistry m_tableDescriptions;
      /// Currently opened table handles
      TableRegistry m_tables;
    };
  }
}

#endif // MYSQLACCESS_SCHEMAPROPERTIES_H
