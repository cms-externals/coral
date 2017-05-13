// $Id: Schema.cpp,v 1.24 2011/03/22 10:36:50 avalassi Exp $

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "ISessionProperties.h"
#include "OracleTableBuilder.h"
#include "Query.h"
#include "Schema.h"
#include "Statement.h"
#include "Table.h"
#include "View.h"

coral::FrontierAccess::Schema::Schema( const coral::FrontierAccess::ISessionProperties& sessionProperties )
  : m_sessionProperties( sessionProperties ), m_tables(), m_views(), m_viewsReadFromDataDictionary( false )
{
}

coral::FrontierAccess::Schema::~Schema()
{
  for ( std::map< std::string, coral::FrontierAccess::Table* >::iterator iTable = m_tables.begin(); iTable != m_tables.end(); ++iTable )
    if ( iTable->second )
      delete iTable->second;

  m_tables.clear();

  m_notExistingTables.clear();

  for ( std::map< std::string, coral::FrontierAccess::View* >::iterator iView = m_views.begin(); iView != m_views.end(); ++iView )
    if ( iView->second )
      delete iView->second;

  m_views.clear();

  m_notExistingViews.clear();

  m_viewsReadFromDataDictionary = false;

  for ( std::map< std::string, std::vector< std::string >* >::iterator iTableColumns = m_tableColumns.begin(); iTableColumns != m_tableColumns.end(); ++iTableColumns )
    if ( iTableColumns->second )
      delete iTableColumns->second;

  m_tableColumns.clear();
}

std::string coral::FrontierAccess::Schema::schemaName() const
{
  return m_sessionProperties.schemaName();
}

void coral::FrontierAccess::Schema::readTableColumns(const std::string& tableName ) const
{
  std::string columnSql =
    "SELECT TABLE_NAME, COLUMN_NAME FROM ALL_TAB_COLUMNS WHERE OWNER=:\"schema\"";
  if ( tableName != "" )
    columnSql += " AND TABLE_NAME=:\"table\"";
  columnSql += " ORDER BY TABLE_NAME, COLUMN_ID";

  coral::FrontierAccess::Statement statement( m_sessionProperties, columnSql );
  statement.setNumberOfPrefetchedRows( 100 );
  coral::AttributeList bindData;
  bindData.extend<std::string>( "schema" );
  bindData["schema"].data<std::string>() = schemaName();
  if ( tableName != "" )
  {
    bindData.extend<std::string>( "table" );
    bindData["table"].data<std::string>() = tableName;
  }
  statement.execute( bindData, false );
  coral::AttributeList outputData;
  outputData.extend<std::string>("TABLE_NAME");
  const std::string& tName = outputData["TABLE_NAME"].data<std::string>();
  outputData.extend<std::string>("COLUMN_NAME");
  const std::string& cName = outputData["COLUMN_NAME"].data<std::string>();
  statement.defineOutput( outputData );
  while( statement.fetchNext() )
  {
    std::map< std::string, std::vector< std::string>* >::iterator iTableColumns = m_tableColumns.find( tName );
    std::vector< std::string>* columnNames;
    if ( iTableColumns == m_tableColumns.end() )
    {
      // the mapping doesn't yet exist
      columnNames = new std::vector< std::string>;
      m_tableColumns.insert( std::make_pair( tName, columnNames ) );
    }
    else
      columnNames = iTableColumns->second;
    columnNames->push_back( cName );
  }
}

std::set<std::string> coral::FrontierAccess::Schema::listTables() const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "ISchema::listTables" );

  std::set<std::string> result;

  if ( m_tableColumns.size() == 0 )
    readTableColumns( "" );

  for ( std::map< std::string, std::vector< std::string>* >::const_iterator iTableColumns = m_tableColumns.begin(); iTableColumns != m_tableColumns.end(); ++iTableColumns )
  {
    result.insert( iTableColumns->first );
  }

  return result;
}

bool coral::FrontierAccess::Schema::existsTable( const std::string& tableName ) const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "ISchema::existsTable" );

  if ( m_tableColumns.size() == 0 )
    // read entire list of tables and columns to reduce total number of queries
    readTableColumns( "" );

  if ( m_tableColumns.find( tableName ) != m_tableColumns.end() )
    return true;

  // try looking specifically for this table in case it has appeared since
  //   the last time the more generic query was cached, unless have already
  //   tried and failed to find it.

  if ( m_notExistingTables.find( tableName ) != m_notExistingTables.end() )
    return false;

  readTableColumns( tableName );
  if ( m_tableColumns.find( tableName ) != m_tableColumns.end() )
    return true;

  // not found, keep track of that name
  m_notExistingTables.insert( tableName );

  return false;
}

const std::vector< std::string >* coral::FrontierAccess::Schema::tableColumns( const std::string& tableName )
{
  if( ! existsTable( tableName ) )
    throw coral::TableNotExistingException( m_sessionProperties.domainProperties().service()->name(), schemaName() + ".\"" + tableName + "\"" );

  return m_tableColumns.find( tableName )->second;
}

void coral::FrontierAccess::Schema::dropTable( const std::string& /*tableName*/ )
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::dropTable" );
}

void coral::FrontierAccess::Schema::dropIfExistsTable( const std::string& /*tableName*/ )
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::dropIfExistsTable" );
}

coral::ITable& coral::FrontierAccess::Schema::createTable( const coral::ITableDescription& /*description*/ )
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::createTable" );
}

coral::ITable& coral::FrontierAccess::Schema::tableHandle( const std::string& tableName )
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "coral::FrontierAccess::Schema::tableHandle" );

  // The fully qualified table name
  std::string fullTableName = m_sessionProperties.schemaName() + ".\"" + tableName + "\"";

  if( ! this->existsTable( tableName ) )
    throw coral::TableNotExistingException( m_sessionProperties.domainProperties().service()->name(), fullTableName );

  // The table exists
  std::map< std::string, Table* >::iterator iTable = m_tables.find( tableName );

  coral::FrontierAccess::Table* table;
  if( iTable == m_tables.end() )
  {
    // Create the table object and insert it into the map
    table = new coral::FrontierAccess::Table( m_sessionProperties, tableName );
    m_tables.insert( std::make_pair( tableName, table ) );
  }
  else if( iTable->second == 0 )
  {
    // The map entry exists but not the Table object yet; create it
    table = new coral::FrontierAccess::Table( m_sessionProperties, tableName );
    iTable->second = table;
  }
  else
    table = iTable->second;

  return *table;
}

void coral::FrontierAccess::Schema::truncateTable( const std::string& )
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::truncateTable" );
}

void coral::FrontierAccess::Schema::callProcedure( const std::string& , const coral::AttributeList& )
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::callProcedure" );
}

coral::IQuery* coral::FrontierAccess::Schema::newQuery() const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "coral::FrontierAccess::Schema::newQuery" );
  return new coral::FrontierAccess::Query( m_sessionProperties );
}

coral::IViewFactory* coral::FrontierAccess::Schema::viewFactory()
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::viewFactory" );
}

bool coral::FrontierAccess::Schema::existsView( const std::string& viewName ) const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "coral::FrontierAccess::Schema::existsView" );

  // Check if the view is already in the local cache.
  if ( m_views.find( viewName ) == m_views.end() )
  {
    // Check if the data dictionary was read
    if ( m_viewsReadFromDataDictionary )
    {
      return false;
    }
    else
    {
      // See if have already checked for this and found it missing
      if ( m_notExistingViews.find( viewName ) != m_notExistingViews.end() )
        return false;

      // Check inside the database.

      // The fully qualified view name
      //std::string fullViewName = m_sessionProperties.schemaName() + ".\"" + viewName + "\"";

      coral::FrontierAccess::Statement
        statement( m_sessionProperties,
                   "SELECT AO.OBJECT_NAME FROM ALL_OBJECTS AO, ALL_VIEWS AV WHERE AO.OWNER=:\"owner\" AND AO.OBJECT_NAME=:\"view\" AND AO.OBJECT_NAME = AV.VIEW_NAME" );

      bool status = false;
      statement.setNumberOfPrefetchedRows( 100 );
      coral::AttributeList bindData;
      bindData.extend<std::string>( "owner" );
      bindData.extend<std::string>( "view" );
      bindData["owner"].data<std::string>() = m_sessionProperties.schemaName();
      bindData["view"].data<std::string>() = viewName;
      statement.execute( bindData, false );
      coral::AttributeList output;
      output.extend<std::string>( "OBJECT_NAME" );
      const std::string& vName = output.begin()->data<std::string>();
      statement.defineOutput( output );
      while( statement.fetchNext() )
      {
        if( vName == viewName )
        {
          status = true;
          break;
        }
      }

      if ( status == false )
      {
        // not found, keep track of that name
        m_notExistingViews.insert( viewName );

        return false;
      }
      else {
        // The view exists. Create the view object and insert it into the map
        coral::FrontierAccess::View* view = new coral::FrontierAccess::View( m_sessionProperties, viewName );
        m_views.insert( std::make_pair( viewName, view ) );
        return true;
      }
    }
  }

  return true;
}

void coral::FrontierAccess::Schema::dropView( const std::string& /*viewName*/ )
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::dropView" );
}

void coral::FrontierAccess::Schema::dropIfExistsView( const std::string& /*viewName*/)
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::Schema::dropIfExistsView" );
}

coral::IView&
coral::FrontierAccess::Schema::viewHandle( const std::string& viewName )
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "ISchema::viewHandle" );

  // Check if the view is already in the local cache.
  std::map< std::string, coral::FrontierAccess::View* >::iterator iView = m_views.find( viewName );

  // The fully qualified view name
  const std::string fullViewName = m_sessionProperties.schemaName() + ".\"" + viewName + "\"";

  if ( iView == m_views.end() )
  {
    // Check if the data dictionary was read
    if ( m_viewsReadFromDataDictionary )
      throw coral::ViewNotExistingException( m_sessionProperties.domainProperties().service()->name(), fullViewName );
    throw coral::Exception( "Views are not fully supported in FrontierAccess (bug #64083)", m_sessionProperties.domainProperties().service()->name(), "ISchema::viewHandle" );
    // FIXME - getting view description
    // Try to get the descriptor handle
    // The view exists. Create the view object and insert it into the map
    //     coral::FrontierAccess::View* view = new coral::FrontierAccess::View( m_sessionProperties, viewName, describeHandle );
    //     iView = m_views.insert( std::make_pair( viewName, view ) ).first;
  }
  else
  {
    throw coral::ViewNotExistingException( m_sessionProperties.domainProperties().service()->name(),fullViewName );
  }

  return *( iView->second );
}

std::set<std::string> coral::FrontierAccess::Schema::listViews() const
{
  if ( ! m_sessionProperties.isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties.domainProperties().service()->name(), "ISchema::listViews" );
  std::set<std::string> result;
  if ( ! m_viewsReadFromDataDictionary )
    this->readViewsFromDataDictionary();

  for ( std::map< std::string, coral::FrontierAccess::View*>::const_iterator iView = m_views.begin(); iView != m_views.end(); ++iView )
  {
    result.insert( iView->first );
  }

  return result;
}

void coral::FrontierAccess::Schema::reactOnEndOfTransaction()
{
  // since Frontier is read-only, nothing needs to be cleaned up
  //  at the end of a transaction
}

void coral::FrontierAccess::Schema::readViewsFromDataDictionary() const
{
  coral::FrontierAccess::Statement statement( m_sessionProperties, "SELECT VIEW_NAME FROM ALL_VIEWS WHERE OWNER=:\"owner\"" );
  statement.setNumberOfPrefetchedRows( 100 );
  coral::AttributeList bindData;
  bindData.extend<std::string>( "owner" );
  bindData.begin()->data<std::string>() = m_sessionProperties.schemaName();
  statement.execute( bindData, false );
  coral::AttributeList output;
  output.extend<std::string>( "VIEW_NAME" );
  const std::string& viewName = output.begin()->data<std::string>();
  statement.defineOutput( output );
  while( statement.fetchNext() )
  {
    if ( m_views.find( viewName ) == m_views.end() )
    {
      m_views[viewName] = 0;
    }
  }
  m_viewsReadFromDataDictionary = true;
}

#ifdef CORAL240SQ
//-----------------------------------------------------------------------------

coral::ISequence&
coral::FrontierAccess::Schema::createSequence( const coral::ISequenceDescription& )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "FrontierAccess", "Schema::createSequence" );
}

//-----------------------------------------------------------------------------

bool
coral::FrontierAccess::Schema::existsSequence( const std::string& /*sequenceName*/ ) const
{
  // Temporary
  throw coral::Exception( "not implemented yet", "FrontierAccess", "Schema::existsSequence" );
}

//-----------------------------------------------------------------------------

coral::ISequence&
coral::FrontierAccess::Schema::sequenceHandle( const std::string& /*sequenceName*/ )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "FrontierAccess", "Schema::sequenceHandle" );
}

//-----------------------------------------------------------------------------

void
coral::FrontierAccess::Schema::dropSequence( const std::string& /*sequenceName*/ )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "FrontierAccess", "Schema::dropSequence" );
}

//-----------------------------------------------------------------------------

void
coral::FrontierAccess::Schema::dropIfExistsSequence( const std::string& /*sequenceName*/ )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "FrontierAccess", "Schema::dropIfExistsSequence" );
}

//-----------------------------------------------------------------------------

std::set<std::string>
coral::FrontierAccess::Schema::listSequences() const
{
  // Temporary
  throw coral::Exception( "not implemented yet", "FrontierAccess", "Schema::listSequences" );
}

#endif
//-----------------------------------------------------------------------------
