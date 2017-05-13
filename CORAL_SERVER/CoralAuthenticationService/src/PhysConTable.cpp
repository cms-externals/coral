#include <memory>
#include "PhysConTable.h"

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

PhysConTable::PhysConTable(ISchema & schema)
  : m_schema( schema )
  , m_table( m_schema.tableHandle( defaultTableName() ) )
{
}

//----------------------------------------------------------------------------

PhysConTable::~PhysConTable()
{
}

//----------------------------------------------------------------------------

void PhysConTable::createTable(ISchema &schema)
{
  // create a table description
  coral::TableDescription tableDesc;
  tableDesc.setName( defaultTableName() );

  tableDesc.insertColumn( columnNames::physConStr(),
                          AttributeSpecification::typeNameForType<columnTypes::physConStr>( ),
                          255, false );
  tableDesc.setPrimaryKey( columnNames::physConStr() );

  schema.createTable( tableDesc );
}

//----------------------------------------------------------------------------

void PhysConTable::deleteTable( ISchema &schema )
{
  schema.dropIfExistsTable( defaultTableName() );
}

//----------------------------------------------------------------------------

void PhysConTable::addPhysCon( const std::string& physConStr )
{
  coral::AttributeList rowBuffer;

  m_table.dataEditor().rowBuffer( rowBuffer );
  rowBuffer[ columnNames::physConStr() ].data< columnTypes::physConStr>() =
    physConStr;

  m_table.dataEditor().insertRow( rowBuffer );
}

//----------------------------------------------------------------------------

void PhysConTable::generateWhere(
                                 AttributeList & whereData, std::string& whereString,
                                 const std::string& table,
                                 const std::string& physConStr )
{
  if ( !physConStr.empty() )
  {
    whereData.extend( "physConStr",
                      AttributeSpecification::typeNameForType<columnTypes::physConStr>() );
    whereData[ "physConStr" ].data<columnTypes::physConStr>() = physConStr;
    whereString =  table+columnNames::physConStr()+" = :physConStr";
  }
}

//----------------------------------------------------------------------------

bool PhysConTable::physConExists( const std::string& physConStr )
{

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, "", physConStr );

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

long PhysConTable::delPhysCon( const std::string& physConStr )
{

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, "", physConStr);

  return m_table.dataEditor().deleteRows( whereString, whereData );
}

//----------------------------------------------------------------------------
