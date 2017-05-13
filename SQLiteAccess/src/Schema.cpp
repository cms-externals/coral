#include <iostream>
#include <string>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "CoralKernel/Service.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "Query.h"
#include "Schema.h"
#include "SessionProperties.h"
#include "SQLiteStatement.h"
#include "SQLiteTableBuilder.h"
#include "Table.h"
#include "View.h"
#include "ViewFactory.h"

#define LOG( msg ){ coral::MessageStream myMsg("Schema"); myMsg << coral::Debug << msg << coral::MessageStream::endmsg; }

coral::SQLiteAccess::Schema::Schema( boost::shared_ptr<const SessionProperties> properties ) :
  m_properties(properties),
  m_tables(),
  m_tablesReadFromDataDictionary( false ),
  m_views(),
  m_viewsReadFromDataDictionary( false )
{
}


coral::SQLiteAccess::Schema::~Schema()
{
  // delete the tables and views
  this->reactOnEndOfTransaction();
}


std::string coral::SQLiteAccess::Schema::schemaName() const
{
  return "";
}


std::set<std::string>
coral::SQLiteAccess::Schema::listTables() const
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::listTables" );
  std::set<std::string> result;
  if ( ! m_tablesReadFromDataDictionary )
  {
    this->readTablesFromDataDictionary();
  }
  for ( std::map< std::string, coral::SQLiteAccess::Table* >::const_iterator iTable = m_tables.begin(); iTable != m_tables.end(); ++iTable )
  {
    if( iTable->first != "coral_sqlite_fk" ) // Fix bug #82563 and bug #82756
      result.insert( iTable->first );
  }
  return result;
}


bool
coral::SQLiteAccess::Schema::existsTable( const std::string& name ) const
{
  if ( ! m_properties->isTransactionActive() ) {
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(), "ISchema::existsTable" );
  }
  // Check if the table is already in the local cache.
  if ( m_tables.find( name ) == m_tables.end() )
  {
    // Check if the data dictionary was read
    if ( m_tablesReadFromDataDictionary )
    {
      return false;
    }
    else
    {
      // Check inside the database.
      readTablesFromDataDictionary();

      // Was there an entry in the database?
      std::map< std::string, coral::SQLiteAccess::Table* >::iterator iTable = m_tables.find( name );
      if ( iTable == m_tables.end() ) return false;
      // The view exists. Create the view object and insert it into the map
      if( !iTable->second )
      {
        // Override zero entry with some content
        iTable->second = new coral::SQLiteAccess::Table( m_properties, name);
      }
      return true;
    }
  }
  return true;
}


coral::ITable&
coral::SQLiteAccess::Schema::createTable( const coral::ITableDescription& description )
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::createTable" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ISchema::createTable" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::createTable" );

  // Get the name of the table
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(),
                                                          "ISchema::createTable" );
  std::string name = description.name();
  // Check if a table with this name already exists
  if ( this->existsTable( name ) ) {
    throw coral::TableAlreadyExistingException( m_properties->domainProperties().service()->name(),name);
  }
  // Construct and execute the statement creating the table
  coral::SQLiteAccess::SQLiteTableBuilder builder( description, m_properties);
  coral::SQLiteAccess::SQLiteStatement statement( m_properties);
  statement.prepare( builder.statement() );
  if ( ! statement.execute() ) {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not create a new table","ISchema::createTable" );
  }
  //register the new table
  coral::SQLiteAccess::Table* t=new coral::SQLiteAccess::Table(m_properties,name);
  m_tables.insert( std::make_pair(name,t) );
  // Get the table handle
  coral::ITable& table = this->tableHandle(name);
  // Use now the schema editor to add the indices
  coral::ITableSchemaEditor& editor = table.schemaEditor();
  for ( int i = 0; i < description.numberOfIndices(); ++i )
  {
    const coral::IIndex& index = description.index( i );
    editor.createIndex( index.name(), index.columnNames(), index.isUnique(), index.tableSpaceName() );
  }
  return table;
}


void
coral::SQLiteAccess::Schema::dropTable( const std::string& name )
{
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"Schema::dropTable "<<name<<coral::MessageStream::endmsg;
#endif
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::dropTable" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ISchema::dropTable" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::dropTable" );

  if ( ! this->existsTable( name ) ) {
    throw coral::TableNotExistingException( m_properties->domainProperties().service()->name(),name,"ISchema::dropTable" );
  }
  std::string query("DROP TABLE \"");
  query+=name;
  query+="\"";
  SQLiteStatement statement(m_properties);
  statement.prepare(query);
  if( !statement.execute() ) {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(), "Could not drop table "+name, "ISchema::dropTable" );
  }
  std::map< std::string, coral::SQLiteAccess::Table* >::iterator iTable = m_tables.find( name );

  if ( iTable->second ) delete iTable->second;
  m_tables.erase( iTable );
  statement.reset();
  if ( this->existsTable( "coral_sqlite_fk" ) )
  {
    query="DELETE FROM coral_sqlite_fk WHERE tablename=\'";
    query+=name;
    query+="\'";

    if(!statement.prepare(query) ) {
      throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not remove table "+name+" from coral_sqlite_fk","Schema::dropTable" );
    }
    statement.execute();
    statement.reset();
  }
}


void
coral::SQLiteAccess::Schema::dropIfExistsTable( const std::string& name )
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(), "ISchema::dropIfExistsTable" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ISchema::dropIfExistsTable" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::dropIfExistsTable" );
  if ( this->existsTable( name ) ) {
    this->dropTable(name);
  }
}


coral::ITable&
coral::SQLiteAccess::Schema::tableHandle( const std::string& name )
{
  /*#ifdef _DEBUG
  coral::MessageStream log( m_properties->domainProperties().service(),
                           m_properties->domainProperties().service()->name(),
                           coral::Nil );
  log<<coral::Debug<<"Schema::tableHandle "<<name<<coral::MessageStream::endmsg;
#endif
  */
  if ( ! m_properties->isTransactionActive() )
  {
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::tableHandle" );
  }
  // Check if the table is already in the local cache
  std::map< std::string, coral::SQLiteAccess::Table* >::iterator iTable = m_tables.find( name );

  if ( iTable != m_tables.end() )
  { //found in local cache
    if( !iTable->second )
    {
      iTable->second=new coral::SQLiteAccess::Table(m_properties,name);
    }
    return *( iTable->second );
  }
  // Check if the data dictionary was read
  if ( m_tablesReadFromDataDictionary )
  {
    throw coral::TableNotExistingException( m_properties->domainProperties().service()->name(), name,"ISchema::tableHandle" );
  }
  readTablesFromDataDictionary();
  iTable = m_tables.find( name );
  if ( iTable == m_tables.end() )
  {
    throw coral::TableNotExistingException( m_properties->domainProperties().service()->name(),name,"ISchema::tableHandle" );
  }
  if( iTable->second )
  {
    delete iTable->second;
  }
  coral::SQLiteAccess::Table* table = new coral::SQLiteAccess::Table( m_properties,name);
  iTable->second = table;
  return *( iTable->second );
}


void
coral::SQLiteAccess::Schema::truncateTable( const std::string& tableName )
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::truncateTable" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ISchema::truncateTable" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::truncateTable" );
  if ( ! this->existsTable( tableName ) )
    throw coral::TableNotExistingException( m_properties->domainProperties().service()->name(),tableName, "truncateTable" );
  std::string query( "DELETE FROM \"" );
  query+=tableName;
  query+="\"";
  coral::SQLiteAccess::SQLiteStatement statement( m_properties );
  statement.prepare(query);
  if ( ! statement.execute() )
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not truncate table \"" + tableName + "\"","ISchema::truncateTable" );
}


void
coral::SQLiteAccess::Schema::callProcedure( const std::string& procedureName,
                                            const coral::AttributeList& /*inputArguments*/ )
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::callProcedure" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ISchema::callProcedure" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::callProcedure" );
  throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Operation not supported, could not call prodecure \"" + procedureName + "\"","ISchema::callProcedure" );
}


coral::IQuery*
coral::SQLiteAccess::Schema::newQuery() const
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::newQuery" );
  return new coral::SQLiteAccess::Query( m_properties );
}


coral::IViewFactory*
coral::SQLiteAccess::Schema::viewFactory()
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::viewFactory" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "ISchema::viewFactory" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::viewFactory" );
  return new coral::SQLiteAccess::ViewFactory( m_properties );
}


bool
coral::SQLiteAccess::Schema::existsView( const std::string& name ) const
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::existsView" );
  // Check if the view is already in the local cache.
  if ( m_views.find( name ) == m_views.end() )
  {
    // Check if the data dictionary was read
    if ( m_viewsReadFromDataDictionary )
    {
      return false;
    }
    else
    {
      // Check inside the database.
      readViewsFromDataDictionary();
      // Was there an entry in the database?
      std::map< std::string, coral::SQLiteAccess::View* >::iterator iView = m_views.find( name );
      if ( iView == m_views.end() ) return false;
      // The view exists. Create the view object and insert it into the map
      if( !iView->second )
      {
        // Override zero entry with some content
        iView->second = new coral::SQLiteAccess::View( m_properties, name);
      }
      return true;
    }
  }
  return true;
}


void
coral::SQLiteAccess::Schema::dropView( const std::string& name )
{
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"Schema::dropView "<<name<<coral::MessageStream::endmsg;
#endif
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::dropView" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(),"ISchema::dropView" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::dropView" );
  if ( ! this->existsView( name ) )
  {
    throw coral::ViewNotExistingException( m_properties->domainProperties().service()->name(),name,"ISchema::dropView" );
  }
  std::string query("DROP VIEW \"");
  query+=name;
  query+="\"";
  SQLiteStatement statement(m_properties);
  statement.prepare(query);
  if( !statement.execute() )
  {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(), "Could not drop a view", "ISchema::dropView" );
  }
  std::map< std::string, coral::SQLiteAccess::View* >::iterator iView = m_views.find( name );
  if ( iView->second ) delete iView->second;
  m_views.erase( iView);
}


void
coral::SQLiteAccess::Schema::dropIfExistsView( const std::string& name )
{
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::dropIfExistsView" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(),"ISchema::dropIfExistsView" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "ISchema::dropIfExistsView" );
  if ( this->existsView( name ) )
  {
    this->dropView(name);
  }
}


std::set<std::string>
coral::SQLiteAccess::Schema::listViews() const
{
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"Schema::listViews "<<coral::MessageStream::endmsg;
#endif
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::listViews" );
  std::set<std::string> result;
  if ( ! m_viewsReadFromDataDictionary )
  {
    this->readViewsFromDataDictionary();
  }
  for ( std::map< std::string, coral::SQLiteAccess::View* >::const_iterator iView = m_views.begin(); iView != m_views.end(); ++iView )
  {
    result.insert( iView->first );
  }
  return result;
}


coral::IView&
coral::SQLiteAccess::Schema::viewHandle( const std::string& name )
{
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"Schema::viewHandle "<<name<<coral::MessageStream::endmsg;
#endif
  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"ISchema::viewHandle" );
  // Check if the view is already in the local cache
  std::map< std::string, coral::SQLiteAccess::View* >::iterator iView = m_views.find( name );
  if ( iView != m_views.end() )
  { //found in local cache
    if( !iView->second )
    {
      iView->second = new coral::SQLiteAccess::View(m_properties,name);
    }
    return *( iView->second );
  }
  // Check if the data dictionary was read
  readViewsFromDataDictionary();
  m_viewsReadFromDataDictionary = true;
  iView = m_views.find( name );
  if ( iView == m_views.end() )
  {
    throw coral::ViewNotExistingException( m_properties->domainProperties().service()->name(),name,"ISchema::viewHandle" );
  }
  if( iView->second )
  {
    delete iView->second;
  }
  coral::SQLiteAccess::View* view = new coral::SQLiteAccess::View( m_properties,name);
  iView->second = view;
  return *( iView->second );
}


void
coral::SQLiteAccess::Schema::reactOnEndOfTransaction()
{
  for ( std::map< std::string, coral::SQLiteAccess::Table* >::iterator iTable = m_tables.begin(); iTable != m_tables.end(); ++iTable )
    if ( iTable->second )
    {
      delete iTable->second;
    }
  for ( std::map< std::string, coral::SQLiteAccess::View* >::iterator iView = m_views.begin(); iView != m_views.end(); ++iView )
    if ( iView->second )
    {
      delete iView->second;
    }
  m_tables.clear();
  m_tablesReadFromDataDictionary = false;
  m_views.clear();
  m_viewsReadFromDataDictionary = false;
}


void
coral::SQLiteAccess::Schema::readTablesFromDataDictionary() const
{
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"Schema::readTablesFromDataDictionary"<<coral::MessageStream::endmsg;
#endif
  coral::SQLiteAccess::SQLiteStatement stmt( m_properties );
  stmt.prepare("SELECT name FROM sqlite_master WHERE type='table' UNION ALL SELECT name FROM sqlite_temp_master WHERE type='table' ORDER BY 1");
  coral::AttributeList output;
  output.extend<std::string>( "name" );
  while( stmt.fetchNext() )
  {
    if( stmt.defineOutput( output ) )
    {
      const std::string& tableName = output.begin()->data<std::string>();

      if (m_tables.find( tableName ) == m_tables.end() )
      {
        // add zero handle
        m_tables.insert( std::pair<std::string, Table*>(tableName, 0) );
      }
    }
  }
  m_tablesReadFromDataDictionary = true;
}


void
coral::SQLiteAccess::Schema::readViewsFromDataDictionary() const
{
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"Schema::readViewsFromDataDictionary"<<coral::MessageStream::endmsg;
#endif
  coral::SQLiteAccess::SQLiteStatement stmt( m_properties );
  stmt.prepare("SELECT name FROM sqlite_master WHERE type='view' UNION ALL SELECT name FROM sqlite_temp_master WHERE type='view' ORDER BY 1");
  coral::AttributeList output;
  output.extend<std::string>( "name" );
  while( stmt.fetchNext() )
  {
    if( stmt.defineOutput( output ) )
    {
      const std::string& viewName = output["name"].data<std::string>();
      if( m_views.find( viewName ) == m_views.end() )
      {
        m_views.insert( std::pair<std::string, View*>(viewName, 0) );
      }
    }
  }
  m_viewsReadFromDataDictionary = true;
}

//-----------------------------------------------------------------------------

#ifdef CORAL240SQ
coral::ISequence&
coral::SQLiteAccess::Schema::createSequence( const coral::ISequenceDescription& )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "SQLiteAccess", "Schema::createSequence" );
}


bool
coral::SQLiteAccess::Schema::existsSequence( const std::string& /*sequenceName*/ ) const
{
  // Temporary
  throw coral::Exception( "not implemented yet", "SQLiteAccess", "Schema::existsSequence" );
}


coral::ISequence&
coral::SQLiteAccess::Schema::sequenceHandle( const std::string& /*sequenceName*/ )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "SQLiteAccess", "Schema::sequenceHandle" );
}


void
coral::SQLiteAccess::Schema::dropSequence( const std::string& /*sequenceName*/ )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "SQLiteAccess", "Schema::dropSequence" );
}


void
coral::SQLiteAccess::Schema::dropIfExistsSequence( const std::string& /*sequenceName*/ )
{
  // Temporary
  throw coral::Exception( "not implemented yet", "SQLiteAccess", "Schema::dropIfExistsSequence" );
}


std::set<std::string>
coral::SQLiteAccess::Schema::listSequences() const
{
  // Temporary
  throw coral::Exception( "not implemented yet", "SQLiteAccess", "Schema::listSequences" );
}
#endif

//-----------------------------------------------------------------------------
