// $Id: Table.cpp,v 1.5 2010/03/09 17:17:04 avalassi Exp $
#include "Table.h"
#include "ISessionProperties.h"
#include "DomainProperties.h"
#include "TableDescriptionProxy.h"
#include "Query.h"

#include "RelationalAccess/SessionException.h"

#include "CoralKernel/Service.h"

coral::FrontierAccess::Table::Table( const coral::FrontierAccess::ISessionProperties& sessionProperties, const std::string& tableName )
  : m_sessionProperties( sessionProperties )
  , m_descriptionProxy( new coral::FrontierAccess::TableDescriptionProxy( sessionProperties, tableName ) )
{
}

coral::FrontierAccess::Table::~Table()
{
  delete m_descriptionProxy;
}

const coral::ITableDescription& coral::FrontierAccess::Table::description() const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "coral::FrontierAccess::Table::description" );
  return static_cast< const coral::ITableDescription& >( *m_descriptionProxy );
}

coral::ITableSchemaEditor& coral::FrontierAccess::Table::schemaEditor()
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Table::schemaEditor" );
  //if ( ! m_sessionProperties.isTransactionActive() ) throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "coral::FrontierAccess::Table::schemaEditor" );
  //return static_cast< coral::ITableSchemaEditor& >( *m_descriptionProxy );
}

coral::ITableDataEditor& coral::FrontierAccess::Table::dataEditor()
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Table::dataEditor" );
}

coral::ITablePrivilegeManager& coral::FrontierAccess::Table::privilegeManager()
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Table::privilegeManager" );
}

coral::IQuery* coral::FrontierAccess::Table::newQuery() const
{
  return new coral::FrontierAccess::Query( m_sessionProperties, m_descriptionProxy->name() );
}
