// $Id: ISchemaProperties.h,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_ISCHEMAPROPERTIES_H
#define MYSQLACCESS_ISCHEMAPROPERTIES_H 1

#include <map>
#include <set>
#include <string>

namespace coral
{
  class ITableDescription;
  class TableDescription;

  namespace MySQLAccess
  {
    class Table;

    typedef std::map<std::string, coral::TableDescription*> TableDescriptionRegistry;
    typedef std::map<std::string, Table*>           TableRegistry;

    /** The interface for the registry holding schema properties */
    class ISchemaProperties
    {
    public:
      /// Return the schema name
      virtual std::string schemaName() const = 0;
      /// Return cached names of the tables
      virtual std::set<std::string>& tableNames() = 0;
      /// Return cached names of the views
      virtual std::set<std::string>& viewNames() = 0;
      /// Return the registry of table descriptions
      virtual TableDescriptionRegistry& tableDescriptionRegistry() = 0;
      /// Return the registry of tables
      virtual TableRegistry& tableRegistry() = 0;
      /// Return table description by table name
      virtual coral::TableDescription& tableDescription( const std::string& ) = 0;
      /// Return table description by table name
      virtual const coral::ITableDescription& tableDescription( const std::string& ) const = 0;

      // Here we might provide short-cuts for managing the cache, to be revisited

      // FIXME - Get access to cached table descriptions
      // FIXME - Solve the puzzle around table schema editor
      // FIXME - Add the view editor or builder

      virtual ~ISchemaProperties() { }
    };
  }
}

#endif // MYSQLACCESS_ISCHEMAPROPERTIES_H
