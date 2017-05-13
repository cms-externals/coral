// $Id: Table.cpp,v 1.10 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralKernel/Service.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

#include "DataEditor.h"
#include "DomainProperties.h"
#include "ISessionProperties.h"
#include "PrivilegeManager.h"
#include "Query.h"
#include "SchemaProperties.h"
#include "Table.h"
#include "TableSchemaEditor.h"

namespace coral
{
  namespace MySQLAccess
  {
    Table::Table( const std::string& tableName, const ISessionProperties& sessionProperties, ISchemaProperties& schemaProperties )
      : m_tableName( tableName )
      , m_sessionProperties( sessionProperties )
      , m_schemaProperties( schemaProperties )
      , m_tableSchemaEditor( 0 )
      , m_tableDataEditor( 0 )
      , m_tablePrivilegeManager( 0 )
    {
      TableDescriptionRegistry::iterator tdri = m_schemaProperties.tableDescriptionRegistry().find( m_tableName );

      coral::TableDescription* newDescription = new coral::TableDescription( m_tableName );
      newDescription->setName( m_tableName );

      if( tdri == m_schemaProperties.tableDescriptionRegistry().end() )
        m_schemaProperties.tableDescriptionRegistry().insert( std::make_pair( m_tableName, newDescription ) );

      m_tableSchemaEditor     = new coral::MySQLAccess::TableSchemaEditor( tableName, sessionProperties, schemaProperties, /*refresh=*/ true );
      m_tablePrivilegeManager = new coral::MySQLAccess::PrivilegeManager( sessionProperties, tableName );
      m_tableDataEditor       = new coral::MySQLAccess::DataEditor( sessionProperties, m_schemaProperties.tableDescription( m_tableName ) );
    }

    Table::~Table()
    {
      if( this->m_tableSchemaEditor != 0 )
      {
        delete this->m_tableSchemaEditor;
        this->m_tableSchemaEditor = 0;
      }

      if( this->m_tableDataEditor != 0 )
      {
        delete this->m_tableDataEditor;
        this->m_tableDataEditor = 0;
      }

      if( this->m_tablePrivilegeManager != 0 )
      {
        delete this->m_tablePrivilegeManager;
        this->m_tablePrivilegeManager = 0;
      }
    }

    const ITableDescription& Table::description() const
    {
      const ITableDescription& tableDescription = this->m_schemaProperties.tableDescription( this->m_tableName );
      return tableDescription;
    }

    ITableSchemaEditor& Table::schemaEditor()
    {
      if ( ! m_sessionProperties.isTransactionActive() )
        throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "ITable::schemaEditor" );
      if ( m_sessionProperties.isReadOnly() )
        throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "ITable::schemaEditor" );
      if ( m_sessionProperties.isTransactionReadOnly() )
        throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties.domainProperties().service()->name(), "ITable::schemaEditor" );
      return *(this->m_tableSchemaEditor);
    }

    ITableDataEditor& Table::dataEditor()
    {
      if ( ! m_sessionProperties.isTransactionActive() )
        throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "ITable::dataEditor" );
      if ( m_sessionProperties.isReadOnly() )
        throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "ITable::dataEditor" );
      if ( m_sessionProperties.isTransactionReadOnly() )
        throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties.domainProperties().service()->name(), "ITable::dataEditor" );
      return *(this->m_tableDataEditor);
    }

    ITablePrivilegeManager& Table::privilegeManager()
    {
      if ( ! m_sessionProperties.isTransactionActive() )
        throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "ITable::privilegeManager" );
      if ( m_sessionProperties.isReadOnly() )
        throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "ITable::privilegeManager" );
      if ( m_sessionProperties.isTransactionReadOnly() )
        throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties.domainProperties().service()->name(), "ITable::privilegeManager" );
      return *(this->m_tablePrivilegeManager);
    }

    IQuery* Table::newQuery() const
    {
      return new coral::MySQLAccess::Query( m_sessionProperties, m_tableName );
    }
  }
}
