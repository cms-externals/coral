// $Id: Query.cpp,v 1.10 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/MessageStream.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"

#include "Cursor.h"
#include "DomainProperties.h"
#include "ISessionProperties.h"
#include "Query.h"
#include "Statement.h"

coral::MySQLAccess::Query::Query( const coral::MySQLAccess::ISessionProperties& properties, const std::string& tableName )
  : coral::MySQLAccess::QueryDefinition( properties, tableName )
  , m_cursor( 0 )
  , m_forUpdate( false )
  , m_cacheSize( 0 )
  , m_outputBuffer( 0 )
  , m_outputTypes()
{
}

coral::MySQLAccess::Query::Query( const coral::MySQLAccess::ISessionProperties& properties ) :
  coral::MySQLAccess::QueryDefinition( properties ),
  m_cursor( 0 ),
  m_forUpdate( false ),
  m_cacheSize( 0 ),
  m_outputBuffer( 0 ),
  m_outputTypes()
{
}

coral::MySQLAccess::Query::~Query()
{
  if ( m_outputBuffer )
  {
    delete m_outputBuffer; m_outputBuffer = 0;
  }

  if ( m_cursor )
  {
    delete m_cursor; m_cursor = 0;
  }
}

void coral::MySQLAccess::Query::setForUpdate()
{
  if ( this->sessionProperties().isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( this->sessionProperties().domainProperties().service()->name(), "IQuery::setForUpdate" );
  if ( this->sessionProperties().isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( this->sessionProperties().domainProperties().service()->name(), "IQuery::setForUpdate" );

  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(), "IQuery::setForUpdate" );

  m_forUpdate = true;
}

void coral::MySQLAccess::Query::setRowCacheSize( int numberOfCachedRows )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(),
                                         "IQuery::setRowCacheSize" );
  m_cacheSize = numberOfCachedRows;
}

void coral::MySQLAccess::Query::setMemoryCacheSize( int sizeInMB )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(),
                                         "IQuery::setMemoryCacheSize" );
  m_cacheSize = -sizeInMB;
}

void coral::MySQLAccess::Query::defineOutputType( const std::string& outputIdentifier, const std::string& cppTypeName )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(),
                                         "IQuery::defineOutputTypes" );
  if ( m_outputBuffer ) {
    delete m_outputBuffer;
    m_outputBuffer = 0;
  }
  m_outputTypes.insert( std::make_pair( outputIdentifier, cppTypeName ) );
}

void coral::MySQLAccess::Query::defineOutput( coral::AttributeList& outputDataBuffer )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(),
                                         "IQuery::defineOutput" );

  if ( m_outputBuffer ) delete m_outputBuffer;
  m_outputBuffer = new coral::AttributeList( outputDataBuffer );
  const unsigned int numberOfVariables = outputDataBuffer.size();
  for ( unsigned int i = 0; i < numberOfVariables; ++i ) {
    (*m_outputBuffer)[i].shareData( outputDataBuffer[i] );
  }
}

coral::ICursor& coral::MySQLAccess::Query::execute()
{
  coral::MessageStream log( this->sessionProperties().domainProperties().service()->name() );

  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(),
                                         "IQuery::execute" );

  // Get the sql fragment from the definition
  std::string sqlStatement = this->sqlFragment();

  // Lock for update
  if ( m_forUpdate ) sqlStatement += " FOR UPDATE";

  // Prepare the statement.
  //#if(  MYSQL_VERSION_ID > 40100 )
  //  coral::MySQLAccess::ServerRevision sr = this->sessionProperties().serverRevision();
  //  if( sr.major >= 4 && sr.minor >= 1 )
  //    coral::MySQLAccess::PreparedStatement* statement = new coral::MySQLAccess::PreparedStatement( this->sessionProperties(), sqlStatement );
  //  else
  //    coral::MySQLAccess::Statement* statement = new coral::MySQLAccess::Statement( this->sessionProperties(), sqlStatement );
  //#else
  coral::MySQLAccess::Statement* statement = new coral::MySQLAccess::Statement( this->sessionProperties(), sqlStatement );
  //#endif

  // Define the cache size.
  if ( m_cacheSize < 0 ) {
    statement->setCacheSize( -m_cacheSize );
  }
  else if ( m_cacheSize > 0 ) {
    statement->setNumberOfPrefetchedRows( m_cacheSize );
  }

  // Bind and execute.
  bool result = statement->execute( this->bindData() ); // FIXME - Should be executed inside try/catch instead
  if ( ! result )
    throw coral::QueryException( this->sessionProperties().domainProperties().service()->name(), "Could not execute a query", "IQuery::execute" );

  log << coral::Debug << "Query " << sqlStatement << " executed successfully..." << coral::MessageStream::endmsg;

  const std::vector< std::string >& outputNames = this->outputVariables();

  // Define the output.
  if ( ! m_outputBuffer )
  {
    m_outputBuffer = new coral::AttributeList;
    int position = 0;

    for ( std::vector<std::string>::const_iterator iOutput = outputNames.begin(); iOutput != outputNames.end(); ++iOutput, ++position )
    {
      // Check if the type is known.
      std::map< std::string, std::string >::const_iterator iType = m_outputTypes.find( *iOutput );
      if ( iType != m_outputTypes.end() )
      {
        m_outputBuffer->extend( iType->first, iType->second );
      }
      else
      {
        const std::type_info* typeId = statement->typeForOutputColumn( position );
        if ( typeId == 0 )
          throw coral::QueryException( this->sessionProperties().domainProperties().service()->name(), "Could not identify output type for \"" + *iOutput + "\"", "IQuery::execute" );
        m_outputBuffer->extend( *iOutput,*typeId );
      }
    }
  }

  statement->defineOutput( *m_outputBuffer );

  log << coral::Debug << "Output buffer bound successfully..." << coral::MessageStream::endmsg;

  // Return the cursor
  m_cursor = new coral::MySQLAccess::Cursor( statement, *m_outputBuffer );

  return *m_cursor;
}
