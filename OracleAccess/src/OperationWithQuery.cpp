#include "OperationWithQuery.h"
#include "OracleStatement.h"
#include "QueryDefinition.h"
#include "SessionProperties.h"

coral::OracleAccess::OperationWithQuery::OperationWithQuery( const std::string& sqlPrefix,
                                                             boost::shared_ptr<const SessionProperties> properties,
                                                             const std::string& schemaName ) :
  m_schemaName( schemaName ),
  m_queryDefinition( new coral::OracleAccess::QueryDefinition( properties, schemaName ) ),
  m_statement( sqlPrefix )
{
}


coral::OracleAccess::OperationWithQuery::~OperationWithQuery()
{
  if ( m_queryDefinition ) delete m_queryDefinition;
}


coral::IQueryDefinition&
coral::OracleAccess::OperationWithQuery::query()
{
  return *m_queryDefinition;
}


long
coral::OracleAccess::OperationWithQuery::execute()
{
  if ( m_statement.empty() ) return 0;

  m_statement += " ( " + m_queryDefinition->sqlFragment() + " )";

  coral::OracleAccess::OracleStatement statement( m_queryDefinition->sessionProperties(),
                                                  m_schemaName,
                                                  m_statement );

  if ( ! statement.execute( m_queryDefinition->bindData() ) ) return 0;
  long result = statement.numberOfRowsProcessed();
  m_statement = "";
  return result;
}
