// $Id: OperationWithQuery.cpp,v 1.5 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "OperationWithQuery.h"
#include "QueryDefinition.h"
#include "Statement.h"

coral::MySQLAccess::OperationWithQuery::OperationWithQuery( const std::string& sqlPrefix, const coral::MySQLAccess::ISessionProperties& properties )
  : m_queryDefinition( new coral::MySQLAccess::QueryDefinition( properties ) )
  , m_statement( sqlPrefix )
{
}

coral::MySQLAccess::OperationWithQuery::~OperationWithQuery()
{
  if ( m_queryDefinition ) delete m_queryDefinition;
}

coral::IQueryDefinition& coral::MySQLAccess::OperationWithQuery::query()
{
  return *m_queryDefinition;
}

long coral::MySQLAccess::OperationWithQuery::execute()
{
  if ( m_statement.empty() )
    return 0;

  m_statement += " ( " + m_queryDefinition->sqlFragment() + " )";

  coral::MySQLAccess::Statement statement( m_queryDefinition->sessionProperties(), m_statement );

  if ( ! statement.execute( m_queryDefinition->bindData() ) ) // FIXME - Should be executed inside try/catch instead
    return 0;

  long result = statement.numberOfRowsProcessed();
  m_statement = "";

  return result;
}
