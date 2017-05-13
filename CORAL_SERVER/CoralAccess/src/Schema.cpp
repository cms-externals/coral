// $Id: Schema.cpp,v 1.15.2.1 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include <iostream>
#include "CoralServerBase/InternalErrorException.h"
#include "CoralServerBase/NotImplemented.h"
#include "RelationalAccess/SchemaException.h"

// Local include files
#include "ConnectionProperties.h"
#include "Query.h"
#include "Schema.h"
#include "SessionProperties.h"
#include "Table.h"
#include "View.h"
#include "logger.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

Schema::Schema( const SessionProperties& sessionProperties,
                const std::string& schemaName )
  : m_sessionProperties( sessionProperties )
  , m_schemaName( schemaName )
  , m_tables()
  , m_tableListFresh( false )
  , m_views()
  , m_viewListFresh( false )
{
  logger << "Create Schema" << endlog;
}

//-----------------------------------------------------------------------------

Schema::~Schema()
{
  logger << "Delete Schema" << endlog;
  for ( std::map< std::string, Table* >::iterator
          ppTable = m_tables.begin(); ppTable != m_tables.end(); ppTable++ )
  {
    if ( ppTable->second ) delete ppTable->second;
    ppTable->second = 0;
  }
  for ( std::map< std::string, View* >::iterator
          ppView = m_views.begin(); ppView != m_views.end(); ppView++ )
  {
    if ( ppView->second ) delete ppView->second;
    ppView->second = 0;
  }
}

//-----------------------------------------------------------------------------

std::string Schema::schemaName() const
{
  return m_schemaName;
}

//-----------------------------------------------------------------------------

std::set<std::string> Schema::listTables() const
{
  std::set<std::string> tableList;
  if ( m_tableListFresh )
  {
    logger << "Table list has already been fetched from the data dictionary" << endlog;
    for ( std::map< std::string, Table* >::const_iterator
            ppTable = m_tables.begin(); ppTable != m_tables.end(); ppTable++ )
      tableList.insert( ppTable->first );
  }
  else
  {
    logger << "Fetch table list from the data dictionary" << endlog;
    if ( ! m_sessionProperties.isTransactionActive() )
      throw TransactionNotActiveException( "coral::Access", "Schema::listTables" );
    tableList = facade().listTables( m_sessionProperties.sessionID(), m_schemaName );
    for ( std::set<std::string>::const_iterator
            pTable = tableList.begin(); pTable != tableList.end(); pTable++ )
      m_tables[ *pTable ] = 0;
    m_tableListFresh = true;
  }
  return tableList;
}

//-----------------------------------------------------------------------------

bool Schema::existsTable( const std::string& tableName ) const
{
  // Table name is cached (table handle may not exist yet)
  if ( m_tables.find( tableName ) != m_tables.end() ) return true;
  // Table name list is fully cached: table does not exist
  else if ( m_tableListFresh ) return false;
  // Table name is not cached: check its existence
  bool exists = facade().existsTable( m_sessionProperties.sessionID(), m_schemaName, tableName );
  if ( exists ) m_tables[ tableName ] = 0;
  return exists;
}

//-----------------------------------------------------------------------------

void Schema::dropTable( const std::string& /*tableName*/ )
{
  throw NotImplemented("Schema::dropTable");
}

//-----------------------------------------------------------------------------

void Schema::dropIfExistsTable( const std::string& /*tableName*/ )
{
  throw NotImplemented("Schema::dropIfExistsTable");
}

//-----------------------------------------------------------------------------

coral::ITable& Schema::createTable( const coral::ITableDescription& /*description*/ )
{
  throw NotImplemented("Schema::createTable");
}

//-----------------------------------------------------------------------------

coral::ITable& Schema::tableHandle( const std::string& tableName )
{
  // Check if the table exists (contact the database if necessary)
  if ( !existsTable( tableName ) )
    throw coral::TableNotExistingException( "coral::CoralAccess", tableName );
  // The table does exist: return its handle (create it if not yet done)
  Table** ppTable = &( m_tables[ tableName ] );
  if ( !( *ppTable ) ) *ppTable = new Table( m_sessionProperties, m_schemaName, tableName );
  return **ppTable;
}

//-----------------------------------------------------------------------------

void Schema::truncateTable( const std::string& /*tableName*/ )
{
  throw NotImplemented("Schema::truncateTable");
}

//-----------------------------------------------------------------------------

void Schema::callProcedure( const std::string& procedureName,
                            const coral::AttributeList& inputArguments )
{
  facade().callProcedure( m_sessionProperties.sessionID(), m_schemaName, procedureName, inputArguments );
}

//-----------------------------------------------------------------------------

coral::IQuery* Schema::newQuery() const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw TransactionNotActiveException( "coral::Access", "Schema::newQuery" );
  return new Query( m_sessionProperties, m_schemaName );
}

//-----------------------------------------------------------------------------

coral::IViewFactory* Schema::viewFactory()
{
  throw NotImplemented("Schema::viewFactory");
}

//-----------------------------------------------------------------------------

bool Schema::existsView( const std::string& viewName ) const
{
  // View name is cached (view handle may not exist yet)
  if ( m_views.find( viewName ) != m_views.end() ) return true;
  // View name list is fully cached: view does not exist
  else if ( m_viewListFresh ) return false;
  // View name is not cached: check its existence
  bool exists = facade().existsView( m_sessionProperties.sessionID(), m_schemaName, viewName );
  if ( exists ) m_views[ viewName ] = 0;
  return exists;
}

//-----------------------------------------------------------------------------

void Schema::dropView( const std::string& /*viewName*/ )
{
  throw NotImplemented("Schema::dropView");
}

//-----------------------------------------------------------------------------

void Schema::dropIfExistsView( const std::string& /*viewName*/ )
{
  throw NotImplemented("Schema::dropIfExistsView");
}

//-----------------------------------------------------------------------------

std::set<std::string> Schema::listViews() const
{
  std::set<std::string> viewList;
  if ( m_viewListFresh )
  {
    logger << "View list has already been fetched from the data dictionary" << endlog;
    for ( std::map< std::string, View* >::const_iterator
            ppView = m_views.begin(); ppView != m_views.end(); ppView++ )
      viewList.insert( ppView->first );
  }
  else
  {
    logger << "Fetch view list from the data dictionary" << endlog;
    if ( ! m_sessionProperties.isTransactionActive() )
      throw TransactionNotActiveException( "coral::Access", "Schema::listViews" );
    viewList = facade().listViews( m_sessionProperties.sessionID(), m_schemaName );
    for ( std::set<std::string>::const_iterator
            pView = viewList.begin(); pView != viewList.end(); pView++ )
      m_views[ *pView ] = 0;
    m_viewListFresh = true;
  }
  return viewList;
}

//-----------------------------------------------------------------------------

coral::IView& Schema::viewHandle( const std::string& viewName )
{
  // Check if the view exists (contact the database if necessary)
  if ( !existsView( viewName ) )
    throw coral::ViewNotExistingException( "coral::CoralAccess", viewName );
  // The view does exist: return its handle (create it if not yet done)
  View** ppView = &( m_views[ viewName ] );
  if ( !( *ppView ) ) *ppView = new View( m_sessionProperties, m_schemaName, viewName );
  return **ppView;
}

//-----------------------------------------------------------------------------

const coral::ICoralFacade& Schema::facade() const
{
  return m_sessionProperties.connectionProperties().facade();
}

//-----------------------------------------------------------------------------
