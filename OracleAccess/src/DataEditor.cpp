#include <sstream>

#include "BulkOperation.h"
#include "BulkOperationWithQuery.h"
#include "DataEditor.h"
#include "DomainProperties.h"
#include "OperationWithQuery.h"
#include "OracleStatement.h"
#include "SessionProperties.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralCommon/ExpressionParser.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/SchemaException.h"

coral::OracleAccess::DataEditor::DataEditor( boost::shared_ptr<const SessionProperties> properties,
                                             const std::string& schemaName,
                                             const coral::ITableDescription& description ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
  m_tableDescription( description )
{
}


void
coral::OracleAccess::DataEditor::rowBuffer( coral::AttributeList& buffer )
{
  int numberOfColumns = m_tableDescription.numberOfColumns();
  for ( int iColumn = 0; iColumn < numberOfColumns; ++iColumn ) {
    const coral::IColumn& column = m_tableDescription.columnDescription( iColumn );
    buffer.extend( column.name(), column.type() );
  }
}


void
coral::OracleAccess::DataEditor::insertRow( const coral::AttributeList& dataBuffer )
{
  std::ostringstream os;
  os << "INSERT INTO " << m_schemaName << ".\"" << m_tableDescription.name() << "\" ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << "\"" << iAttribute->specification().name() << "\"";
  }
  os << " ) VALUES ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << ":\"" << iAttribute->specification().name() << "\"";
  }
  os << " )";

  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( dataBuffer ) )
    throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                      "Could not insert a new row in the table",
                                      "ITableDataEditor::insertRow" );
}


coral::IOperationWithQuery*
coral::OracleAccess::DataEditor::insertWithQuery()
{
  std::ostringstream os;
  os << "INSERT INTO " << m_schemaName << ".\"" << m_tableDescription.name() << "\"";
  return new coral::OracleAccess::OperationWithQuery( os.str(), m_sessionProperties, m_schemaName );
}


coral::IBulkOperation*
coral::OracleAccess::DataEditor::bulkInsert( const coral::AttributeList& dataBuffer,
                                             int rowCacheSize )
{
  std::ostringstream os;
  os << "INSERT INTO " << m_schemaName << ".\"" << m_tableDescription.name() << "\" ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << "\"" << iAttribute->specification().name() << "\"";
  }
  os << " ) VALUES ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << ":\"" << iAttribute->specification().name() << "\"";
  }
  os << " )";

  return new coral::OracleAccess::BulkOperation( m_sessionProperties,
                                                 m_schemaName,
                                                 dataBuffer,
                                                 rowCacheSize,
                                                 os.str() );
}


coral::IBulkOperationWithQuery*
coral::OracleAccess::DataEditor::bulkInsertWithQuery( int dataCacheSize )
{
  std::ostringstream os;
  os << "INSERT INTO " << m_schemaName << ".\"" << m_tableDescription.name() << "\"";
  return new coral::OracleAccess::BulkOperationWithQuery( m_sessionProperties,
                                                          m_schemaName,
                                                          dataCacheSize,
                                                          os.str() );
}


long
coral::OracleAccess::DataEditor::updateRows( const std::string& setClause,
                                             const std::string& condition,
                                             const coral::AttributeList& inputData )
{
  coral::CoralCommon::ExpressionParser parser;
  parser.addToTableList( m_tableDescription );
  parser.appendToBindVariables( inputData );
  std::ostringstream os;
  os << "UPDATE " << m_schemaName << ".\"" << m_tableDescription.name() << "\" SET "
     << parser.parseExpression( setClause );
  if ( ! condition.empty() ) {
    os << " WHERE "
       << parser.parseExpression( condition );
  }
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( inputData ) )
    throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                      "Could not updates rows in the table",
                                      "ITableDataEditor::updateRows" );
  return statement.numberOfRowsProcessed();
}


coral::IBulkOperation*
coral::OracleAccess::DataEditor::bulkUpdateRows( const std::string& setClause,
                                                 const std::string& condition,
                                                 const coral::AttributeList& inputData,
                                                 int dataCacheSize )
{
  coral::CoralCommon::ExpressionParser parser;
  parser.addToTableList( m_tableDescription );
  parser.appendToBindVariables( inputData );
  std::ostringstream os;
  os << "UPDATE " << m_schemaName << ".\"" << m_tableDescription.name() << "\" SET "
     << parser.parseExpression( setClause );
  if ( ! condition.empty() ) {
    os << " WHERE "
       << parser.parseExpression( condition );
  }

  return new coral::OracleAccess::BulkOperation( m_sessionProperties,
                                                 m_schemaName,
                                                 inputData,
                                                 dataCacheSize,
                                                 os.str() );
}


long
coral::OracleAccess::DataEditor::deleteRows( const std::string& condition,
                                             const coral::AttributeList& conditionData )
{
  std::ostringstream os;
  os << "DELETE FROM " << m_schemaName << ".\"" << m_tableDescription.name() << "\"";
  if ( ! condition.empty() ) {
    coral::CoralCommon::ExpressionParser parser;
    parser.addToTableList( m_tableDescription );
    parser.appendToBindVariables( conditionData );
    os << " WHERE "
       << parser.parseExpression( condition );
  }
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( conditionData ) )
    throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                      "Could not delete rows from the table",
                                      "ITableDataEditor::updateRows" );
  return statement.numberOfRowsProcessed();
}


coral::IBulkOperation*
coral::OracleAccess::DataEditor::bulkDeleteRows( const std::string& condition,
                                                 const coral::AttributeList& conditionData,
                                                 int dataCacheSize )
{
  std::ostringstream os;
  os << "DELETE FROM " << m_schemaName << ".\"" << m_tableDescription.name() << "\"";
  if ( ! condition.empty() ) {
    coral::CoralCommon::ExpressionParser parser;
    parser.addToTableList( m_tableDescription );
    parser.appendToBindVariables( conditionData );
    os << " WHERE "
       << parser.parseExpression( condition );
  }

  return new coral::OracleAccess::BulkOperation( m_sessionProperties,
                                                 m_schemaName,
                                                 conditionData,
                                                 dataCacheSize,
                                                 os.str() );
}
