#include <iostream>
#include <sstream>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralCommon/ExpressionParser.h"
#include "CoralCommon/SimpleExpressionParser.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "SessionProperties.h"
#include "QueryDefinition.h"
#include "View.h"

coral::OracleAccess::QueryDefinition::QueryDefinition( boost::shared_ptr<const SessionProperties> properties,
                                                       const std::string& schemaName ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
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


coral::OracleAccess::QueryDefinition::QueryDefinition( boost::shared_ptr<const SessionProperties> properties,
                                                       const std::string& schemaName,
                                                       const std::string& tableName ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
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


coral::OracleAccess::QueryDefinition::~QueryDefinition()
{
  if ( m_setQuery.first ) delete m_setQuery.first;
  if ( m_inputData ) delete m_inputData;
}


std::string
coral::OracleAccess::QueryDefinition::sqlFragment() const
{
  if ( m_sqlFragmnent.empty() )
    const_cast< coral::OracleAccess::QueryDefinition* >( this )->process();
  return m_sqlFragmnent;
}


const coral::AttributeList&
coral::OracleAccess::QueryDefinition::bindData() const
{
  if ( m_sqlFragmnent.empty() )
    const_cast< coral::OracleAccess::QueryDefinition* >( this )->process();
  return *m_inputData;
}


const std::vector<std::string>
coral::OracleAccess::QueryDefinition::outputVariables() const
{
  if ( m_sqlFragmnent.empty() )
    const_cast< coral::OracleAccess::QueryDefinition* >( this )->process();
  return m_output;
}


boost::shared_ptr<const coral::OracleAccess::SessionProperties>
coral::OracleAccess::QueryDefinition::sessionProperties() const
{
  return m_sessionProperties;
}


void
coral::OracleAccess::QueryDefinition::setDistinct()
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::setDistinct" );
  m_distinct = true;
}


void
coral::OracleAccess::QueryDefinition::addToOutputList( const std::string& expression,
                                                       std::string alias )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::addToOutputList" );
  m_outputList.push_back( std::make_pair( expression, alias ) );
}


void
coral::OracleAccess::QueryDefinition::addToTableList( const std::string& tableName,
                                                      std::string alias )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::addToTableList" );
  if ( m_singleTable )
    throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                 "method cannot be invoked on queries on a single table",
                                 "IQueryDefinition::addToTableList" );

  m_tableList.push_back( std::make_pair( tableName,
                                         alias.empty() ? tableName : alias ) );
}


coral::IQueryDefinition&
coral::OracleAccess::QueryDefinition::defineSubQuery( const std::string& alias )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::defineSubQuery" );

  if ( m_singleTable ) {
    return m_subQueries.insert( std::make_pair( alias,
                                                coral::OracleAccess::QueryDefinition( m_sessionProperties, m_schemaName, m_tableList.begin()->first ) ) ).first->second;
  }
  else {
    return m_subQueries.insert( std::make_pair( alias,
                                                coral::OracleAccess::QueryDefinition( m_sessionProperties, m_schemaName ) ) ).first->second;
  }
}


void
coral::OracleAccess::QueryDefinition::setCondition( const std::string& condition,
                                                    const coral::AttributeList& inputData )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::setCondition" );
  if ( m_inputData ) {
    delete m_inputData;
    m_inputData = 0;
  }
  if ( inputData.size() > 0 ) {
    m_inputData = new coral::AttributeList( inputData );
    const unsigned int numberOfVariables = inputData.size();
    for ( unsigned int i = 0; i < numberOfVariables; ++i ) {
      (*m_inputData)[i].shareData( inputData[i] );
    }
  }
  m_condition = condition;
}


void
coral::OracleAccess::QueryDefinition::groupBy( const std::string& expression )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::groupBy" );
  m_grouping = expression;
}


void
coral::OracleAccess::QueryDefinition::addToOrderList( const std::string& expression )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::addToOrderList" );
  m_orderList.push_back( expression );
}


void
coral::OracleAccess::QueryDefinition::limitReturnedRows( int maxRows,
                                                         int offset )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::limitReturnedRows" );
  m_rowLimit = maxRows;
  m_rowOffset = offset;
}


coral::IQueryDefinition&
coral::OracleAccess::QueryDefinition::applySetOperation( coral::IQueryDefinition::SetOperation opetationType )
{
  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_sessionProperties->domainServiceName(),
                                         "IQueryDefinition::applySetOperation" );

  if ( m_setQuery.first ) delete m_setQuery.first;
  m_setQuery.second = opetationType;

  if ( m_singleTable )
    m_setQuery.first = new coral::OracleAccess::QueryDefinition( m_sessionProperties,
                                                                 m_schemaName,
                                                                 m_tableList.begin()->first );
  else
    m_setQuery.first = new coral::OracleAccess::QueryDefinition( m_sessionProperties, m_schemaName );

  return *( m_setQuery.first );
}


void
coral::OracleAccess::QueryDefinition::process()
{
  if ( ! m_inputData ) m_inputData = new coral::AttributeList;

  coral::CoralCommon::SimpleExpressionParser sExpressionParser;
  coral::CoralCommon::ExpressionParser expressionParser;
  static bool useSimpleParser = !::getenv( "CORAL_ORA_DISABLE_SIMPLEPARSER" );
  if ( !useSimpleParser )
  {
    // An expression parser for the subsequent calls
    for ( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin();
          iTable != m_tableList.end(); ++iTable ) {

      const std::string& tableName = iTable->first;
      const std::string& tableAlias = iTable->second;

      if( ! tableAlias.empty() )
      {
        expressionParser.addToAliases( tableAlias );
      }

      // Is this a subquery?
      std::map< std::string, coral::OracleAccess::QueryDefinition >::const_iterator iTableQ = m_subQueries.find( tableName );
      if ( iTableQ != m_subQueries.end() )
      {
        expressionParser.addToTableList( iTableQ->first, iTableQ->second.outputVariables() );
      }
      // Or is this a table?
      else if ( m_sessionProperties->existsTable( m_schemaName, tableName) )
      {
        expressionParser.addToTableList( tableName, m_sessionProperties->describeTable( m_schemaName, tableName ) );
      }
      // Or is this a view?
      else if ( m_sessionProperties->existsView( m_schemaName, tableName ) )
      {
        expressionParser.addToTableList( tableName, m_sessionProperties->describeView( m_schemaName, tableName ) );
      }
    }
  }

  // Construct the query.
  std::ostringstream os;
  os << "SELECT ";
  if ( m_distinct ) os << "DISTINCT ";

  // Output list is empty ==> 'select *'
  // (wildcard query on tables, views and subqueries - with fix for bug #78574)
  if( m_outputList.empty() )
  {
    os << "*";
    // Add all columns from all tables ('select *') to the output buffer
    for ( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin();
          iTable != m_tableList.end(); ++iTable )
    {
      const std::string& tableName = iTable->first;
      // Check if this is a table
      if ( m_singleTable ||
           m_sessionProperties->existsTable( m_schemaName, tableName ) )
      {
        const std::vector< std::string > columnNames = m_sessionProperties->describeTable( m_schemaName, tableName );
        // Add all columns
        for ( unsigned int iCol = 0; iCol < columnNames.size(); ++iCol )
          m_output.push_back( columnNames[iCol] );
      }
      // Check if this is a view
      else if ( m_sessionProperties->existsView( m_schemaName, tableName ) )
      {
        const std::vector< std::string > columnNames = m_sessionProperties->describeView( m_schemaName, tableName );
        // Add all columns
        for ( unsigned int iCol = 0; iCol < columnNames.size(); ++iCol )
          m_output.push_back( columnNames[iCol] );
      }
      // Check if this is a sub-query
      else
      {
        std::map< std::string, coral::OracleAccess::QueryDefinition >::const_iterator iTableSQ = m_subQueries.find( tableName );
        if ( iTableSQ != m_subQueries.end() )
        {
          const std::vector< std::string >& outputColumns = iTableSQ->second.outputVariables();
          for ( std::vector< std::string >::const_iterator iCol = outputColumns.begin(); iCol != outputColumns.end(); ++iCol )
          {
            m_output.push_back( *iCol );
          }
        }
      }
    }
  }

  // Output list is not empty ==> 'select x, y, ...' (all variables in list)
  else
  {
    for ( std::vector< std::pair< std::string, std::string > >::const_iterator iVariable = m_outputList.begin();
          iVariable != m_outputList.end(); ++iVariable )
    {
      if ( iVariable != m_outputList.begin() ) os << ", ";
      if ( useSimpleParser )
        os << sExpressionParser.parseExpression( iVariable->first );
      else
        os << expressionParser.parseExpression( iVariable->first );
      //std::cout <<  "Parsed " << iVariable->first << ": " << expressionParser.parseExpression( iVariable->first ) << std::endl; // debug bug #73530
      if ( iVariable->second.empty() )
      {
        m_output.push_back( iVariable->first );
      }
      else
      {
        os << " AS \"" << iVariable->second << "\"";
        m_output.push_back( iVariable->second );
      }
    }
  }

  // The table list.
  os << " FROM ";
  for ( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin();
        iTable != m_tableList.end(); ++iTable ) {
    if ( iTable != m_tableList.begin() ) os << ", ";

    // Check if this is a subquery
    const std::string& tableName = iTable->first;
    std::map< std::string, coral::OracleAccess::QueryDefinition >::const_iterator iSubQuery = m_subQueries.find( tableName );
    bool isDual = false;
    if ( iSubQuery != m_subQueries.end() )
      os << "( " << iSubQuery->second.sqlFragment() << " ) ";
    else if ( tableName == "DUAL" || tableName == "dual" || tableName == "Dual" ) {
      isDual = true;
      os << "DUAL ";
    }
    else
      os << m_schemaName << ".\"" << tableName << "\" ";
    const std::string& aliasName = iTable->second;
    if ( m_subQueries.find( aliasName ) != m_subQueries.end() ||
         m_sessionProperties->existsTable( m_schemaName, aliasName ) ||
         m_sessionProperties->existsView( m_schemaName, aliasName ) )
      os << "\"" << aliasName << "\"";
    else
      if ( ! isDual )
        os << "\"" << aliasName << "\"";
  }

  // The WHERE clause.
  if ( ! m_condition.empty() ) 
  {
    if ( m_inputData )
    {
      if ( useSimpleParser )
        os << " WHERE " << sExpressionParser.parseExpression( m_condition );
      else
      {
        expressionParser.appendToBindVariables( *m_inputData );
        os << " WHERE " << expressionParser.parseExpression( m_condition );
      }
    }
  }

  // The GROUP BY clause.
  if ( ! m_grouping.empty() ) {
    if ( useSimpleParser )
      os << " GROUP BY " << sExpressionParser.parseExpression( m_grouping );
    else
      os << " GROUP BY " << expressionParser.parseExpression( m_grouping );
  }

  // The ORDER BY clause.
  if ( ! m_orderList.empty() ) {
    os << " ORDER BY ";
    for ( std::vector< std::string>::const_iterator iExpression = m_orderList.begin();
          iExpression != m_orderList.end(); ++iExpression ) {
      if ( iExpression != m_orderList.begin() ) os << ", ";
      if ( useSimpleParser )
        os << sExpressionParser.parseExpression( *iExpression );
      else
        os << expressionParser.parseExpression( *iExpression );
    }
  }

  std::string sqlStatement = os.str();

  // The limiting rows.
  if ( m_rowLimit > 0 )
  {
    std::ostringstream osFinalStatement;
    if ( m_rowOffset == 0 ) {
      osFinalStatement << "SELECT * FROM (" << sqlStatement << ") WHERE ROWNUM < " << m_rowLimit + m_rowOffset + 1;
    }
    else
    {
      osFinalStatement << "SELECT";
      for ( std::vector<std::string>::const_iterator iOutput = m_output.begin();
            iOutput != m_output.end(); ++iOutput ) {
        if ( iOutput == m_output.begin() )
          osFinalStatement << " \"" << *iOutput << "\"";
        else
          osFinalStatement << ", \"" << *iOutput << "\"";
      }
      osFinalStatement << " FROM (SELECT ROWNUM ROW#";
      for ( std::vector<std::string>::const_iterator iOutput = m_output.begin();
            iOutput != m_output.end(); ++iOutput )
      {
        osFinalStatement << ", \"" << *iOutput << "\"";
      }
      osFinalStatement << " FROM (" << sqlStatement << ") ) WHERE ROW# BETWEEN "
                       << 1 + m_rowOffset << " AND " << m_rowLimit + m_rowOffset;
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
