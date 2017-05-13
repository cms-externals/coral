#include "PermissionsTable.h"
#include "CredentialsTable.h"

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

PermissionsTable::PermissionsTable(ISchema & schema)
  : m_schema( schema )
  , m_table( m_schema.tableHandle( defaultTableName() ) )
{
}

//----------------------------------------------------------------------------

PermissionsTable::~PermissionsTable()
{
}

//----------------------------------------------------------------------------

void PermissionsTable::createTable(ISchema &schema)
{
  // create a table description
  coral::TableDescription tableDesc;
  tableDesc.setName( defaultTableName() );

  tableDesc.insertColumn( columnNames::physConStr(),
                          AttributeSpecification::typeNameForType<columnTypes::physConStr>(),
                          255, false );
  // FIXME primary key!
  //tableDesc.setPrimaryKey( columnNames::physConStr() );
  tableDesc.setNotNullConstraint( columnNames::physConStr() );

  tableDesc.insertColumn( columnNames::dbRole(),
                          AttributeSpecification::typeNameForType<columnTypes::dbRole>(),
                          255, false );

  tableDesc.insertColumn( columnNames::distinguishedName(),
                          AttributeSpecification::typeNameForType<columnTypes::distinguishedName>(),
                          255, false );

  tableDesc.insertColumn( columnNames::vomsFQAN(),
                          AttributeSpecification::typeNameForType<columnTypes::vomsFQAN>(),
                          255, false );

  // create foreign key to database table
  tableDesc.createForeignKey( columnNames::physConStr()+"_FK",
                              columnNames::physConStr(),
                              CredentialsTable::defaultTableName(),
                              CredentialsTable::columnNames::physConStr() );

  schema.createTable( tableDesc );
}

//----------------------------------------------------------------------------

void PermissionsTable::deleteTable( ISchema &schema )
{
  schema.dropIfExistsTable( defaultTableName() );
}

//----------------------------------------------------------------------------

void PermissionsTable::addPermission(
                                     const columnTypes::physConStr& physConStr,
                                     const columnTypes::dbRole& dbRole,
                                     const columnTypes::distinguishedName &dn,
                                     const columnTypes::vomsFQAN &vomsFQAN )
{
  coral::AttributeList rowBuffer;

  m_table.dataEditor().rowBuffer( rowBuffer );
  rowBuffer[ columnNames::physConStr() ].data< columnTypes::physConStr >() =
    physConStr;
  rowBuffer[ columnNames::dbRole() ].data< columnTypes::dbRole >() =
    dbRole;
  rowBuffer[ columnNames::distinguishedName() ].data<
    columnTypes::distinguishedName >() = dn;
  rowBuffer[ columnNames::vomsFQAN() ].data<
    columnTypes::vomsFQAN >() = vomsFQAN;

  m_table.dataEditor().insertRow( rowBuffer );
}

//----------------------------------------------------------------------------

int PermissionsTable::delPermission(
                                    const columnTypes::physConStr& physConStr,
                                    const columnTypes::dbRole& dbRole,
                                    const columnTypes::distinguishedName &dn,
                                    const columnTypes::vomsFQAN &vomsFQAN )
{
  coral::AttributeList whereData;
  std::string whereString="";

  generateWhere(whereData, whereString, true, "",
                physConStr, dbRole, dn, vomsFQAN);

  return m_table.dataEditor().deleteRows( whereString, whereData );
}

//----------------------------------------------------------------------------

void PermissionsTable::generateWhere(
                                     AttributeList & whereData, std::string& whereString,
                                     bool wildcards, const std::string& table,
                                     const std::string& physConStr, const std::string& dbRole,
                                     const std::string& dn, const std::string& vomsFQAN )
{
  if ( !wildcards || !physConStr.empty() )
  {
    whereData.extend( "physConStr",
                      AttributeSpecification::typeNameForType<columnTypes::physConStr>() );
    whereData[ "physConStr" ].data<columnTypes::physConStr>() = physConStr;
    whereString =  table+columnNames::physConStr()+" = :physConStr";
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
  if ( !wildcards || !dn.empty() )
  {
    whereData.extend( "distinguishedName",
                      AttributeSpecification::typeNameForType<columnTypes::distinguishedName>() );
    whereData[ "distinguishedName" ].data<columnTypes::distinguishedName>() = dn;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::distinguishedName()+" = :distinguishedName";
  }
  if ( !wildcards || !vomsFQAN.empty() )
  {
    whereData.extend( "vomsFQAN",
                      AttributeSpecification::typeNameForType<columnTypes::vomsFQAN>() );
    whereData[ "vomsFQAN" ].data<columnTypes::vomsFQAN>() = vomsFQAN;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::vomsFQAN()+" = :vomsFQAN";
  }
}
