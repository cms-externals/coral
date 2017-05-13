// $Id: QueryDefinition.cpp,v 1.14 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <sstream>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralCommon/ExpressionParser.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "ISessionProperties.h"
#include "QueryDefinition.h"
//#include "View.h" // VIEWS ARE NOT YET SUPPORTED IN MYSQLACCESS (bug #36512)

using coral::CoralCommon::ExpressionParser;

coral::MySQLAccess::QueryDefinition::QueryDefinition( const coral::MySQLAccess::ISessionProperties& properties ) :
  m_properties( properties ),
  m_singleTable( false ),
  m_sqlFragmnent( "" ),
  m_inputData( 0 ),
  m_distinct( false ),
  m_outputList(),
  m_tableList(),
  m_subQueries(),
  m_condition( "" ),
  m_grouping( "" ),
  m_orderList(),
  m_rowLimit( 0 ),
  m_rowOffset( 0 ),
  m_setQuery( 0, coral::IQueryDefinition::Union ),
  m_output()
{
}

coral::MySQLAccess::QueryDefinition::QueryDefinition( const coral::MySQLAccess::ISessionProperties& properties, const std::string& tableName ) :
  m_properties( properties ),
  m_singleTable( true ),
  m_sqlFragmnent( "" ),
  m_inputData( 0 ),
  m_distinct( false ),
  m_outputList(),
  m_tableList(),
  m_subQueries(),
  m_condition( "" ),
  m_grouping( "" ),
  m_orderList(),
  m_rowLimit( 0 ),
  m_rowOffset( 0 ),
  m_setQuery( 0, coral::IQueryDefinition::Union ),
  m_output()
{
  m_tableList.push_back( std::make_pair( tableName, tableName ) );
}

coral::MySQLAccess::QueryDefinition::~QueryDefinition()
{
  if ( m_setQuery.first ) delete m_setQuery.first;
  if ( m_inputData ) delete m_inputData;
}

std::string coral::MySQLAccess::QueryDefinition::sqlFragment() const
{
  if ( m_sqlFragmnent.empty() )
    const_cast< coral::MySQLAccess::QueryDefinition* >( this )->process();
  return m_sqlFragmnent;
}

const coral::AttributeList& coral::MySQLAccess::QueryDefinition::bindData() const
{
  if ( m_sqlFragmnent.empty() )
    const_cast< coral::MySQLAccess::QueryDefinition* >( this )->process();
  return *m_inputData;
}

const std::vector<std::string> coral::MySQLAccess::QueryDefinition::outputVariables() const
{
  if ( m_sqlFragmnent.empty() )
    const_cast< coral::MySQLAccess::QueryDefinition* >( this )->process();
  return m_output;
}

const coral::MySQLAccess::ISessionProperties& coral::MySQLAccess::QueryDefinition::sessionProperties() const
{
  return m_properties;
}

void coral::MySQLAccess::QueryDefinition::setDistinct()
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(), "IQueryDefinition::setDistinct" );
  m_distinct = true;
}

void coral::MySQLAccess::QueryDefinition::addToOutputList( const std::string& expression, std::string alias )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(), "IQueryDefinition::addToOutputList" );
  m_outputList.push_back( std::make_pair( expression, alias ) );
}

void coral::MySQLAccess::QueryDefinition::addToTableList( const std::string& tableName, std::string alias )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(), "IQueryDefinition::addToTableList" );
  if ( m_singleTable )
    throw coral::QueryException( m_properties.domainProperties().service()->name(), "method cannot be invoked on queries on a single table", "IQueryDefinition::addToTableList" );

  m_tableList.push_back( std::make_pair( tableName, alias.empty() ? tableName : alias ) );
}

coral::IQueryDefinition& coral::MySQLAccess::QueryDefinition::defineSubQuery( const std::string& alias )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(),
                                         "IQueryDefinition::defineSubQuery" );

  if ( m_singleTable ) {
    return m_subQueries.insert( std::make_pair( alias,
                                                coral::MySQLAccess::QueryDefinition( m_properties,
                                                                                     m_tableList.begin()->first ) ) ).first->second;
  }
  else {
    return m_subQueries.insert( std::make_pair( alias,
                                                coral::MySQLAccess::QueryDefinition( m_properties ) ) ).first->second;
  }
}

void coral::MySQLAccess::QueryDefinition::setCondition( const std::string& condition, const coral::AttributeList& inputData )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(), "IQueryDefinition::setCondition" );

  if ( m_inputData )
  {
    delete m_inputData;
    m_inputData = 0;
  }

  if ( inputData.size() > 0 )
  {
    m_inputData = new coral::AttributeList( inputData );
    const unsigned int numberOfVariables = inputData.size();

    for ( unsigned int i = 0; i < numberOfVariables; ++i )
    {
      (*m_inputData)[i].shareData( inputData[i] );
    }
  }

  m_condition = condition;
}

void coral::MySQLAccess::QueryDefinition::groupBy( const std::string& expression )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(),
                                         "IQueryDefinition::groupBy" );
  m_grouping = expression;
}


void coral::MySQLAccess::QueryDefinition::addToOrderList( const std::string& expression )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(), "IQueryDefinition::addToOrderList" );
  m_orderList.push_back( expression );
}

void coral::MySQLAccess::QueryDefinition::limitReturnedRows( int maxRows, int offset )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(), "IQueryDefinition::limitReturnedRows" );
  m_rowLimit = maxRows;
  m_rowOffset = offset;
}

coral::IQueryDefinition& coral::MySQLAccess::QueryDefinition::applySetOperation( coral::IQueryDefinition::SetOperation opetationType )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties.domainProperties().service()->name(), "IQueryDefinition::applySetOperation" );

  if ( m_setQuery.first ) delete m_setQuery.first;
  m_setQuery.second = opetationType;

  if ( m_singleTable )
    m_setQuery.first = new coral::MySQLAccess::QueryDefinition( m_properties, m_tableList.begin()->first );
  else
    m_setQuery.first = new coral::MySQLAccess::QueryDefinition( m_properties );

  return *( m_setQuery.first );
}

void coral::MySQLAccess::QueryDefinition::process()
{
  if ( ! m_inputData )
    m_inputData = new coral::AttributeList;

  // An expression parser for the subsequent calls
  ExpressionParser expressionParser;

  for ( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin(); iTable != m_tableList.end(); ++iTable )
  {
    const std::string& tableName  = iTable->first;
    const std::string& tableAlias = iTable->second;

    if( ! tableAlias.empty() )
    {
      expressionParser.addToAliases( tableAlias );
    }

    // Check if it is a subquery...
    std::map< std::string, coral::MySQLAccess::QueryDefinition >::const_iterator iTableQ = m_subQueries.find( tableName );
    if ( iTableQ != m_subQueries.end() )
    {
      expressionParser.addToTableList( iTableQ->first, iTableQ->second.outputVariables() );
    }
    else if ( m_properties.schema().existsTable( tableName ) )
      expressionParser.addToTableList( m_properties.schema().tableHandle( tableName ).description() );
    /*
    // VIEWS ARE NOT YET SUPPORTED IN MYSQLACCESS (bug #36512)
    else if ( m_properties.schema().existsView( tableName ) )
      expressionParser.addToTableList( dynamic_cast< const::coral::MySQLAccess::View& >( m_properties.schema().viewHandle( tableName ) ).description() );
    */
  }

  // Construct the query.
  std::ostringstream os;
  os << "SELECT ";

  if( this->m_distinct )
    os << "DISTINCT ";

  if ( m_outputList.empty() && m_tableList.size() == 1 ) { // wildcard query on a table or view.
    os << "*";
    const std::string& tableName = m_tableList.begin()->first;
    if ( m_singleTable || m_properties.schema().existsTable( tableName ) ) { // Check if it is a table
      const coral::ITableDescription& description = m_properties.schema().tableHandle( tableName ).description();
      int numberOfColumns = description.numberOfColumns();
      for ( int i = 0; i < numberOfColumns; ++i )
        m_output.push_back( description.columnDescription(i).name() );
    }
    /*
    // VIEWS ARE NOT YET SUPPORTED IN MYSQLACCESS (bug #36512)
    else if ( m_properties.schema().existsView( tableName ) ) { // Check if it is a query
      const coral::IView& view = m_properties.schema().viewHandle( tableName );
      int numberOfColumns = view.numberOfColumns();
      for ( int i = 0; i < numberOfColumns; ++i )
        m_output.push_back( view.column(i).name() );
    }
    */
    else { // Check if it is a sub-query
      std::map< std::string, coral::MySQLAccess::QueryDefinition >::const_iterator iTable = m_subQueries.find( tableName );
      if ( iTable != m_subQueries.end() ) {
        const std::vector< std::string >& outputColumns = iTable->second.outputVariables();
        for ( std::vector< std::string >::const_iterator iColumn = outputColumns.begin();
              iColumn != outputColumns.end(); ++iColumn ) {
          m_output.push_back( *iColumn );
        }
      }
    }
  }

  // The output variables
  for ( std::vector< std::pair< std::string, std::string > >::const_iterator iVariable = m_outputList.begin();
        iVariable != m_outputList.end(); ++iVariable ) {
    if ( iVariable != m_outputList.begin() ) os << ", ";
    m_output.push_back( iVariable->first );
    os << expressionParser.parseExpression( iVariable->first );
    if ( ! ( iVariable->second.empty() ) ) {
      os << " AS \"" << iVariable->second << "\"";
      m_output.back() = iVariable->second;
    }
  }

  // The table list.
  os << " FROM ";
  for ( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin();
        iTable != m_tableList.end(); ++iTable ) {
    if ( iTable != m_tableList.begin() ) os << ", ";

    // Check if this is a subquery
    const std::string& tableName = iTable->first;
    std::map< std::string, coral::MySQLAccess::QueryDefinition >::const_iterator iSubQuery = m_subQueries.find( tableName );
    if ( iSubQuery != m_subQueries.end() )
      os << "( " << iSubQuery->second.sqlFragment() << " ) ";
    else
      os << "\"" << m_properties.schemaName() << "\".\"" << tableName << "\" ";
    const std::string& aliasName = iTable->second;
    if ( m_subQueries.find( aliasName ) != m_subQueries.end() ||
         m_properties.schema().existsTable( aliasName ) ||
         m_properties.schema().existsView( aliasName ) )
      os << "\"" << aliasName << "\"";
    else
      os << "\"" << aliasName << "\"";
  }

  // The WHERE clause.
  if ( ! m_condition.empty() ) {
    if ( m_inputData )
      expressionParser.appendToBindVariables( *m_inputData );
    os << " WHERE " << expressionParser.parseExpression( m_condition );
  }

  // The GROUP BY clause.
  if ( ! m_grouping.empty() ) {
    os << " GROUP BY " << expressionParser.parseExpression( m_grouping );
  }

  // The ORDER BY clause.
  if ( ! m_orderList.empty() ) {
    os << " ORDER BY ";
    for ( std::vector< std::string>::const_iterator iExpression = m_orderList.begin();
          iExpression != m_orderList.end(); ++iExpression ) {
      if ( iExpression != m_orderList.begin() ) os << ", ";
      os << expressionParser.parseExpression( *iExpression );
    }
  }

  std::string sqlStatement = os.str();

  // The limiting rows.
  if ( m_rowLimit > 0 ) {
    std::ostringstream osFinalStatement;
    if ( m_rowOffset == 0 ) {
      osFinalStatement << sqlStatement << " LIMIT " << m_rowLimit;
    }
    else {
      osFinalStatement << sqlStatement << " LIMIT " << m_rowOffset << " , " << m_rowLimit;
    }
    sqlStatement = osFinalStatement.str();
  }

  // The set operation
  m_sqlFragmnent = sqlStatement;
  if ( m_setQuery.first ) {
    switch ( m_setQuery.second ) {
    case coral::IQueryDefinition::Union:
      m_sqlFragmnent += " UNION ";
      break;
    case coral::IQueryDefinition::Minus:
      m_sqlFragmnent += " MINUS ";
      break;
    case coral::IQueryDefinition::Intersect:
      m_sqlFragmnent += " INTERSECT ";
      break;
    default:
      break;
    };
    m_sqlFragmnent += m_setQuery.first->sqlFragment();
  }

  // Merge the input data
  for ( std::map< std::string, QueryDefinition >::const_iterator iSubQuery = m_subQueries.begin();
        iSubQuery != m_subQueries.end(); ++iSubQuery )
    m_inputData->merge( iSubQuery->second.bindData() );
  if (  m_setQuery.first )
    m_inputData->merge( m_setQuery.first->bindData() );

  // Now we can clean up all the underlying stuff
  m_outputList.clear();
  m_tableList.clear();
  m_subQueries.clear();
  m_orderList.clear();
  if ( m_setQuery.first ) {
    delete m_setQuery.first;
    m_setQuery.first = 0;
  }
}
