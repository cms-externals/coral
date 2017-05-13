#include "BulkOperationWithQuery.h"
#include "BulkOperation.h"
#include "QueryDefinition.h"

coral::OracleAccess::BulkOperationWithQuery::BulkOperationWithQuery( boost::shared_ptr<const SessionProperties> properties,
                                                                     const std::string& schemaName,
                                                                     int cacheSize,
                                                                     const std::string& statement ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
  m_rowsInCache( ( ( cacheSize > 65534 ) ? 65534 : cacheSize ) ),
  m_sqlPrefix( statement ),
  m_queryDefinition( new coral::OracleAccess::QueryDefinition( m_sessionProperties, schemaName ) ),
  m_bulkOperation( 0 )
{
}


coral::OracleAccess::BulkOperationWithQuery::~BulkOperationWithQuery()
{
  if ( m_bulkOperation ) delete m_bulkOperation;
  // Delete the query definition
  delete m_queryDefinition;
}


coral::IQueryDefinition&
coral::OracleAccess::BulkOperationWithQuery::query()
{
  return *m_queryDefinition;
}


void
coral::OracleAccess::BulkOperationWithQuery::processNextIteration()
{
  if ( ! m_bulkOperation ) {
    m_bulkOperation = new coral::OracleAccess::BulkOperation( m_sessionProperties,
                                                              m_schemaName,
                                                              m_queryDefinition->bindData(),
                                                              m_rowsInCache,
                                                              m_sqlPrefix + " ( " + m_queryDefinition->sqlFragment() + " )" );
  }

  m_bulkOperation->processNextIteration();
}


void
coral::OracleAccess::BulkOperationWithQuery::flush()
{
  if ( ! m_bulkOperation ) {
    m_bulkOperation = new coral::OracleAccess::BulkOperation( m_sessionProperties,
                                                              m_schemaName,
                                                              m_queryDefinition->bindData(),
                                                              m_rowsInCache,
                                                              m_sqlPrefix + " ( " + m_queryDefinition->sqlFragment() + " )" );
  }
  m_bulkOperation->flush();
}
