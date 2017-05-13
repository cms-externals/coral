#include <iostream>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListException.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"

#include "Cursor.h"
#include "DomainProperties.h"
#include "OracleStatement.h"
#include "Query.h"
#include "SessionProperties.h"


coral::OracleAccess::Query::Query( boost::shared_ptr<const SessionProperties> properties,
                                   const std::string& schemaName,
                                   const std::string& tableName ) :
  coral::OracleAccess::QueryDefinition( properties, schemaName, tableName ),
  m_cursor( 0 ),
  m_forUpdate( false ),
  m_memoryCache( 0 ),
  m_rowCache( 0 ),
  m_outputBuffer( 0 ),
  m_outputTypes()
{
}


coral::OracleAccess::Query::Query( boost::shared_ptr<const SessionProperties> properties,
                                   const std::string& schemaName ) :
  coral::OracleAccess::QueryDefinition( properties, schemaName ),
  m_cursor( 0 ),
  m_forUpdate( false ),
  m_memoryCache( 0 ),
  m_rowCache( 0 ),
  m_outputBuffer( 0 ),
  m_outputTypes()
{
}


coral::OracleAccess::Query::~Query()
{
  if ( m_outputBuffer ) delete m_outputBuffer;
  if ( m_cursor ) delete m_cursor;
}

void
coral::OracleAccess::Query::setForUpdate()
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties()->domainServiceName(), "IQuery::setForUpdate" );
  if ( this->sessionProperties()->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( this->sessionProperties()->domainServiceName(), "IQuery::setForUpdate" );
  // This is not strictly necessary (would fail with ORA-01456 anyway)
  if ( this->sessionProperties()->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( this->sessionProperties()->domainServiceName(), "IQuery::setForUpdate" );
  m_forUpdate = true;
}


void
coral::OracleAccess::Query::setRowCacheSize( int numberOfCachedRows )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties()->domainServiceName(), "IQuery::setRowCacheSize" );
  m_rowCache = numberOfCachedRows;
}


void
coral::OracleAccess::Query::setMemoryCacheSize( int sizeInMB )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties()->domainServiceName(), "IQuery::setMemoryCacheSize" );
  m_memoryCache = sizeInMB;
}


void
coral::OracleAccess::Query::defineOutputType( const std::string& outputIdentifier,
                                              const std::string& cppTypeName )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties()->domainServiceName(), "IQuery::defineOutputTypes" );
  if ( m_outputBuffer )
  {
    delete m_outputBuffer;
    m_outputBuffer = 0;
  }
  m_outputTypes.insert( std::make_pair( outputIdentifier, cppTypeName ) );
}


void
coral::OracleAccess::Query::defineOutput( coral::AttributeList& outputDataBuffer )
{
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties()->domainServiceName(), "IQuery::defineOutput" );
  if ( m_outputBuffer ) delete m_outputBuffer;
  m_outputBuffer = new coral::AttributeList( outputDataBuffer );
  const unsigned int numberOfVariables = outputDataBuffer.size();
  for ( unsigned int i = 0; i < numberOfVariables; ++i )
  {
    (*m_outputBuffer)[i].shareData( outputDataBuffer[i] );
  }
}


coral::ICursor&
coral::OracleAccess::Query::execute()
{
  //std::cout << "Query::execute starting" << std::endl; // debug bug #54968
  if ( m_cursor )
    throw coral::QueryExecutedException( this->sessionProperties()->domainServiceName(), "IQuery::execute" );
  // Get the sql fragment from the definition
  std::string sqlStatement = this->sqlFragment();
  // Lock for update
  if ( m_forUpdate ) sqlStatement += " FOR UPDATE";
  // Prepare the statement.
  std::auto_ptr<OracleStatement> statement( new coral::OracleAccess::OracleStatement( this->sessionProperties(), m_schemaName, sqlStatement ) );
  // Set the prefetch-caches for rows and memory
  //std::cout << "Query::execute calling setCacheSize" << std::endl; // debug bug #54968
  statement->setCacheSize( m_memoryCache );
  //std::cout << "Query::execute called setCacheSize" << std::endl; // debug bug #54968
  statement->setNumberOfPrefetchedRows( m_rowCache );
  // Bind and execute.
  bool result = statement->execute( this->bindData() );
  if ( ! result )
    throw coral::QueryException( this->sessionProperties()->domainServiceName(), "Could not execute a query", "IQuery::execute" );
  // Define the output.
  if ( ! m_outputBuffer )
  {
    // Create a new AttributeList
    m_outputBuffer = new coral::AttributeList;
    // Get all defined output names
    const std::vector< std::string >& outputNames = this->outputVariables();
    // Iterate through the output names to extend the new output buffer
    for ( unsigned position = 0; position < outputNames.size(); ++position )
    {
      try
      {
        // Check if the type is known.
        std::map< std::string, std::string >::const_iterator iType = m_outputTypes.find( outputNames[position] );
        if ( iType != m_outputTypes.end() )
        {
          m_outputBuffer->extend( iType->first, iType->second );
        }
        else
        {
          //std::cout << "Query::execute calling OracleStatement::typeForOutputColumn: " << outputNames[position] << ", " << position << std::endl; // debug bug #54968
          const std::type_info* typeId = statement->typeForOutputColumn( position );
          //std::cout << "Query::execute called OracleStatement::typeForOutputColumn" << std::endl; // debug bug #54968
          if ( typeId == 0 )
            throw coral::QueryException( this->sessionProperties()->domainServiceName(), "Could not identify output type for \"" + outputNames[position] + "\"", "IQuery::execute" );
          m_outputBuffer->extend( outputNames[position], *typeId );
        }
      }
      catch( coral::AttributeListException& e )
      {
        std::string expMsg( "An attribute with the name \"" + outputNames[position] + "\" exists already" );
        // Throw a QueryException if the name is already used (bug #79765)
        if ( e.what() == expMsg + " ( CORAL : \"AttributeList\" from \"CoralBase\" )" )
          throw QueryException( this->sessionProperties()->domainServiceName(), expMsg, "IQuery::execute" );
        else throw;
      }
    }
  }
  statement->defineOutput( *m_outputBuffer );
  // Return the cursor
  m_cursor = new coral::OracleAccess::Cursor( statement, *m_outputBuffer );
  return *m_cursor;
}
