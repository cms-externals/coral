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

  const char* reloadEnv = ::getenv( "CORAL_FRONTIER_RELOAD" ); bool reload = false;
  if( reloadEnv != 0 )
    reload = true;

  m_statement += " ( " + m_queryDefinition->sqlFragment() + " )";

  coral::FrontierAccess::Statement statement( m_queryDefinition->sessionProperties(), m_statement );

  if ( ! statement.execute( m_queryDefinition->bindData(), reload ) )
    return 0;

  long result = statement.numberOfRowsProcessed();
  m_statement = "";
  return result;
}
