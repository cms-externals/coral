#include <iostream>
#include "CredentialsTable.h"

#include "Config.h"

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "RelationalAccess/AuthenticationCredentials.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"


using namespace coral;

//---------------------------------------------------------------------------

CredentialsTable::CredentialsTable(ISchema & schema)
  : m_schema( schema )
  , m_table( m_schema.tableHandle( defaultTableName() ) )
  , m_cipher()
{
}

//----------------------------------------------------------------------------

CredentialsTable::~CredentialsTable()
{
}

//----------------------------------------------------------------------------

void CredentialsTable::createTable(ISchema &schema)
{
  // create a table description
  coral::TableDescription tableDesc;
  tableDesc.setName( defaultTableName() );
  /*
  tableDesc.insertColumn( columnNames::connectionId(),
      AttributeSpecification::typeNameForType<columnTypes::connectionId>( ) );
  tableDesc.setPrimaryKey( columnNames::connectionId() );
  tableDesc.setNotNullConstraint( columnNames::connectionId() );
  */
  tableDesc.insertColumn( columnNames::physConStr(),
                          AttributeSpecification::typeNameForType<columnTypes::physConStr>( ),
                          255, false );

  tableDesc.setNotNullConstraint( columnNames::physConStr() );

  tableDesc.insertColumn( columnNames::user(),
                          AttributeSpecification::typeNameForType<columnTypes::user>( ),
                          255, false );

  tableDesc.insertColumn( columnNames::passwd(),
                          AttributeSpecification::typeNameForType<columnTypes::passwd>( ),
                          255, false );

  tableDesc.insertColumn( columnNames::dbRole(),
                          AttributeSpecification::typeNameForType<columnTypes::dbRole>( ),
                          255, false );
  tableDesc.setNotNullConstraint( columnNames::dbRole() );
  /*
  // the combination physConStr, dbRole, (FIXME owner) should be unique
  std::vector<std::string> uniqueCol;
  uniqueCol.push_back( columnNames::physConStr() );
  uniqueCol.push_back( columnNames::dbRole() );
  tableDesc.setUniqueConstraint( uniqueCol );
  */
  // add primary key
  std::vector<std::string> primaryKey;
  primaryKey.push_back( columnNames::physConStr() );
  primaryKey.push_back( columnNames::dbRole() );
  tableDesc.setPrimaryKey( primaryKey );

  schema.createTable( tableDesc );
}

//----------------------------------------------------------------------------

void CredentialsTable::deleteTable( ISchema &schema )
{
  schema.dropIfExistsTable( defaultTableName() );
}

//----------------------------------------------------------------------------

void CredentialsTable::addCredentials( const std::string& physConStr,
                                       const std::string& user, const std::string& password,
                                       const std::string& dbRole )
{
  coral::AttributeList rowBuffer;

  m_table.dataEditor().rowBuffer( rowBuffer );
  rowBuffer[ columnNames::physConStr() ].data< columnTypes::physConStr>() =
    physConStr;
  rowBuffer[ columnNames::user() ].data< columnTypes::user >() = user;
  rowBuffer[ columnNames::passwd() ].data< columnTypes::passwd >() =
    EncodeBase64( m_cipher.Encrypt(getConfig().password(), password ) );
  if ( !dbRole.empty() )
    rowBuffer[ columnNames::dbRole() ].data< columnTypes::dbRole >() = dbRole;
  else
    rowBuffer[ columnNames::dbRole() ].data< columnTypes::dbRole >() =
      IAuthenticationCredentials::defaultRole();

  m_table.dataEditor().insertRow( rowBuffer );
}

//----------------------------------------------------------------------------

void CredentialsTable::generateWhere(
                                     AttributeList & whereData, std::string& whereString,
                                     bool wildcards,
                                     const std::string& table,
                                     const std::string& physConStr, const std::string& user,
                                     const std::string& dbRole )
{
  if ( !wildcards || !physConStr.empty() )
  {
    whereData.extend( "physConStr",
                      AttributeSpecification::typeNameForType<columnTypes::physConStr>() );
    whereData[ "physConStr" ].data<columnTypes::physConStr>() = physConStr;
    whereString =  table+columnNames::physConStr()+" = :physConStr";
  }
  if ( !wildcards || !user.empty() )
  {
    whereData.extend( "user",
                      AttributeSpecification::typeNameForType<columnTypes::user>() );
    whereData[ "user" ].data<columnTypes::user>() = user;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::user()+" = :user";
  }
  if ( !wildcards || !dbRole.empty() )
  {
    whereData.extend( "dbRole",
                      AttributeSpecification::typeNameForType<columnTypes::dbRole>() );
    whereData[ "dbRole" ].data<columnTypes::dbRole>() = dbRole;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::dbRole()+" = :dbRole";
  }
}

//----------------------------------------------------------------------------

long CredentialsTable::delCredentials( const std::string& physConStr,
                                       const std::string& user, const std::string& dbRole )
{

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, true,  "", physConStr, user, dbRole);

  return m_table.dataEditor().deleteRows( whereString, whereData );
}

//----------------------------------------------------------------------------

void CredentialsTable::listCredentials( const std::string& physConStr,
                                        const std::string& user, const std::string& dbRole )
{
  coral::AttributeList rowBuffer;
  m_table.dataEditor().rowBuffer( rowBuffer );

  std::unique_ptr<IQuery> query( m_schema.newQuery() );

  query->defineOutput( rowBuffer );
  query->addToTableList( defaultTableName(), "db" );

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, true, "db.", physConStr, user, dbRole);

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();

  while ( cursor.next() ) {
    const AttributeList &curr( cursor.currentRow() );
    std::cout << " connection: '" << curr[ columnNames::physConStr() ].data<
      columnTypes::physConStr>()
              << "' User: '" << curr[ columnNames::user() ].data<
      columnTypes::user>()
              << "' passwd: '" << curr[ columnNames::passwd() ].data<
      columnTypes::passwd>()
              << "' dbRole: '" << curr[ columnNames::dbRole() ].data<
      columnTypes::dbRole>()
              << "'" << std::endl;
  }
}

//----------------------------------------------------------------------------

bool CredentialsTable::existCredentials( const std::string& physConStr,
                                         const std::string& user, const std::string& dbRole )
{
  coral::AttributeList rowBuffer;
  m_table.dataEditor().rowBuffer( rowBuffer );

  std::unique_ptr<IQuery> query( m_schema.newQuery() );

  query->defineOutput( rowBuffer );
  query->addToTableList( defaultTableName(), "db" );

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, true, "db.", physConStr, user, dbRole);

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();

  bool ret = cursor.next();
  return ret;
}

//----------------------------------------------------------------------------

const AuthenticationCredentials CredentialsTable::getCredentials( const std::string& physConStr,
                                                                  const std::string& dbRole )
{
  coral::AttributeList rowBuffer;
  m_table.dataEditor().rowBuffer( rowBuffer );

  std::unique_ptr<IQuery> query( m_schema.newQuery() );

  query->defineOutput( rowBuffer );
  query->addToTableList( defaultTableName(), "db" );

  coral::AttributeList whereData;
  std::string whereString="";

  // wildcards = true needed because we don't know the user name
  generateWhere(whereData, whereString, true, "db.", physConStr, "", dbRole);

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();

  if ( ! cursor.next() ) {
    throw AuthenticationServiceException("no credentials found", "CredentialsTable::getCredentials",
                                         "CoralAuthentication");
  };

  AuthenticationCredentials credentials("CoralAuthentication");
  const AttributeList &curr( cursor.currentRow() );

  credentials.registerItem( IAuthenticationCredentials::userItem(),
                            curr[ columnNames::user() ].data< columnTypes::user >() );
  credentials.registerItem( IAuthenticationCredentials::passwordItem(),
                            decryptPasswd( curr[ columnNames::passwd() ].data< columnTypes::passwd >() )
                            );
  return credentials;
}

//----------------------------------------------------------------------------

const std::string CredentialsTable::decryptPasswd( const std::string & cryptPw )
{
  return m_cipher.Decrypt( getConfig().password(), DecodeBase64( cryptPw ) );
}

//----------------------------------------------------------------------------
#if 0
std::vector<unsigned int>
CredentialsTable::getIds( const std::string& physConStr,
                          const std::string& user, const std::string& dbRole )
{
  coral::AttributeList rowBuffer;
  rowBuffer.extend( columnNames::connectionId(),
                    AttributeSpecification::typeNameForType<columnTypes::connectionId>() );

  IQuery *query = m_schema.newQuery();

  query->defineOutput( rowBuffer );
  query->addToTableList( defaultTableName(), "db" );

  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, "db.", physConStr, user, dbRole);

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();

  std::vector<unsigned int> result;
  while (cursor.next() ) {
    const AttributeList &curr( cursor.currentRow() );

    result.push_back(
                     curr[ columnNames::connectionId() ].
                     data< columnTypes::connectionId >() );
  }
  return result;
}
#endif
