// $Id: OperationWithQuery.cpp,v 1.4 2011/03/22 10:36:50 avalassi Exp $
#include "OperationWithQuery.h"
#include "QueryDefinition.h"
#include "Statement.h"

#include <cstdlib>

coral::FrontierAccess::OperationWithQuery::OperationWithQuery( const std::string& sqlPrefix, const coral::FrontierAccess::ISessionProperties& properties )
  : m_queryDefinition( new coral::FrontierAccess::QueryDefinition( properties ) ), m_statement( sqlPrefix )
{
}

coral::FrontierAccess::OperationWithQuery::~OperationWithQuery()
{
  if ( m_queryDefinition ) delete m_queryDefinition;
}

coral::IQueryDefinition& coral::FrontierAccess::OperationWithQuery::query()
{
  return *m_queryDefinition;
}

long coral::FrontierAccess::OperationWithQuery::execute()
{
  if ( m_statement.empty() )
    return 0;

  m_statement += " ( " + m_queryDefinition->sqlFragment() + " )";

  coral::FrontierAccess::Statement statement( m_queryDefinition->sessionProperties(), m_statement );

  if ( ! statement.execute( m_queryDefinition->bindData() ) )
    return 0;

  long result = statement.numberOfRowsProcessed();
  m_statement = "";
  return result;
}
