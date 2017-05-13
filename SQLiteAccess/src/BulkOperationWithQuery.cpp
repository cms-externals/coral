#include <iostream>
#include "BulkOperationWithQuery.h"
#include "BulkOperation.h"
#include "QueryDefinition.h"

coral::SQLiteAccess::BulkOperationWithQuery::BulkOperationWithQuery( boost::shared_ptr<const SessionProperties> properties,int cacheSize, const std::string& statement ) :
  m_properties( properties ),
  m_rowsInCache( cacheSize ),
  m_sqlPrefix( statement ),
  m_queryDefinition( new coral::SQLiteAccess::QueryDefinition( m_properties ) ),
  m_bulkOperation( 0 )
{

}

coral::SQLiteAccess::BulkOperationWithQuery::~BulkOperationWithQuery()
{

  // release the bulk operation
  if ( m_bulkOperation ) delete m_bulkOperation;
  // release query definition
  if( m_queryDefinition ) delete m_queryDefinition;
}

coral::IQueryDefinition&
coral::SQLiteAccess::BulkOperationWithQuery::query()
{

  return *m_queryDefinition;
}

void
coral::SQLiteAccess::BulkOperationWithQuery::processNextIteration()
{

  if ( ! m_bulkOperation ) {
    m_bulkOperation = new coral::SQLiteAccess::BulkOperation( m_properties,
                                                              m_queryDefinition->bindData(),
                                                              m_rowsInCache,
                                                              m_sqlPrefix + " " + m_queryDefinition->sqlFragment() );
  }

  m_bulkOperation->processNextIteration();
}


void
coral::SQLiteAccess::BulkOperationWithQuery::flush()
{

  if ( ! m_bulkOperation ) {
    m_bulkOperation = new coral::SQLiteAccess::BulkOperation( m_properties,
                                                              m_queryDefinition->bindData(),
                                                              m_rowsInCache,
                                                              m_sqlPrefix + " " + m_queryDefinition->sqlFragment() );
  }
  m_bulkOperation->flush();
}
