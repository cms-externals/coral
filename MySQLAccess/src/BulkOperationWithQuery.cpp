// $Id: BulkOperationWithQuery.cpp,v 1.5 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "BulkOperation.h"
#include "BulkOperationWithQuery.h"
#include "QueryDefinition.h"

coral::MySQLAccess::BulkOperationWithQuery::BulkOperationWithQuery( const coral::MySQLAccess::ISessionProperties& properties, int cacheSize, const std::string& statement )
  : m_properties( properties )
  , m_rowsInCache( cacheSize )
  , m_sqlPrefix( statement )
  , m_queryDefinition( new coral::MySQLAccess::QueryDefinition( m_properties ) )
  , m_bulkOperation( 0 )
{
}

coral::MySQLAccess::BulkOperationWithQuery::~BulkOperationWithQuery()
{
  if ( m_bulkOperation )
    delete m_bulkOperation;
  // Fix for memeory leak, found by code coverity
  delete m_queryDefinition;
}

coral::IQueryDefinition& coral::MySQLAccess::BulkOperationWithQuery::query()
{
  return *m_queryDefinition;
}

void coral::MySQLAccess::BulkOperationWithQuery::processNextIteration()
{
  if ( ! m_bulkOperation )
  {
    m_bulkOperation = new coral::MySQLAccess::BulkOperation( m_properties, m_queryDefinition->bindData(), m_rowsInCache, m_sqlPrefix + " ( " + m_queryDefinition->sqlFragment() + " )" );
  }

  m_bulkOperation->processNextIteration();
}

void coral::MySQLAccess::BulkOperationWithQuery::flush()
{
  if ( ! m_bulkOperation )
  {
    m_bulkOperation = new coral::MySQLAccess::BulkOperation( m_properties, m_queryDefinition->bindData(), m_rowsInCache, m_sqlPrefix + " ( " + m_queryDefinition->sqlFragment() + " )" );
  }

  m_bulkOperation->flush();
}
