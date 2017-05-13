// $Id: Table.cpp,v 1.15.2.1 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include <iostream>
#include "CoralServerBase/InternalErrorException.h"
#include "CoralServerBase/NotImplemented.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"

// Local include files
#include "Query.h"
#include "SessionProperties.h"
#include "Table.h"
#include "TableDataEditor.h"
#include "logger.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

Table::Table( const SessionProperties& sessionProperties,
              const std::string& schemaName,
              const std::string& tableName )
  : m_sessionProperties( sessionProperties )
  , m_schemaName( schemaName )
  , m_description( new TableDescriptionProxy( sessionProperties, schemaName, tableName ) )
  , m_dataEditor( new TableDataEditor( sessionProperties, *m_description ) )
{
  logger << "Create Table '" << tableName << "'" << endlog;
  if ( m_schemaName == "" )
    throw InternalErrorException( "PANIC! Invalid schema name ''",
                                  "Table::Table()",
                                  "coral::CoralAccess" );
}

//-----------------------------------------------------------------------------

Table::~Table()
{
  if ( m_description ) logger << "Delete Table '" << m_description->name() << "'" << endlog;
  else logger << "Delete Table (WARNING: invalid description)" << endlog;
  if ( m_dataEditor ) delete m_dataEditor;
  m_dataEditor = 0;
  if ( m_description ) delete m_description;
  m_description = 0;
}

//-----------------------------------------------------------------------------

const coral::ITableDescription& Table::description() const
{
  if ( ! m_description )
    throw InternalErrorException( "PANIC! Invalid table description",
                                  "Table::description",
                                  "coral::CoralAccess" );
  return *m_description;
}

//-----------------------------------------------------------------------------

coral::ITableSchemaEditor& Table::schemaEditor()
{
  throw NotImplemented("Table::schemaEditor");
}

//-----------------------------------------------------------------------------

coral::ITableDataEditor& Table::dataEditor()
{
  if ( m_sessionProperties.accessMode() != coral::ReadOnly )
    throw coral::InvalidOperationInReadOnlyModeException( "coral::CoralAccess",
                                                          "Table::dataEditor" );
  if ( ! m_dataEditor )
    throw InternalErrorException( "PANIC! Invalid table data editor",
                                  "Table::dataEditor",
                                  "coral::CoralAccess" );
  return *m_dataEditor;
}

//-----------------------------------------------------------------------------

coral::ITablePrivilegeManager& Table::privilegeManager()
{
  throw NotImplemented("Table::privilegeManager");
}

//-----------------------------------------------------------------------------

coral::IQuery* Table::newQuery() const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw TransactionNotActiveException( "coral::Access", "Table::newQuery" );
  return new Query( m_sessionProperties, m_schemaName, description().name() );
}

//-----------------------------------------------------------------------------
