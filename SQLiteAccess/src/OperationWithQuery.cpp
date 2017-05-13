#include <iostream>
#include "OperationWithQuery.h"
#include "QueryDefinition.h"
#include "SQLiteStatement.h"
#include "SQLiteExpressionParser.h"
#include "SessionProperties.h"
#include "CoralBase/AttributeList.h"
coral::SQLiteAccess::OperationWithQuery::OperationWithQuery( const std::string& sqlPrefix,
                                                             boost::shared_ptr<const SessionProperties> properties) :
  m_statement( sqlPrefix ),
  m_queryDefinition( new coral::SQLiteAccess::QueryDefinition( properties ) )
{

}


coral::SQLiteAccess::OperationWithQuery::~OperationWithQuery()
{

  if ( m_queryDefinition ) delete m_queryDefinition;
}


coral::IQueryDefinition&
coral::SQLiteAccess::OperationWithQuery::query()
{

  return *m_queryDefinition;
}


long
coral::SQLiteAccess::OperationWithQuery::execute()
{

  if ( m_statement.empty() ) return 0;
  coral::SQLiteAccess::SQLiteExpressionParser parser;
  //parser necessary?
  //m_statement += " ( " + parser.parseExpression(m_queryDefinition->sqlFragment()) + " )";
  m_statement += " "+parser.parseExpression(m_queryDefinition->sqlFragment());
  coral::SQLiteAccess::SQLiteStatement statement( m_queryDefinition->sessionProperties() );
  statement.prepare(m_statement);
  if( m_queryDefinition->bindData().size()!=0 ) {
    statement.bind( m_queryDefinition->bindData() );
  }
  if ( ! statement.execute() ) return 0;
  long result = statement.numberOfRowsProcessed();
  m_statement = "";
  return result;
}
