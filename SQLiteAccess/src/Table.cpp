#include <iostream>
#include "Table.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "TableDescriptionProxy.h"
#include "DataEditor.h"
#include "PrivilegeManager.h"
#include "Query.h"

#include "RelationalAccess/SessionException.h"

#include "CoralKernel/Service.h"

//#include "sqlite3.h"
//#include <cstdlib>
//#include <iostream>
coral::SQLiteAccess::Table::Table(boost::shared_ptr<const SessionProperties> properties, const std::string& tableName) :
  m_properties(properties),
  m_descriptionProxy(new coral::SQLiteAccess::TableDescriptionProxy( m_properties, tableName)),
  m_privilegeManager( new coral::SQLiteAccess::PrivilegeManager( m_properties, m_properties->schemaName() + "." + tableName) ),
  m_dataEditor(new coral::SQLiteAccess::DataEditor( m_properties, *m_descriptionProxy))
{

}

coral::SQLiteAccess::Table::~Table()
{

  if ( m_dataEditor ) delete m_dataEditor;
  if ( m_privilegeManager ) delete m_privilegeManager;
  if ( m_descriptionProxy ) delete m_descriptionProxy;
}

const coral::ITableDescription&
coral::SQLiteAccess::Table::description() const
{

  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ITable::description" );
  return static_cast< const coral::ITableDescription& >( *m_descriptionProxy );
}

coral::ITableSchemaEditor&
coral::SQLiteAccess::Table::schemaEditor()
{

  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(), "ITable::schemaEditor" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ITable::schemaEditor" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ITable::schemaEditor" );
  return static_cast< coral::ITableSchemaEditor& >( *m_descriptionProxy );
}

coral::ITableDataEditor&
coral::SQLiteAccess::Table::dataEditor()
{

  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(), "ITable::dataEditor" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ITable::dataEditor" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ITable::dataEditor" );
  return *m_dataEditor;
}

coral::ITablePrivilegeManager&
coral::SQLiteAccess::Table::privilegeManager()
{

  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ITable::privilegeManager" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ITable::privilegeManager" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ITable::privilegeManager" );
  return *m_privilegeManager;
}

coral::IQuery*
coral::SQLiteAccess::Table::newQuery() const
{

  return new coral::SQLiteAccess::Query( m_properties,
                                         m_descriptionProxy->name() );
}
