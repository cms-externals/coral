// $Id: DataEditor.cpp,v 1.11 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <sstream>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralCommon/ExpressionParser.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/SchemaException.h"

#include "BulkOperation.h"
#include "BulkOperationWithQuery.h"
#include "DataEditor.h"
#include "DomainProperties.h"
#include "ISessionProperties.h"
#include "OperationWithQuery.h"
#include "Statement.h"

using coral::CoralCommon::ExpressionParser;

coral::MySQLAccess::DataEditor::DataEditor( const coral::MySQLAccess::ISessionProperties& properties, const coral::ITableDescription& description )
  : m_properties( properties )
  , m_tableDescription( description )
{
}

void coral::MySQLAccess::DataEditor::rowBuffer( coral::AttributeList& buffer )
{
  int numberOfColumns = m_tableDescription.numberOfColumns();
  for ( int iColumn = 0; iColumn < numberOfColumns; ++iColumn ) {
    const coral::IColumn& column = m_tableDescription.columnDescription( iColumn );
    buffer.extend( column.name(), column.type() );
  }
}

void coral::MySQLAccess::DataEditor::insertRow( const coral::AttributeList& dataBuffer )
{
  std::ostringstream os;

  os << "INSERT INTO " << m_properties.schemaName() << ".\"" << m_tableDescription.name() << "\" ( ";

  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin(); iAttribute != dataBuffer.end(); ++iAttribute )
  {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << "\"" << iAttribute->specification().name() << "\"";
  }

  os << " ) VALUES ( ";

  for ( coral::AttributeList::iterator iAttribute = dataBuffer.begin(); iAttribute != dataBuffer.end(); ++iAttribute )
  {
    if ( iAttribute != dataBuffer.begin() )
      os << ", ";
    if( iAttribute->isNull() )
    {
      os << "NULL";
    }
    else
    {
      os << ":\"" << iAttribute->specification().name() << "\"";
      iAttribute->setNull( false );
    }
  }

  os << " )";

  std::string sql = os.str();

  coral::MySQLAccess::Statement statement( m_properties, sql );

  if ( ! statement.execute( dataBuffer ) ) // FIXME - Should be executed inside try/catch instead
    throw coral::DataEditorException( m_properties.domainProperties().service()->name(), "Could not insert a new row in the table", "ITableDataEditor::insertRow" );
}

coral::IOperationWithQuery* coral::MySQLAccess::DataEditor::insertWithQuery()
{
  std::ostringstream os;
  os << "INSERT INTO " << m_properties.schemaName() << ".\"" << m_tableDescription.name() << "\"";
  return new coral::MySQLAccess::OperationWithQuery( os.str(), m_properties );
}

coral::IBulkOperation* coral::MySQLAccess::DataEditor::bulkInsert( const coral::AttributeList& dataBuffer, int rowCacheSize )
{
  std::ostringstream os;
  os << "INSERT INTO " << m_properties.schemaName() << ".\"" << m_tableDescription.name() << "\" ( ";

  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin(); iAttribute != dataBuffer.end(); ++iAttribute )
  {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << "\"" << iAttribute->specification().name() << "\"";
  }

  os << " ) VALUES ( ";

  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin(); iAttribute != dataBuffer.end(); ++iAttribute )
  {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << ":\"" << iAttribute->specification().name() << "\"";
  }

  os << " )";

  return new coral::MySQLAccess::BulkOperation( m_properties, dataBuffer, rowCacheSize, os.str() );
}

coral::IBulkOperationWithQuery* coral::MySQLAccess::DataEditor::bulkInsertWithQuery( int dataCacheSize )
{
  std::ostringstream os;
  os << "INSERT INTO " << m_properties.schemaName() << ".\"" << m_tableDescription.name() << "\"";
  return new coral::MySQLAccess::BulkOperationWithQuery( m_properties, dataCacheSize, os.str() );
}

long coral::MySQLAccess::DataEditor::updateRows( const std::string& setClause, const std::string& condition, const coral::AttributeList& inputData )
{
  ExpressionParser parser;
  parser.addToTableList( m_tableDescription );
  parser.appendToBindVariables( inputData );

  std::ostringstream os;

  os << "UPDATE " << m_properties.schemaName() << ".\"" << m_tableDescription.name() << "\" SET " << parser.parseExpression( setClause );

  if ( ! condition.empty() )
  {
    os << " WHERE " << parser.parseExpression( condition );
  }

  std::string sql = os.str();
  coral::MySQLAccess::Statement statement( m_properties, sql );

  if ( ! statement.execute( inputData ) ) // FIXME - Should be executed inside try/catch instead
    throw coral::DataEditorException( m_properties.domainProperties().service()->name(), "Could not updates rows in the table", "ITableDataEditor::updateRows" );

  return statement.numberOfRowsProcessed();
}

coral::IBulkOperation* coral::MySQLAccess::DataEditor::bulkUpdateRows( const std::string& setClause, const std::string& condition, const coral::AttributeList& inputData, int dataCacheSize )
{
  ExpressionParser parser;
  parser.addToTableList( m_tableDescription );
  parser.appendToBindVariables( inputData );

  std::ostringstream os;
  os << "UPDATE " << m_properties.schemaName() << ".\"" << m_tableDescription.name() << "\" SET " << parser.parseExpression( setClause );

  if ( ! condition.empty() )
  {
    os << " WHERE " << parser.parseExpression( condition );
  }

  return new coral::MySQLAccess::BulkOperation( m_properties, inputData, dataCacheSize, os.str() );
}

long coral::MySQLAccess::DataEditor::deleteRows( const std::string& condition, const coral::AttributeList& conditionData )
{
  std::ostringstream os;
  os << "DELETE FROM \"" << m_properties.schemaName() << "\".\"" << m_tableDescription.name() << "\" ";

  if ( ! condition.empty() )
  {
    ExpressionParser parser;
    parser.addToTableList( m_tableDescription );
    parser.appendToBindVariables( conditionData );

    os << " WHERE " << parser.parseExpression( condition );
  }

  std::string sql = os.str();
  coral::MySQLAccess::Statement statement( m_properties, sql );

  if ( ! statement.execute( conditionData ) ) // FIXME - Should be executed inside try/catch instead
    throw coral::DataEditorException( m_properties.domainProperties().service()->name(), "Could not delete rows from the table", "ITableDataEditor::updateRows" );

  return statement.numberOfRowsProcessed();
}

coral::IBulkOperation* coral::MySQLAccess::DataEditor::bulkDeleteRows( const std::string& condition, const coral::AttributeList& conditionData, int dataCacheSize )
{
  std::ostringstream os;
  os << "DELETE FROM \"" << m_properties.schemaName() << "\".\"" << m_tableDescription.name() << "\" ";

  if ( ! condition.empty() )
  {
    ExpressionParser parser;
    parser.addToTableList( m_tableDescription );
    parser.appendToBindVariables( conditionData );

    os << " WHERE " << parser.parseExpression( condition );
  }

  return new coral::MySQLAccess::BulkOperation( m_properties, conditionData, dataCacheSize, os.str() );
}
