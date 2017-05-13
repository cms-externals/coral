#include <memory>
#include "LogConTable.h"

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"

using namespace coral;

//---------------------------------------------------------------------------

LogConTable::LogConTable(ISchema & schema)
  : m_schema( schema )
  , m_table( m_schema.tableHandle( defaultTableName() ) )
{
}

//----------------------------------------------------------------------------

LogConTable::~LogConTable()
{
}

//----------------------------------------------------------------------------

void LogConTable::createTable(ISchema &schema)
{
  // create a table description
  coral::TableDescription tableDesc;
  tableDesc.setName( defaultTableName() );

  tableDesc.insertColumn( columnNames::logConStr(),
                          AttributeSpecification::typeNameForType<columnTypes::logConStr>( ),
                          255, false );
  tableDesc.setPrimaryKey( columnNames::logConStr() );

  schema.createTable( tableDesc );
}

//----------------------------------------------------------------------------

void LogConTable::deleteTable( ISchema &schema )
{
  schema.dropIfExistsTable( defaultTableName() );
}

//----------------------------------------------------------------------------

void LogConTable::addLogCon( const std::string& logConStr )
{
  coral::AttributeList rowBuffer;

  m_table.dataEditor().rowBuffer( rowBuffer );
  rowBuffer[ columnNames::logConStr() ].data< columnTypes::logConStr>() =
    logConStr;

  m_table.dataEditor().insertRow( rowBuffer );
}

//----------------------------------------------------------------------------

void LogConTable::generateWhere(
                                AttributeList & whereData, std::string& whereString,
                                const std::string& table,
                                const std::string& logConStr )
{
  if ( !logConStr.empty() )
  {
    whereData.extend( "logConStr",
                      AttributeSpecification::typeNameForType<columnTypes::logConStr>() );
    whereData[ "logConStr" ].data<columnTypes::logConStr>() = logConStr;
    whereString =  table+columnNames::logConStr()+" = :logConStr";
  }
}

//----------------------------------------------------------------------------

bool LogConTable::logConExists( const std::string& logConStr )
{

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, "", logConStr );

  coral::AttributeList rowBuffer;
  m_table.dataEditor().rowBuffer( rowBuffer );

  std::auto_ptr<IQuery> query( m_schema.newQuery() );

  query->defineOutput( rowBuffer );
  query->addToTableList( defaultTableName(), "" );

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();
  return cursor.next();
}


//----------------------------------------------------------------------------

long LogConTable::delLogCon( const std::string& logConStr )
{

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, "", logConStr );

  return m_table.dataEditor().deleteRows( whereString, whereData );
}

//----------------------------------------------------------------------------
