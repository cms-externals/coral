// $Id: BulkOperation.cpp,v 1.9 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/MessageStream.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"

#include "BulkOperation.h"
#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "ISessionProperties.h"
#include "Statement.h"

coral::MySQLAccess::BulkOperation::BulkOperation( const coral::MySQLAccess::ISessionProperties& properties, const coral::AttributeList& inputBuffer, int cacheSize, const std::string& statement )
  : m_sessionProperties( properties )
  , m_statement( 0 )
  , m_data( const_cast<coral::AttributeList&>(inputBuffer), cacheSize )
{
  coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

  m_statement = new coral::MySQLAccess::Statement( properties, statement );

  log << coral::Debug << "Prepared for bulk operation statement \"" << statement << "\"" << std::endl << coral::MessageStream::endmsg;
}

coral::MySQLAccess::BulkOperation::~BulkOperation()
{
  this->reset();
}

void coral::MySQLAccess::BulkOperation::processNextIteration()
{
  coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

  // Copy the elements from m_inputBuffer into bulk data cache
  m_data.insert();

  log << coral::Verbose << "Adding row " << m_data.size() << " into bulk data cache " << std::endl << coral::MessageStream::endmsg;

  // If necessary flush the cache.
  if( m_data.full() )
    this->flush();
}

void coral::MySQLAccess::BulkOperation::flush()
{
  coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

  log << coral::Debug << "Executing a bulk insert (flushing the cache)" << std::endl << coral::MessageStream::endmsg;

  // Bind the data & execute one by one as MySQL does not provide yet the bulk operation API
  for( size_t idx = 0; idx < m_data.size(); ++idx )
  {
    m_statement->execute( m_data.row( idx ) );
  }

  // Clear the cache
  m_data.clear();
}

void coral::MySQLAccess::BulkOperation::reset()
{
  if( m_statement )
  {
    delete m_statement; m_statement = 0;
  }

  m_data.reset();
}
