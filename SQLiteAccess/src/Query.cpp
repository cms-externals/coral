#include <iostream>
#include "Query.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "Cursor.h"
#include "SQLiteStatement.h"

#include "RelationalAccess/SchemaException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/MessageStream.h"

#include "CoralKernel/Service.h"

coral::SQLiteAccess::Query::Query( boost::shared_ptr<const SessionProperties> properties, const std::string& tableName ) :
  coral::SQLiteAccess::QueryDefinition( properties, tableName ),
  m_properties(properties),
  m_cursor( 0 ),
  m_forUpdate( false ),
  m_cacheSize( 0 ),
  m_outputBuffer( 0 ),
  m_outputTypes()
{

}

coral::SQLiteAccess::Query::Query( boost::shared_ptr<const SessionProperties> properties ) :
  coral::SQLiteAccess::QueryDefinition( properties ),
  m_properties(properties),
  m_cursor( 0 ),
  m_forUpdate( false ),
  m_cacheSize( 0 ),
  m_outputBuffer( 0 ),
  m_outputTypes()
{

}

coral::SQLiteAccess::Query::~Query()
{

  if ( m_outputBuffer ) delete m_outputBuffer;
  if ( m_cursor ) delete m_cursor;
}

void
coral::SQLiteAccess::Query::setForUpdate()
{

  if ( m_cursor )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQuery::setForUpdate" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "IQuery::setForUpdate" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "IQuery::setForUpdate" );
  m_forUpdate = true;
}

void
coral::SQLiteAccess::Query::setRowCacheSize( int numberOfCachedRows )
{

  if ( m_cursor )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQuery::setRowCacheSize" );
  m_cacheSize = numberOfCachedRows;
}

void
coral::SQLiteAccess::Query::setMemoryCacheSize( int sizeInMB )
{

  if ( m_cursor )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQuery::setMemoryCacheSize" );
  m_cacheSize = -sizeInMB;
}

void
coral::SQLiteAccess::Query::defineOutputType( const std::string& outputIdentifier,
                                              const std::string& cppTypeName )
{

  if ( m_cursor )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),
                                         "IQuery::defineOutputTypes" );
  if ( m_outputBuffer ) {
    delete m_outputBuffer;
    m_outputBuffer = 0;
  }
  m_outputTypes.insert( std::make_pair( outputIdentifier, cppTypeName ) );
}


void
coral::SQLiteAccess::Query::defineOutput( coral::AttributeList& outputDataBuffer )
{

  if ( m_cursor )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQuery::defineOutput" );

  if ( m_outputBuffer ) delete m_outputBuffer;
  m_outputBuffer = new coral::AttributeList( outputDataBuffer );
  const unsigned int numberOfVariables = outputDataBuffer.size();
  for ( unsigned int i = 0; i < numberOfVariables; ++i ) {
    (*m_outputBuffer)[i].shareData( outputDataBuffer[i] );
  }
}


coral::ICursor&
coral::SQLiteAccess::Query::execute()
{

  if ( m_cursor )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(), "IQuery::execute" );

  // Get the sql fragment from the definition
  std::string sqlStatement = this->sqlFragment();

  // Lock for update
  //if ( m_forUpdate ) sqlStatement += " FOR UPDATE";
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"Query::execute "<<sqlStatement<<coral::MessageStream::endmsg;
#endif
  // Prepare the statement.
  coral::SQLiteAccess::SQLiteStatement* statement = new coral::SQLiteAccess::SQLiteStatement( this->sessionProperties() );
  if( ! statement->prepare(sqlStatement) ) {
    throw coral::QueryException( m_properties->domainProperties().service()->name(),"Could not prepare a query","IQuery::execute" );
  }

  // Define the cache size.
  if ( m_cacheSize < 0 ) {
    statement->setCacheSize( -m_cacheSize );
  }else if ( m_cacheSize > 0 ) {
    statement->setNumberOfPrefetchedRows( m_cacheSize );
  }
  if( this->bindData().size()!=0 ) {
    statement->bind( this->bindData() );
  }
  const std::vector< std::string >& outputNames = this->outputVariables();
  // Define the output.
  if ( ! m_outputBuffer ) {
    m_outputBuffer = new coral::AttributeList;
    int position = 0;
    for ( std::vector<std::string>::const_iterator iOutput = outputNames.begin(); iOutput != outputNames.end(); ++iOutput, ++position ) {
      // Check if the type is known.
      std::map< std::string, std::string >::const_iterator iType = m_outputTypes.find( *iOutput );
      if ( iType != m_outputTypes.end() ) {
        m_outputBuffer->extend( iType->first, iType->second );
      }else{
        const std::type_info* typeId = statement->decltypeForColumn( position );
        if ( typeId == 0 )
          throw coral::QueryException( m_properties->domainProperties().service()->name(), "Could not identify output type for " + *iOutput,
                                       "ICursor::Cursor" );
        m_outputBuffer->extend( *iOutput,*typeId );
      }
    }
  }

  // Return the cursor
  m_cursor = new coral::SQLiteAccess::Cursor( statement, *m_outputBuffer );
  return *m_cursor;
}
