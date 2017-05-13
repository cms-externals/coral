// $Id: SchemaProperties.cpp,v 1.7 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "RelationalAccess/TableDescription.h"

#include "ISessionProperties.h"
#include "Schema.h"
#include "SchemaProperties.h"
#include "Table.h"

namespace coral
{
  namespace MySQLAccess
  {
    SchemaProperties::SchemaProperties( const ISessionProperties& sessprops, const std::string& schemaName, Schema& schema )
      : m_sessprops( sessprops ),
        m_schemaName( schemaName ),
        m_schema( schema ),
        m_tableNames(),
        m_viewNames(),
        m_tableDescriptions(),
        m_tables()
    {
      this->m_tableNames.clear();
      this->m_viewNames.clear();
      this->m_tableDescriptions.clear();
      this->m_tables.clear();
    }

    SchemaProperties::~SchemaProperties()
    {
      TableDescriptionRegistry::iterator tdri = this->tableDescriptionRegistry().begin();
      while( tdri != this->tableDescriptionRegistry().end() )
      {
        delete (*tdri).second;
        ++tdri;
      }

      TableRegistry::iterator tri = this->tableRegistry().begin();
      while( tri != this->tableRegistry().end() )
      {
        delete (*tri).second;
        ++tri;
      }
    }

    std::string SchemaProperties::schemaName() const
    {
      return this->m_schemaName;
    }

    std::set<std::string>& SchemaProperties::tableNames()
    {
      return this->m_tableNames;
    }

    std::set<std::string>& SchemaProperties::viewNames()
    {
      return this->m_viewNames;
    }

    TableDescriptionRegistry& SchemaProperties::tableDescriptionRegistry()
    {
      return this->m_tableDescriptions;
    }

    TableRegistry& SchemaProperties::tableRegistry()
    {
      return this->m_tables;
    }

    coral::TableDescription& SchemaProperties::tableDescription( const std::string& tableName )
    {
      TableDescriptionRegistry::iterator tdri = this->m_tableDescriptions.find( tableName );

      if( tdri == this->m_tableDescriptions.end() )
        throw 0;

      return *((*tdri).second);
    }

    const coral::ITableDescription& SchemaProperties::tableDescription( const std::string& tableName ) const
    {
      TableDescriptionRegistry::const_iterator tdri = this->m_tableDescriptions.find( tableName );

      if( tdri == this->m_tableDescriptions.end() )
        throw 0;

      return *((*tdri).second);
    }

  } // namespace MySQLAccess
} // namespace coral
