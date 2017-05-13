#include <iostream>
#include <sstream>
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/MessageStream.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "QueryDefinition.h"
#include "SessionProperties.h"
#include "SQLiteExpressionParser.h"
#include "View.h"

coral::SQLiteAccess::QueryDefinition::QueryDefinition( boost::shared_ptr<const SessionProperties> properties) :
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

coral::SQLiteAccess::QueryDefinition::QueryDefinition( boost::shared_ptr<const SessionProperties> properties,
                                                       const std::string& tableName ) :
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


coral::SQLiteAccess::QueryDefinition::~QueryDefinition()
{

  if ( m_setQuery.first ) delete m_setQuery.first;
  if ( m_inputData ) delete m_inputData;
}


std::string
coral::SQLiteAccess::QueryDefinition::sqlFragment() const
{

  if ( m_sqlFragmnent.empty() )
    const_cast< coral::SQLiteAccess::QueryDefinition* >( this )->process();
  return m_sqlFragmnent;
}


const coral::AttributeList&
coral::SQLiteAccess::QueryDefinition::bindData() const
{

  if ( m_sqlFragmnent.empty() )
    const_cast< coral::SQLiteAccess::QueryDefinition* >( this )->process();
  return *m_inputData;
}


const std::vector<std::string>
coral::SQLiteAccess::QueryDefinition::outputVariables() const
{

  if ( m_sqlFragmnent.empty() )
    const_cast< coral::SQLiteAccess::QueryDefinition* >( this )->process();
  return m_output;
}

boost::shared_ptr<const coral::SQLiteAccess::SessionProperties>
coral::SQLiteAccess::QueryDefinition::sessionProperties() const
{

  return m_properties;
}

void
coral::SQLiteAccess::QueryDefinition::setDistinct()
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQueryDefinition::setDistinct" );
  m_distinct = true;
}


void
coral::SQLiteAccess::QueryDefinition::addToOutputList( const std::string& expression,std::string alias )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQueryDefinition::addToOutputList" );
  m_outputList.push_back( std::make_pair( expression, alias ) );
}


void
coral::SQLiteAccess::QueryDefinition::addToTableList( const std::string& tableName, std::string alias )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQueryDefinition::addToTableList" );
  if ( m_singleTable )
    throw coral::QueryException( m_properties->domainProperties().service()->name(),
                                 "method cannot be invoked on queries on a single table",
                                 "IQueryDefinition::addToTableList" );

  m_tableList.push_back( std::make_pair( tableName,
                                         alias.empty() ? tableName : alias ) );
}


coral::IQueryDefinition&
coral::SQLiteAccess::QueryDefinition::defineSubQuery( const std::string& alias )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),
                                         "IQueryDefinition::defineSubQuery" );

  if ( m_singleTable ) {
    return m_subQueries.insert( std::make_pair( alias,
                                                coral::SQLiteAccess::QueryDefinition( m_properties,
                                                                                      m_tableList.begin()->first ) ) ).first->second;
  }
  else {
    return m_subQueries.insert( std::make_pair( alias,
                                                coral::SQLiteAccess::QueryDefinition( m_properties ) ) ).first->second;
  }
}


void
coral::SQLiteAccess::QueryDefinition::setCondition( const std::string& condition,
                                                    const coral::AttributeList& inputData )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQueryDefinition::setCondition" );
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
coral::SQLiteAccess::QueryDefinition::groupBy( const std::string& expression )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),
                                         "IQueryDefinition::groupBy" );
  m_grouping = expression;
}


void
coral::SQLiteAccess::QueryDefinition::addToOrderList( const std::string& expression )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),"IQueryDefinition::addToOrderList" );
  m_orderList.push_back( expression );
}


void
coral::SQLiteAccess::QueryDefinition::limitReturnedRows( int maxRows,
                                                         int offset )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),
                                         "IQueryDefinition::limitReturnedRows" );
  m_rowLimit = maxRows;
  m_rowOffset = offset;
}


coral::IQueryDefinition&
coral::SQLiteAccess::QueryDefinition::applySetOperation( coral::IQueryDefinition::SetOperation opetationType )
{

  if ( ! m_sqlFragmnent.empty() )
    throw coral::QueryExecutedException( m_properties->domainProperties().service()->name(),
                                         "IQueryDefinition::applySetOperation" );

  if ( m_setQuery.first ) delete m_setQuery.first;
  m_setQuery.second = opetationType;

  if ( m_singleTable )
    m_setQuery.first = new coral::SQLiteAccess::QueryDefinition( m_properties,
                                                                 m_tableList.begin()->first );
  else
    m_setQuery.first = new coral::SQLiteAccess::QueryDefinition( m_properties );

  return *( m_setQuery.first );
}


void
coral::SQLiteAccess::QueryDefinition::process()
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"QueryDefinition::process"<<coral::MessageStream::endmsg;
#endif
  if ( ! m_inputData ) m_inputData = new coral::AttributeList;
  // An expression parser for the subsequent calls
  coral::SQLiteAccess::SQLiteExpressionParser expressionParser;
  for ( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin(); iTable != m_tableList.end(); ++iTable ) {
    const std::string& tableName = iTable->first;
    const std::string& alias = iTable->second;
    // Check if it is a subquery...
    std::map< std::string, coral::SQLiteAccess::QueryDefinition >::const_iterator iTableQ = m_subQueries.find( tableName );
    if ( iTableQ != m_subQueries.end() ) {
      expressionParser.addToTableList( alias, tableName, iTableQ->second.outputVariables() );
    }else if ( m_properties->schema().existsTable( tableName ) ) {
      expressionParser.addToTableList( alias, m_properties->schema().tableHandle( tableName ).description() );
    }else if ( m_properties->schema().existsView( tableName ) ) {
      expressionParser.addToTableList( alias, dynamic_cast< const ::coral::SQLiteAccess::View& >( m_properties->schema().viewHandle( tableName ) ).description() );
    }
  }

  // Construct the query.
  std::ostringstream os;
  os << "SELECT ";
  if ( m_distinct ) os << "DISTINCT ";

  // Wildcard query on tables or views or subqueries ('select *')
  //if ( m_outputList.empty() && m_tableList.size() == 1 ) // fix bug #78574
  if ( m_outputList.empty() )
  {
    os << "*";
    // Iterate through all tables from the addToTable method
    for( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin();
         iTable != m_tableList.end(); ++iTable )
    {
      const std::string& tableName = iTable->first;
      ISchema& schema = m_properties->schema();
      // Check if this is a table
      if ( m_singleTable || schema.existsTable( tableName ) )
      {
        const coral::ITableDescription& description = schema.tableHandle( tableName ).description();
        int numberOfColumns = description.numberOfColumns();
        for ( int iCol = 0; iCol < numberOfColumns; ++iCol )
          m_output.push_back( description.columnDescription(iCol).name() );
      }
      // Check if this is a view
      else if ( schema.existsView( tableName ) )
      {
        const coral::IView& view = schema.viewHandle( tableName );
        int numberOfColumns = view.numberOfColumns();
        for ( int iCol = 0; iCol < numberOfColumns; ++iCol )
          m_output.push_back( view.column(iCol).name() );
      }
      // Check if this is a sub-query
      else
      {
        std::map< std::string, coral::SQLiteAccess::QueryDefinition >::const_iterator iTableSQ = m_subQueries.find( tableName );
        if ( iTableSQ != m_subQueries.end() ) {
          const std::vector< std::string >& outputColumns = iTableSQ->second.outputVariables();
          for ( std::vector< std::string >::const_iterator iCol = outputColumns.begin(); iCol != outputColumns.end(); ++iCol ) {
            m_output.push_back( *iCol );
          }
        }
      }
    }
  }

  // The output variables
  for ( std::vector< std::pair< std::string, std::string > >::const_iterator iVariable = m_outputList.begin(); iVariable != m_outputList.end(); ++iVariable )
  {
    if ( iVariable != m_outputList.begin() ) os << ", ";
    os << expressionParser.parseExpression( iVariable->first );
    if ( iVariable->second.empty() )
    {
      m_output.push_back( iVariable->first );
    }
    else
    {
      os << " AS \"" << iVariable->second <<"\"";
      m_output.push_back( iVariable->second );
    }
  }

  // The table list.
  os << " FROM ";
  for ( std::vector< std::pair< std::string, std::string > >::const_iterator iTable = m_tableList.begin(); iTable != m_tableList.end(); ++iTable ) {
    if ( iTable != m_tableList.begin() ) os << ", ";
    // Check if this is a subquery
    const std::string& tableName = iTable->first;
    const std::string& aliasName = iTable->second;
    std::map< std::string, coral::SQLiteAccess::QueryDefinition >::const_iterator iSubQuery = m_subQueries.find( tableName );
    if ( iSubQuery != m_subQueries.end() ) {
      os << "( " << iSubQuery->second.sqlFragment() << " ) ";
      if ( m_subQueries.find( aliasName ) != m_subQueries.end() ) {
        os << "\""<<aliasName<<"\"";
      }
    }else{
      //os << m_properties->schemaName() << "." << "\""<<tableName<<"\"";
      os <<"\""<<tableName<<"\"";
      if(tableName != aliasName) {
        os << " AS \""<<aliasName<<"\"";
      }
    }
  }

  // The WHERE clause.
  if ( ! m_condition.empty() ) {
    os << " WHERE " << expressionParser.parseExpression( m_condition );
  }

  // The GROUP BY clause.
  if ( ! m_grouping.empty() ) {
    os << " GROUP BY " << expressionParser.parseExpression( m_grouping );
  }

  // The ORDER BY clause.
  if ( ! m_orderList.empty() ) {
    os << " ORDER BY ";
    for ( std::vector< std::string>::const_iterator iExpression = m_orderList.begin(); iExpression != m_orderList.end(); ++iExpression ) {
      if ( iExpression != m_orderList.begin() ) os << ", ";
      os << expressionParser.parseExpression( *iExpression );
    }
  }

#ifdef _DEBUG
  std::string sqlStatement = os.str();
  log<<coral::Debug<<"sqlStatement "<<sqlStatement<<coral::MessageStream::endmsg;
#endif
  // The limiting rows.
  if ( m_rowLimit > 0 ) {
    os << " LIMIT "<<m_rowLimit;
    if( m_rowOffset > 0 ) {
      os<< " OFFSET "<<m_rowOffset;
    }
  }

  m_sqlFragmnent = os.str();
  // The set operation
  if ( m_setQuery.first ) {
    switch ( m_setQuery.second ) {
    case coral::IQueryDefinition::Union:
      m_sqlFragmnent += " UNION ALL ";
      break;
    case coral::IQueryDefinition::Minus:
      m_sqlFragmnent += " EXCEPT ";
      break;
    case coral::IQueryDefinition::Intersect:
      m_sqlFragmnent += " INTERSECT ";
      break;
    default:
      break;
    };
    m_sqlFragmnent += m_setQuery.first->sqlFragment();
    m_sqlFragmnent="SELECT DISTINCT * FROM ("+m_sqlFragmnent+")";
  }

  // Merge the input data
  for ( std::map< std::string, QueryDefinition >::const_iterator iSubQuery = m_subQueries.begin(); iSubQuery != m_subQueries.end(); ++iSubQuery )
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
