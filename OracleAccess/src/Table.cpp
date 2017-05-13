#include "CoralKernel/Service.h"
#include "RelationalAccess/SessionException.h"

#include "DataEditor.h"
#include "DomainProperties.h"
#include "PrivilegeManager.h"
#include "Query.h"
#include "SessionProperties.h"
#include "Table.h"
#include "TableDescriptionProxy.h"


coral::OracleAccess::Table::Table( boost::shared_ptr<const SessionProperties> sessionProperties,
                                   const std::string& schemaName,
                                   const std::string& tableName,
                                   OCIDescribe* descriptor ) :
  m_sessionProperties( sessionProperties ),
  m_descriptionProxy( new coral::OracleAccess::TableDescriptionProxy( sessionProperties,
                                                                      schemaName,
                                                                      descriptor,
                                                                      tableName ) ),
  m_privilegeManager( new coral::OracleAccess::PrivilegeManager( sessionProperties,
                                                                 schemaName,
                                                                 schemaName + ".\"" + tableName + "\"" ) ),
  m_dataEditor( new coral::OracleAccess::DataEditor( sessionProperties, schemaName, *m_descriptionProxy ) )
{
}


coral::OracleAccess::Table::~Table()
{
  delete m_dataEditor;
  delete m_privilegeManager;
  delete m_descriptionProxy;
}


const coral::ITableDescription&
coral::OracleAccess::Table::description() const
{
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ITable::description" );
  return static_cast< const coral::ITableDescription& >( *m_descriptionProxy );
}


coral::ITableSchemaEditor&
coral::OracleAccess::Table::schemaEditor()
{
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ITable::schemaEditor" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ITable::schemaEditor" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ITable::schemaEditor" );
  return static_cast< coral::ITableSchemaEditor& >( *m_descriptionProxy );
}


coral::ITableDataEditor&
coral::OracleAccess::Table::dataEditor()
{
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ITable::dataEditor" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ITable::dataEditor" );
  // This is not strictly necessary (would fail with ORA-01456 anyway)
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::dataEditor" );
  return *m_dataEditor;
}


coral::ITablePrivilegeManager&
coral::OracleAccess::Table::privilegeManager()
{
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ITable::privilegeManager" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ITable::privilegeManager" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ITable::privilegeManager" );
  return *m_privilegeManager;
}


coral::IQuery*
coral::OracleAccess::Table::newQuery() const
{
  return new coral::OracleAccess::Query( m_sessionProperties,
                                         m_descriptionProxy->schemaName(),
                                         m_descriptionProxy->name() );
}
