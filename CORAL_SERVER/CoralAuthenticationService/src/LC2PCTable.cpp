#include <iostream>
#include "LC2PCTable.h"

#include "LogConTable.h"
#include "PhysConTable.h"

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"


using namespace coral;

//---------------------------------------------------------------------------

LC2PCTable::LC2PCTable(ISchema & schema)
  : m_schema( schema )
  , m_table( m_schema.tableHandle( defaultTableName() ) )
{
}

//----------------------------------------------------------------------------

LC2PCTable::~LC2PCTable()
{
}

//----------------------------------------------------------------------------

void LC2PCTable::createTable(ISchema &schema)
{
  // create a table description
  coral::TableDescription tableDesc;
  tableDesc.setName( defaultTableName() );

  tableDesc.insertColumn( columnNames::logConStr(),
                          AttributeSpecification::typeNameForType<columnTypes::logConStr>( ),
                          255, false );
  tableDesc.createForeignKey( columnNames::logConStr() + "_FK",
                              columnNames::logConStr(),
                              LogConTable::defaultTableName(), LogConTable::columnNames::logConStr() );

  tableDesc.insertColumn( columnNames::physConStr(),
                          AttributeSpecification::typeNameForType<columnTypes::physConStr>( ),
                          255, false );
  tableDesc.createForeignKey( columnNames::physConStr() + "_FK",
                              columnNames::physConStr(),
                              PhysConTable::defaultTableName(),
                              PhysConTable::columnNames::physConStr() );

  tableDesc.insertColumn( columnNames::order(),
                          AttributeSpecification::typeNameForType<columnTypes::order>( ));

  tableDesc.insertColumn( columnNames::accessMode(),
                          AttributeSpecification::typeNameForType<columnTypes::accessMode>( ),
                          25, false );

  tableDesc.insertColumn( columnNames::authenticationMechanism(),
                          AttributeSpecification::typeNameForType<columnTypes::authenticationMechanism>( ),
                          25, false );

  // add primary key logConStr, order
  std::vector< std::string > primaryKey;
  primaryKey.push_back( columnNames::logConStr() );
  primaryKey.push_back( columnNames::order() );
  tableDesc.setPrimaryKey( primaryKey );

  schema.createTable( tableDesc );
}

//----------------------------------------------------------------------------

void LC2PCTable::deleteTable( ISchema &schema )
{
  schema.dropIfExistsTable( defaultTableName() );
}

//----------------------------------------------------------------------------

static const std::string RDONLY="RDONLY";
static const std::string UPDATE="UPDATE";

const std::string& LC2PCTable::AccessMode2Str( AccessMode mode ) const
{
  if (mode==Update)
    return UPDATE;
  else if (mode==ReadOnly)
    return RDONLY;
  else throw Exception("AccessMode is neither Update or Read-Only!", "LC2PCTable::AccessMode2Str",
                       "CoralAuthenticationService");
}

AccessMode LC2PCTable::Str2AccessMode( const std::string& mode ) const
{
  if (mode==UPDATE)
    return Update;
  else if (mode==RDONLY)
    return ReadOnly;
  else throw Exception("AccessMode is neither Update or Read-Only!", "LC2PCTable::Str2AccessMode",
                       "CoralAuthenticationService");
}

//----------------------------------------------------------------------------

void LC2PCTable::addLC2PC( const std::string& logConStr,
                           const std::string& physConStr,
                           AccessMode accessMode,
                           const std::string& authenticationMechanism )
{
  coral::AttributeList rowBuffer;

  unsigned int order = getNextOrderNo( logConStr );

  m_table.dataEditor().rowBuffer( rowBuffer );
  rowBuffer[ columnNames::logConStr() ].data< columnTypes::logConStr>() =
    logConStr;
  rowBuffer[ columnNames::physConStr() ].data< columnTypes::physConStr>() =
    physConStr;
  rowBuffer[ columnNames::order() ].data< columnTypes::order >() = order;
  rowBuffer[ columnNames::accessMode() ].data< columnTypes::accessMode >() =
    AccessMode2Str( accessMode );
  rowBuffer[ columnNames::authenticationMechanism() ].data<
    columnTypes::authenticationMechanism >() = authenticationMechanism;

  m_table.dataEditor().insertRow( rowBuffer );
}

//----------------------------------------------------------------------------

void LC2PCTable::generateWhere(
                               AttributeList& whereData, std::string& whereString,
                               bool wildcards,
                               const std::string& table,
                               const std::string& logConStr,
                               const std::string& physConStr,
                               const std::string& accessMode,
                               const std::string& authenticationMechanism )
{
  if ( !wildcards || !logConStr.empty() )
  {
    whereData.extend( "logConStr",
                      AttributeSpecification::typeNameForType<columnTypes::logConStr>() );
    whereData[ "logConStr" ].data<columnTypes::logConStr>() = logConStr;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::logConStr()+" = :logConStr";
  }
  if ( !wildcards || !physConStr.empty() )
  {
    whereData.extend( "physConStr",
                      AttributeSpecification::typeNameForType<columnTypes::physConStr>() );
    whereData[ "physConStr" ].data<columnTypes::physConStr>() = physConStr;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::physConStr()+" = :physConStr";
  }
  if ( !wildcards || !accessMode.empty() )
  {
    whereData.extend( "accessMode",
                      AttributeSpecification::typeNameForType<columnTypes::accessMode>() );
    whereData[ "accessMode" ].data<columnTypes::accessMode>() = accessMode;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::accessMode()+" = :accessMode";
  }
  if ( !wildcards || !authenticationMechanism.empty() )
  {
    whereData.extend( "authenticationMechanism",
                      AttributeSpecification::typeNameForType<columnTypes::authenticationMechanism>() );
    whereData[ "authenticationMechanism" ].data<columnTypes::authenticationMechanism>() =
      authenticationMechanism;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::authenticationMechanism()+" = :authenticationMechanism";
  }
}

//----------------------------------------------------------------------------

bool LC2PCTable::LC2PCExists( bool wildcards,
                              const std::string& logConStr,
                              const std::string& physConStr,
                              const std::string& accessMode,
                              const std::string& authenticationMechanism )
{

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, wildcards,
                "", logConStr, physConStr, accessMode, authenticationMechanism );

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

long LC2PCTable::delLC2PC( const std::string& logConStr,
                           const std::string& physConStr,
                           const std::string& accessMode,
                           const std::string& authenticationMechanism )
{

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, true /*wildcards*/,
                "", logConStr, physConStr, accessMode, authenticationMechanism );

  return m_table.dataEditor().deleteRows( whereString, whereData );
}


//----------------------------------------------------------------------------

DatabaseServiceSet* LC2PCTable::lookup( const std::string& logicalName, AccessMode accessMode,
                                        const std::string& authenticationMechanism )
{
  //  std::cout << "lookup( " << logicalName << ", " << AccessMode2Str( accessMode ) << ", "
  //    << authenticationMechanism << " )" << std::endl;

  coral::AttributeList whereData;
  std::string whereString="";

  std::auto_ptr<IQuery> query( m_schema.newQuery() );

  query->addToTableList( defaultTableName(), "" );

  query->addToOutputList( columnNames::physConStr(), "pcs");
  query->addToOutputList( columnNames::authenticationMechanism(),
                          "authMechanism");
  query->addToOutputList( columnNames::accessMode(),
                          "accessMode");

  query->addToOrderList( columnNames::order() );

  // return also update databases for readonly connections
  std::string accessModeStr="";
  if ( accessMode == coral::Update )
    accessModeStr = AccessMode2Str( coral::Update );

  generateWhere(whereData, whereString, true /*wildcards*/,
                "" /*table*/, logicalName, "" /*physConStr*/, accessModeStr,
                authenticationMechanism );

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();
  DatabaseServiceSet* set = new DatabaseServiceSet("Coral::CoralAuthenticationService");

  while ( cursor.next() ) {
    const AttributeList &curr( cursor.currentRow() );
    //   std::cout << " connection: '" << curr << std::endl;
    set->appendReplica( curr[ "pcs" ].data< columnTypes::physConStr >(),
                        curr[ "authMechanism" ].data< columnTypes::authenticationMechanism >(),
                        Str2AccessMode(  curr[ "accessMode" ].data< columnTypes::accessMode >() ));
  }

  return set;
}

//----------------------------------------------------------------------------

unsigned int LC2PCTable::getNextOrderNo( const std::string& logConStr)
{
  coral::AttributeList whereData;
  std::string whereString="";

  std::auto_ptr<IQuery> query( m_schema.newQuery() );

  query->addToTableList( defaultTableName(), "" );

  query->addToOutputList( columnNames::order(), "orderNo");
  //query->addToOutputList( "max( "+columnNames::order()+" )", "order");

  generateWhere( whereData, whereString, true /*wildcards*/,
                 "" /*table*/, logConStr, "" /*physConStr*/,
                 "" /*accessMode*/, "" /*authMech*/ );

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  query->addToOrderList( " \"orderNo\" desc");
  query->limitReturnedRows( 1 );

  ICursor &cursor=query->execute();

  if ( !cursor.next() )
    return 0;

  return cursor.currentRow()[ "orderNo" ].data< columnTypes::order>()+1;
}

//----------------------------------------------------------------------------
