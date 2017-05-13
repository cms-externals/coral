#include <iostream>
#include "DataEditor.h"
#include "SQLiteStatement.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "SQLiteExpressionParser.h"
#include "BulkOperation.h"
#include "BulkOperationWithQuery.h"
#include "OperationWithQuery.h"

#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/SchemaException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/MessageStream.h"

#include "CoralKernel/Service.h"

#include <sstream>
#include <string>

coral::SQLiteAccess::DataEditor::DataEditor( boost::shared_ptr<const SessionProperties> properties,
                                             const coral::ITableDescription& description ) :
  m_properties( properties ),
  m_tableDescription( description )
{

}

coral::SQLiteAccess::DataEditor::~DataEditor(){

}

void
coral::SQLiteAccess::DataEditor::rowBuffer( coral::AttributeList& buffer )
{

  int numberOfColumns = m_tableDescription.numberOfColumns();
  for ( int iColumn = 0; iColumn < numberOfColumns; ++iColumn ) {
    const coral::IColumn& column = m_tableDescription.columnDescription( iColumn );
    buffer.extend( column.name(), column.type() );
  }
}

void
coral::SQLiteAccess::DataEditor::insertRow( const coral::AttributeList& dataBuffer )
{

  std::ostringstream os;
  os << "INSERT INTO " << "\"" << m_tableDescription.name()<<"\" ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << "\""<<iAttribute->specification().name()<<"\"";
  }
  os << " ) VALUES ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    //os << "?";
    os << ':'<<iAttribute->specification().name();
  }
  os << " )";
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::insertRow "<<os.str()<<coral::MessageStream::endmsg;
#endif
  coral::SQLiteAccess::SQLiteStatement statement( m_properties );
  statement.prepare( os.str() );
  statement.bind( dataBuffer );
  try {
    statement.execute();
  }catch (const coral::Exception& er) {
    std::string errmsg(er.what());
    if(errmsg.find("is not unique")!=std::string::npos ) {
      throw coral::DuplicateEntryInUniqueKeyException(m_properties->domainProperties().service()->name() );
    }else{
      throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not insert a new row in the table","ITableDataEditor::insertRow" );
    }
  }
}


coral::IOperationWithQuery*
coral::SQLiteAccess::DataEditor::insertWithQuery()
{

  std::ostringstream os;
  os << "INSERT INTO \"" << m_tableDescription.name()<<"\"";
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::insertWithQuery "<<os.str()<<coral::MessageStream::endmsg;
#endif
  return new coral::SQLiteAccess::OperationWithQuery(os.str(),m_properties);
}


coral::IBulkOperation*
coral::SQLiteAccess::DataEditor::bulkInsert( const coral::AttributeList& dataBuffer,
                                             int rowCacheSize )
{

  std::ostringstream os;
  os << "INSERT INTO \""<< m_tableDescription.name() << "\" ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    os << "\""<<iAttribute->specification().name()<< "\"";
  }
  os << " ) VALUES ( ";
  for ( coral::AttributeList::const_iterator iAttribute = dataBuffer.begin();
        iAttribute != dataBuffer.end(); ++iAttribute ) {
    if ( iAttribute != dataBuffer.begin() ) os << ", ";
    //os << "?";
    os << ':'<<iAttribute->specification().name();
  }
  os << " )";
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::bulkInsert "<<os.str()<<coral::MessageStream::endmsg;
#endif
  return new coral::SQLiteAccess::BulkOperation( m_properties,
                                                 dataBuffer,
                                                 rowCacheSize,
                                                 os.str() );
}

coral::IBulkOperationWithQuery*
coral::SQLiteAccess::DataEditor::bulkInsertWithQuery( int dataCacheSize )
{

  std::ostringstream os;
  os << "INSERT INTO " << "\""<<m_tableDescription.name()<<"\"";
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::bulkInsertWithQuery "<<os.str()<<coral::MessageStream::endmsg;
#endif
  return new coral::SQLiteAccess::BulkOperationWithQuery( m_properties,dataCacheSize,os.str() );
}

long
coral::SQLiteAccess::DataEditor::updateRows( const std::string& setClause,
                                             const std::string& condition,
                                             const coral::AttributeList& inputData )
{

  coral::SQLiteAccess::SQLiteStatement statement( m_properties );
  coral::SQLiteAccess::SQLiteExpressionParser parser;
  parser.addToTableList( "", m_tableDescription );
  std::ostringstream os;
  os << "UPDATE \"" << m_tableDescription.name() << "\" SET "<< parser.parseExpression( setClause );
  if ( ! condition.empty() ) {
    os << " WHERE "
       << parser.parseExpression( condition );
  }
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::updateRows "<<os.str()<<coral::MessageStream::endmsg;
#endif
  statement.prepare(os.str());
  if( inputData.size() != 0 ) {
    statement.bind( inputData );
  }
  if ( ! statement.execute() )
    throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not updates rows in the table","ITableDataEditor::updateRows" );
  return statement.numberOfRowsProcessed();
}


coral::IBulkOperation*
coral::SQLiteAccess::DataEditor::bulkUpdateRows( const std::string& setClause,
                                                 const std::string& condition,
                                                 const coral::AttributeList& inputData,
                                                 int dataCacheSize )
{

  coral::SQLiteAccess::SQLiteExpressionParser parser;
  parser.addToTableList("", m_tableDescription );
  std::ostringstream os;
  os << "UPDATE \"" << m_tableDescription.name() << "\" SET " << parser.parseExpression( setClause );
  if ( ! condition.empty() ) {
    os << " WHERE "
       << parser.parseExpression( condition );
  }
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::bulkUpdateRows "<<os.str()<<coral::MessageStream::endmsg;
#endif
  return new coral::SQLiteAccess::BulkOperation( m_properties,
                                                 inputData,
                                                 dataCacheSize,
                                                 os.str() );
}


long
coral::SQLiteAccess::DataEditor::deleteRows( const std::string& condition,
                                             const coral::AttributeList& conditionData )
{

  coral::SQLiteAccess::SQLiteStatement statement( m_properties );
  std::ostringstream os;
  os << "DELETE FROM \"" << m_tableDescription.name()<<"\"";
  if ( ! condition.empty() ) {
    coral::SQLiteAccess::SQLiteExpressionParser parser;
    parser.addToTableList("", m_tableDescription );
    os << " WHERE " << parser.parseExpression( condition );
  }else{
    os << " WHERE 1 ";
  }
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::deleteRows "<<os.str()<<coral::MessageStream::endmsg;
#endif
  statement.prepare(os.str());
  if( conditionData.size() != 0 ) {
    statement.bind(conditionData);
  }
  if ( ! statement.execute() )
    throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not delete rows from the table","ITableDataEditor::updateRows" );
  return statement.numberOfRowsProcessed();
}

coral::IBulkOperation*
coral::SQLiteAccess::DataEditor::bulkDeleteRows( const std::string& condition,
                                                 const coral::AttributeList& conditionData,
                                                 int dataCacheSize )
{

  std::ostringstream os;
  os << "DELETE FROM \""<< m_tableDescription.name()<<"\"";
  if ( ! condition.empty() ) {
    coral::SQLiteAccess::SQLiteExpressionParser parser;
    parser.addToTableList("", m_tableDescription );
    os << " WHERE "
       << parser.parseExpression( condition );
  }
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"DataEditor::bulkDeleteRows "<<os.str()<<coral::MessageStream::endmsg;
#endif
  return new coral::SQLiteAccess::BulkOperation( m_properties,
                                                 conditionData,
                                                 dataCacheSize,
                                                 os.str() );
}
