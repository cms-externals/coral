// $Id: QueryMgr.cpp,v 1.4.2.1 2010/12/20 09:10:10 avalassi Exp $

// Include files
#include "RelationalAccess/ISchema.h"

// Local include files
#include "QueryMgr.h"

// Namespaces
using namespace coral::CoralServer;

//---------------------------------------------------------------------------

coral::IQuery* QueryMgr::newQuery( coral::AttributeList* pDataBuffer,
                                   coral::ISessionProxy& session,
                                   const coral::QueryDefinition& qd )
{
  // Get a new query in the appropriate schema
  coral::IQuery* pQ;
  const std::string& schemaName = qd.getSchemaName();
  // No special handling if schemaName == ""
  pQ = session.schema( schemaName ).newQuery();

  // Prepare the query from its definition
  prepareQuery( *pQ, qd );

  // Define the output buffer
  if ( pDataBuffer ) pQ->defineOutput( *pDataBuffer );

  // Return the query
  return pQ;

}

//---------------------------------------------------------------------------

coral::IQuery* QueryMgr::newQuery( const std::map< std::string, std::string > outputTypes,
                                   coral::ISessionProxy& session,
                                   const coral::QueryDefinition& qd )
{
  // Get a new query in the appropriate schema
  coral::IQuery* pQ;
  const std::string& schemaName = qd.getSchemaName();
  // No special handling if schemaName == ""
  pQ = session.schema( schemaName ).newQuery();

  // Prepare the query from its definition
  prepareQuery( *pQ, qd );

  // Define the output types
  for ( std::map< std::string, std::string >::const_iterator
          outputType = outputTypes.begin(); outputType != outputTypes.end(); outputType++ )
    pQ->defineOutputType( outputType->first, outputType->second );

  // Return the query
  return pQ;

}

//---------------------------------------------------------------------------

void QueryMgr::prepareQuery( coral::IQueryDefinition& query,
                             const coral::QueryDefinition& qd )
{

  // Distinct flag
  if ( qd.getDistinct() ) query.setDistinct();

  // Output list
  const std::vector< std::pair<std::string,std::string> >& cols = qd.getOutputList();
  for ( unsigned int i=0; i<cols.size(); i++ )
    query.addToOutputList( cols[i].first, cols[i].second );

  // Table list
  const std::vector< std::pair<std::string,std::string> >& tabs = qd.getTableList();
  for ( unsigned int i=0; i<tabs.size(); i++ )
    query.addToTableList( tabs[i].first, tabs[i].second );

  // Subqueries
  const std::map< std::string, QueryDefinition >& sqds = qd.getSubQueries();
  for ( std::map< std::string, QueryDefinition >::const_iterator
          sqd = sqds.begin(); sqd != sqds.end(); sqd++ )
  {
    IQueryDefinition& sq = query.defineSubQuery( sqd->first );
    prepareQuery( sq, sqd->second );
  }

  // Condition
  query.setCondition( qd.getCondition(), qd.getConditionData() );

  // Grouping
  query.groupBy( qd.getGroupBy() );

  // Order
  const std::vector< std::string >& ords = qd.getOrderList();
  for ( unsigned int i=0; i<ords.size(); i++ )
    query.addToOrderList( ords[i] );

  // Limit returned rows
  const std::pair<int,int>* rowLimit = qd.getRowLimitAndOffset();
  if ( rowLimit )
    query.limitReturnedRows( rowLimit->first, rowLimit->second );

  // Set operation
  const std::pair<IQueryDefinition::SetOperation,QueryDefinition>* setOp = qd.getSetOperation();
  if ( setOp )
  {
    IQueryDefinition& sq = query.applySetOperation( setOp->first );
    prepareQuery( sq, setOp->second );
  }

}

//---------------------------------------------------------------------------
