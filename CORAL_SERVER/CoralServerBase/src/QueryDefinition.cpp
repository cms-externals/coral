// $Id: QueryDefinition.cpp,v 1.8.2.1 2010/12/20 09:10:10 avalassi Exp $

// Include files
#include "CoralBase/Attribute.h"
#include "CoralBase/Exception.h"
#include "CoralServerBase/QueryDefinition.h"
#include "RelationalAccess/SchemaException.h"

// Namespace
using namespace coral;

//-----------------------------------------------------------------------------

QueryDefinition::QueryDefinition( const std::string& schemaName,
                                  const std::string& tableName )
  : m_schemaName( schemaName )
  , m_singleTable( ( tableName != "" ) )
  , m_distinct( false )
  , m_outputList()
  , m_tableList()
  , m_subqueries()
  , m_condition("")
  , m_conditionData()
  , m_groupBy("")
  , m_orderList()
  , m_rowLimitAndOffset()
  , m_setOperation()
{
  if ( tableName != "" ) m_tableList.push_back( std::pair<std::string,std::string>( tableName, "" ) );
}

//-----------------------------------------------------------------------------

QueryDefinition::QueryDefinition( const QueryDefinition& rhs )
  : IQueryDefinition( rhs )
{
  m_schemaName = rhs.m_schemaName;
  m_singleTable = rhs.m_singleTable;
  m_distinct = rhs.m_distinct;
  m_outputList = rhs.m_outputList;
  m_tableList = rhs.m_tableList;
  m_subqueries = rhs.m_subqueries;
  m_condition = rhs.m_condition;
  m_conditionData = rhs.m_conditionData;
  m_groupBy = rhs.m_groupBy;
  m_orderList = rhs.m_orderList;
  if ( rhs.m_rowLimitAndOffset.get() )
    m_rowLimitAndOffset.reset( new std::pair<int,int>( *(rhs.m_rowLimitAndOffset) ) );
  if ( rhs.m_setOperation.get() )
    m_setOperation.reset( new std::pair<SetOperation,QueryDefinition>( *(rhs.m_setOperation) ) );
}

//-----------------------------------------------------------------------------

QueryDefinition& QueryDefinition::operator=( const QueryDefinition& rhs )
{
  m_schemaName = rhs.m_schemaName;
  m_singleTable = rhs.m_singleTable;
  m_distinct = rhs.m_distinct;
  m_outputList = rhs.m_outputList;
  m_tableList = rhs.m_tableList;
  m_subqueries = rhs.m_subqueries;
  m_condition = rhs.m_condition;
  m_conditionData = rhs.m_conditionData;
  m_groupBy = rhs.m_groupBy;
  m_orderList = rhs.m_orderList;
  if ( rhs.m_rowLimitAndOffset.get() )
    m_rowLimitAndOffset.reset( new std::pair<int,int>( *(rhs.m_rowLimitAndOffset) ) );
  if ( rhs.m_setOperation.get() )
    m_setOperation.reset( new std::pair<SetOperation,QueryDefinition>( *(rhs.m_setOperation) ) );
  return *this;
}

//-----------------------------------------------------------------------------

QueryDefinition::~QueryDefinition()
{
}

//-----------------------------------------------------------------------------

void QueryDefinition::setSchemaName( const std::string& schemaName )
{
  m_schemaName = schemaName;
}

//-----------------------------------------------------------------------------

void QueryDefinition::setDistinct()
{
  m_distinct = true;
}

//-----------------------------------------------------------------------------

void QueryDefinition::addToOutputList( const std::string& expression,
                                       std::string alias )
{
  m_outputList.push_back( std::pair<std::string,std::string>( expression, alias ) );
}

//-----------------------------------------------------------------------------

void QueryDefinition::addToTableList( const std::string& tableName,
                                      std::string alias )
{
  if ( m_singleTable )
    throw coral::QueryException( "coral::CoralServerBase",
                                 "Cannot add tables to queries on a single table",
                                 "QueryDefinition::addToTableList" );
  m_tableList.push_back( std::pair<std::string,std::string>( tableName, alias ) );
}

//-----------------------------------------------------------------------------

IQueryDefinition& QueryDefinition::defineSubQuery( const std::string& alias )
{
  if ( m_subqueries.find( alias ) != m_subqueries.end() )
    throw Exception( "Subquery "+alias+" already exists",
                     "QueryDefinition::defineSubQuery",
                     "coral::CoralServerBase" );
  m_subqueries[ alias ] = QueryDefinition();
  return m_subqueries[ alias ];
}

//-----------------------------------------------------------------------------

void QueryDefinition::setCondition( const std::string& condition,
                                    const AttributeList& conditionData )
{
  m_condition = condition;
  m_conditionData = conditionData;
  for ( size_t i=0; i<conditionData.size(); i++ )
    (m_conditionData[i]).shareData( conditionData[i] );
}

//-----------------------------------------------------------------------------

void QueryDefinition::groupBy( const std::string& groupBy )
{
  m_groupBy = groupBy;
}

//-----------------------------------------------------------------------------

void QueryDefinition::addToOrderList( const std::string& expression )
{
  m_orderList.push_back( expression );
}

//-----------------------------------------------------------------------------

void QueryDefinition::limitReturnedRows( int rowLimit,
                                         int rowOffset )
{
  m_rowLimitAndOffset.reset( new std::pair<int,int>( rowLimit, rowOffset ) );
}

//-----------------------------------------------------------------------------

IQueryDefinition& QueryDefinition::applySetOperation( SetOperation operationType )
{
  m_setOperation.reset( new std::pair<SetOperation,QueryDefinition>() );
  m_setOperation->first = operationType;
  return m_setOperation->second;
}

//-----------------------------------------------------------------------------
