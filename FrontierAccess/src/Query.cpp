#include "Query.h"
#include "ISessionProperties.h"
#include "DomainProperties.h"
#include "Cursor.h"
#include "Statement.h"

// #include "RelationalAccess/ISchema.h"
// #include "RelationalAccess/ITable.h"
// #include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/SchemaException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include "CoralKernel/Service.h"

coral::FrontierAccess::Query::Query( const coral::FrontierAccess::ISessionProperties& properties, const std::string& tableName )
  : coral::FrontierAccess::QueryDefinition( properties, tableName ),
    m_cursor( 0 ),
    m_cacheSize( 0 ),
    m_outputBuffer( 0 ),
    m_outputTypes()
{
}

coral::FrontierAccess::Query::Query( const coral::FrontierAccess::ISessionProperties& properties )
  : coral::FrontierAccess::QueryDefinition( properties ),
    m_cursor( 0 ),
    m_cacheSize( 0 ),
    m_outputBuffer( 0 ),
    m_outputTypes()
{
}

coral::FrontierAccess::Query::~Query()
{
  if ( m_outputBuffer ) delete m_outputBuffer;
  if ( m_cursor ) delete m_cursor;
}

void coral::FrontierAccess::Query::setForUpdate()
{
  throw coral::InvalidOperationInReadOnlyModeException( m_properties.domainProperties().service()->name(), "FrontierAccess::Query::setForUpdate" );
}

void coral::FrontierAccess::Query::setRowCacheSize( int numberOfCachedRows )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(), "IQuery::setRowCacheSize" );
  m_cacheSize = numberOfCachedRows;
}

void coral::FrontierAccess::Query::setMemoryCacheSize( int sizeInMB )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(), "IQuery::setMemoryCacheSize" );
  m_cacheSize = -sizeInMB;
}

void coral::FrontierAccess::Query::defineOutputType( const std::string& outputIdentifier, const std::string& cppTypeName )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(), "IQuery::defineOutputTypes" );
  if ( m_outputBuffer ) {
    delete m_outputBuffer;
    m_outputBuffer = 0;
  }
  m_outputTypes.insert( std::make_pair( outputIdentifier, cppTypeName ) );
}

void coral::FrontierAccess::Query::defineOutput( coral::AttributeList& outputDataBuffer )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(), "IQuery::defineOutput" );

  if ( m_outputBuffer )
    delete m_outputBuffer;

  m_outputBuffer = new coral::AttributeList( outputDataBuffer );
  const unsigned int numberOfVariables = outputDataBuffer.size();

  for ( unsigned int i = 0; i < numberOfVariables; ++i )
  {
    (*m_outputBuffer)[i].shareData( outputDataBuffer[i] );
  }
}

coral::ICursor& coral::FrontierAccess::Query::execute()
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties().domainProperties().service()->name(), "IQuery::execute" );

  // Get the sql fragment from the definition
  std::string sqlStatement = this->sqlFragment();

  // Prepare the statement.
  coral::FrontierAccess::Statement* statement = new coral::FrontierAccess::Statement( this->sessionProperties(), sqlStatement );

  // Define the cache size.
  if ( m_cacheSize < 0 ) {
    statement->setCacheSize( -m_cacheSize );
  }
  else if ( m_cacheSize > 0 ) {
    statement->setNumberOfPrefetchedRows( m_cacheSize );
  }

  bool result = statement->execute( this->bindData(), this->timeToLive() );

  if ( ! result )
    throw coral::QueryException( this->sessionProperties().domainProperties().service()->name(), "Could not execute a query", "IQuery::execute" );

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

  // Return the cursor
  m_cursor = new coral::FrontierAccess::Cursor( statement, *m_outputBuffer );
  return *m_cursor;
}
