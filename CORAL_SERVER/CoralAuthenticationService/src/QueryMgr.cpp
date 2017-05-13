#include <iostream>
#include "QueryMgr.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/IOperationWithQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"

#include <iomanip>

using namespace coral;

//---------------------------------------------------------------------------

QueryMgr::QueryMgr(ISchema & schema)
  : m_schema( schema )
  , m_lcTable( schema )
  , m_pcTable( schema )
  , m_lc2pcTable( schema )
  , m_crTable( schema )
  , m_permTable( schema )
{
}

//----------------------------------------------------------------------------

QueryMgr::~QueryMgr()
{
}

//----------------------------------------------------------------------------

void QueryMgr::initDatabase( ISchema &schema)
{
  PhysConTable::createTable( schema );
  LogConTable::createTable( schema );
  LC2PCTable::createTable( schema );
  CredentialsTable::createTable( schema );
  PermissionsTable::createTable( schema );
}

//-----------------------------------------------------------------------

void QueryMgr::deleteDatabase( ISchema & schema )
{
  PermissionsTable::deleteTable( schema );
  CredentialsTable::deleteTable( schema );
  LC2PCTable::deleteTable( schema );
  PhysConTable::deleteTable( schema );
  LogConTable::deleteTable( schema );
}

//----------------------------------------------------------------------------

void QueryMgr::addConnection(const std::string& lcString, const std::string& pcString,
                             const std::string &user, const std::string& passwd, const std::string& role_,
                             const std::string &accessMode, const std::string& authMechanism )
{
  std::string role=role_;

  AccessMode aMode=Update;
  if ( !accessMode.empty() ) {
    if (accessMode == "update" || accessMode == "Update" || accessMode == "UPDATE" )
      aMode = Update;
    else if (accessMode == "read-only" || accessMode == "Read-Only" || accessMode == "READ-ONLY"
             || accessMode == "ReadOnly" || accessMode == "readonly")
      aMode = ReadOnly;
    else throw coral::Exception("access mode should either be 'update' or 'read-only'",
                                "coral_authentication_manager","");
  }

  if ( !lcString.empty() )
  {
    if ( pcString.empty() )
      throw coral::Exception("please specify a physical connection for the alias",
                             "coral_authentication_manager", "" );

    if ( !pcString.empty() &&  !m_pcTable.physConExists( pcString ) )
      m_pcTable.addPhysCon( pcString );

    if ( !m_lcTable.logConExists( lcString ) )
      m_lcTable.addLogCon( lcString );

    // add lc -> pc mapping
    if ( !m_lc2pcTable.LC2PCExists( false, lcString, pcString, aMode, authMechanism ) )
      m_lc2pcTable.addLC2PC( lcString, pcString, aMode, authMechanism );
    // FIXME order, accessmode
  }

  if ( !user.empty() || !passwd.empty() ) {
    if ( pcString.empty() || user.empty() || passwd.empty() )
      throw Exception("to add credentials, pcs, user and password are necessary",
                      "coral_authentication_manager","");

    if ( !pcString.empty() &&  !m_pcTable.physConExists( pcString ) )
      m_pcTable.addPhysCon( pcString );

    // add credentials to pc
    if ( role.empty() )
      role = IAuthenticationCredentials::defaultRole();

    m_crTable.addCredentials( pcString, user, passwd, role );
  }
}

//----------------------------------------------------------------------------

void QueryMgr::delConnection(const std::string& lcString, const std::string& pcString,
                             const std::string &user, const std::string& role_)
{
  std::string role = role_;

  if (!lcString.empty() ) {
    m_lc2pcTable.delLC2PC( lcString, pcString );

    // if we deleted the last lc2pc, we also delete the lc
    if ( !m_lc2pcTable.LC2PCExists( true, lcString, "", "", "" ) )
      m_lcTable.delLogCon( lcString );

  }
  // if we got a user name, delete the credentials
  if ( !user.empty() || !role.empty() ) {
    if ( pcString.empty() )
      throw Exception("to delete credentials, pcs and user or role are necessary",
                      "coral_authentication_manager","");
    /*
      if ( role.empty() )
        role = IAuthenticationCredentials::defaultRole();
    */
    long deleted = m_crTable.delCredentials( pcString, user, role );
    std::cout << "Deleted "<< deleted << " credentials" << std::endl;
  };

  // remove pc if there are no references any more
  if ( !m_lc2pcTable.LC2PCExists( true, "", pcString, "", "" ) &&
       !m_crTable.existCredentials( pcString, "" ) )
  {
    m_pcTable.delPhysCon( pcString );
  };
}

//----------------------------------------------------------------------------

void QueryMgr::addPermission( const std::string& lcString, const std::string& pcString,
                              const std::string& role_,
                              const std::string& dn, const std::string& vomsFQAN )
{
  std::string role( role_ );

  if ( role.empty() )
    role = IAuthenticationCredentials::defaultRole();

  if ( ( !lcString.empty()  && !pcString.empty() )
       || ( lcString.empty()  && pcString.empty() ) )
    throw Exception("Please specify logical or physical connection string!",
                    "QueryMgr", "CoralAuthenticationService");

  if ( dn.empty() && vomsFQAN.empty() )
    throw Exception("Please specify either voms or dn.",
                    "coral_authentication_manager","");

  if ( !pcString.empty() )
  {
    // simple case is done by class PermissionTable

    if ( m_crTable.existCredentials( pcString, role ) )
      throw  Exception("Error, there are now matching credentials in the database",
                       "coral_authentication_manager","");

    m_permTable.addPermission( pcString, role, dn, vomsFQAN );
  }
  else
  {
    // we have to resolve a logical connection string

    //std::auto_ptr<IOperationWithQuery> operation( m_permTable.dataEditor().insertWithQuery() );
    std::auto_ptr<IOperationWithQuery> operation( m_schema.tableHandle(PermissionsTable::defaultTableName()).dataEditor().insertWithQuery() );

    IQueryDefinition *query( &operation->query() );
    coral::AttributeList outputBuffer;

    query->addToTableList( m_lc2pcTable.defaultTableName(), "lc2pc" );
    query->addToTableList( m_crTable.defaultTableName(), "cr" );

    query->addToOutputList("lc2pc."+LC2PCTable::columnNames::physConStr(),
                           PermissionsTable::columnNames::physConStr() );
    query->addToOutputList("cr."+PermissionsTable::columnNames::dbRole(),
                           PermissionsTable::columnNames::dbRole() );

    outputBuffer.extend( PermissionsTable::columnNames::physConStr() ,
                         AttributeSpecification::typeNameForType<
                         PermissionsTable::columnTypes::physConStr >() );
    outputBuffer.extend( PermissionsTable::columnNames::dbRole() ,
                         AttributeSpecification::typeNameForType<
                         PermissionsTable::columnTypes::dbRole >() );

    query->addToOutputList(" :fqan ",
                           PermissionsTable::columnNames::vomsFQAN() );
    query->addToOutputList(" :dn ",
                           PermissionsTable::columnNames::distinguishedName() );

    outputBuffer.extend( PermissionsTable::columnNames::vomsFQAN() ,
                         AttributeSpecification::typeNameForType<
                         PermissionsTable::columnTypes::vomsFQAN >() );
    outputBuffer.extend( PermissionsTable::columnNames::distinguishedName() ,
                         AttributeSpecification::typeNameForType<
                         PermissionsTable::columnTypes::distinguishedName >() );

    coral::AttributeList whereData;
    std::string whereString;

    // this is not actually part of the where, but rather part of the output list
    whereData.extend( "fqan",
                      AttributeSpecification::typeNameForType<
                      PermissionsTable::columnTypes::vomsFQAN >() );
    whereData[ "fqan" ].data< PermissionsTable::columnTypes::vomsFQAN >() = vomsFQAN;

    whereData.extend( "dn",
                      AttributeSpecification::typeNameForType<
                      PermissionsTable::columnTypes::distinguishedName >() );
    whereData[ "dn" ].data< PermissionsTable::columnTypes::distinguishedName >() = dn;

    // the join
    whereString += "lc2pc."+LC2PCTable::columnNames::physConStr()+" = "
      + "cr."+CredentialsTable::columnNames::physConStr();
    whereString += " and ";

    // the actual where clause
    whereString +="lc2pc."+LC2PCTable::columnNames::logConStr()
      + " = :lcString ";
    whereData.extend( "lcString",
                      AttributeSpecification::typeNameForType<
                      LC2PCTable::columnTypes::logConStr >() );
    whereData[ "lcString" ].data< LC2PCTable::columnTypes::logConStr >() = lcString;

    whereString +=" and cr."+CredentialsTable::columnNames::dbRole()
      + " = :dbRole ";
    whereData.extend( "dbRole",
                      AttributeSpecification::typeNameForType<
                      CredentialsTable::columnTypes::dbRole >() );
    whereData[ "dbRole" ].data< CredentialsTable::columnTypes::dbRole >() = role;


    query->setCondition( whereString, whereData);

    //    query->defineOutput( outputBuffer );

    operation->execute();
#if 0
    ICursor &cursor=query->execute();

    while ( cursor.next() ) {
      const AttributeList &curr( cursor.currentRow() );
      std::cout << " connection: '" << curr << std::endl;
    };
#endif

  }
}

//----------------------------------------------------------------------------

int QueryMgr::delPermission( const std::string& /*lcString*/, const std::string& pcString,
                             const std::string& role_,
                             const std::string& dn, const std::string& vomsFQAN )
{
  std::string role = role_;

  if ( role.empty() )
    role = IAuthenticationCredentials::defaultRole();

  if ( pcString.empty() )
    throw Exception("physical connection string is missing",
                    "coral_authentication_manager","");

  if ( dn.empty() && vomsFQAN.empty() )
    throw Exception("please specify either voms or dn.",
                    "coral_authentication_manager","");

  return m_permTable.delPermission( pcString, role, dn, vomsFQAN );
}

//----------------------------------------------------------------------------

DatabaseServiceSet *QueryMgr::lookup( const std::string& logicalName, AccessMode accessMode,
                                      const std::string& authenticationMechanism )
{
  return m_lc2pcTable.lookup( logicalName, accessMode, authenticationMechanism );
}

//----------------------------------------------------------------------------
/*
void QueryMgr::generateWhere(
    AttributeList & whereData, std::string& whereString,
    const std::string& table,
    const std::string& physConStr, const std::string& user,
    const std::string& dbRole )
{
  if ( !physConStr.empty() )
  {
    whereData.extend( "physConStr",
        AttributeSpecification::typeNameForType<columnTypes::physConStr>() );
    whereData[ "physConStr" ].data<columnTypes::physConStr>() = physConStr;
    whereString =  table+columnNames::physConStr()+" = :physConStr";
  }
  if ( !user.empty() )
  {
    whereData.extend( "user",
        AttributeSpecification::typeNameForType<columnTypes::user>() );
    whereData[ "user" ].data<columnTypes::user>() = user;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::user()+" = :user";
  }
  if ( !dbRole.empty() )
  {
    whereData.extend( "dbRole",
        AttributeSpecification::typeNameForType<columnTypes::dbRole>() );
    whereData[ "dbRole" ].data<columnTypes::dbRole>() = dbRole;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  table+columnNames::dbRole()+" = :dbRole";
  }
}
*/
 //----------------------------------------------------------------------------
#if 0
void QueryMgr::queryConnections(
                                const std::string& lcString, const std::string& pcString,
                                const std::string& user, const std::string& dbRole )
{
  /*
  coral::AttributeList rowBuffer;
  rowBuffer.extend( LC2PCTable::columnNames::logConStr(),
      AttributeSpecification::typeNameForType<
        LC2PCTable::columnTypes::logConStr>( ) );

  rowBuffer.extend( LC2PCTable::columnNames::physConStr(),
      AttributeSpecification::typeNameForType<
        LC2PCTable::columnTypes::physConStr>( ) );
  */
  /*
  rowBuffer.extend( m_pc2lcTable.columnNames::physConStr(),
      AttributeSpecification::typeNameForType<
        m_pc2lcTable.columnTypes::physConStr>( ) );
  */
  //  m_table.dataEditor().rowBuffer( rowBuffer );

  IQuery *query = m_schema.newQuery();

  query->addToTableList( m_lc2pcTable.defaultTableName(), "lc2pc" );
  //query->addToTableList( m_crTable.defaultTableName(), "cr" );

  query->addToOutputList("lc2pc."+LC2PCTable::columnNames::logConStr(),
                         "lcs");
  query->addToOutputList("lc2pc."+LC2PCTable::columnNames::physConStr(),
                         "pcs");
  /*
  query->addToOutputList("cr."+CredentialsTable::columnNames::user(),
      "user");
  query->addToOutputList("cr."+CredentialsTable::columnNames::dbRole(),
      "role");
  */
  //query->defineOutput( rowBuffer );

  coral::AttributeList whereData;
  std::string whereString=    "cr." + CredentialsTable::columnNames::physConStr()
    + " = " + " lc2pc." + LC2PCTable::columnNames::physConStr();

  if ( !lcString.empty() )
  {
    whereData.extend( "lcString",
                      AttributeSpecification::typeNameForType<
                      LC2PCTable::columnTypes::logConStr>() );
    whereData[ "lcString" ].data< LC2PCTable::columnTypes::logConStr >() =
      lcString;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  "lc2pc."+ LC2PCTable::columnNames::logConStr() +
      " = :lcString";
  }

  if ( !pcString.empty() )
  {
    whereData.extend( "pcString",
                      AttributeSpecification::typeNameForType<
                      LC2PCTable::columnTypes::physConStr>() );
    whereData[ "pcString" ].data< LC2PCTable::columnTypes::physConStr >() =
      pcString;
    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  "lc2pc." + LC2PCTable::columnNames::physConStr() +
      " = :pcString";
  }

  if ( !user.empty() )
  {
    whereData.extend( "user",
                      AttributeSpecification::typeNameForType<
                      CredentialsTable::columnTypes::user>() );
    whereData[ "user" ].data< CredentialsTable::columnTypes::user >() = user;

    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  "cr." + CredentialsTable::columnNames::user() +
      " = :user";
  }

  if ( !dbRole.empty() )
  {
    whereData.extend( "dbRole",
                      AttributeSpecification::typeNameForType<
                      CredentialsTable::columnTypes::dbRole>() );
    whereData[ "dbRole" ].data< CredentialsTable::columnTypes::dbRole >() =
      dbRole;

    if ( !whereString.empty() )
      whereString += " and ";
    whereString +=  "cr." + CredentialsTable::columnNames::dbRole() +
      " = :dbRole";
  }

  if ( !whereString.empty() )
    query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();

  while ( cursor.next() ) {
    const AttributeList &curr( cursor.currentRow() );
    std::cout << " connection: '" << curr << std::endl;
    /*[ columnNames::physConStr() ].data<
        columnTypes::physConStr>()
        << "' User: '" << curr[ columnNames::user() ].data<
        columnTypes::user>()
        << "' passwd: '" << curr[ columnNames::passwd() ].data<
        columnTypes::passwd>()
        << "' dbRole: '" << curr[ columnNames::dbRole() ].data<
        columnTypes::dbRole>()
        << "'" << std::endl;*/
  }
  delete query;
}
#endif
void QueryMgr::queryConnections(
                                const std::string& lcString, const std::string& pcString,
                                const std::string& user, const std::string& dbRole )
{

  std::auto_ptr<IQuery> query( m_schema.newQuery() );

  // main query is on credentials table
  query->addToTableList( m_crTable.defaultTableName(), "cr" );
  query->addToOutputList("cr."+CredentialsTable::columnNames::user(),
                         "user");
  query->addToOutputList("cr."+CredentialsTable::columnNames::dbRole(),
                         "role");
  query->addToOutputList("cr."+CredentialsTable::columnNames::physConStr(),
                         "physConStr");

  coral::AttributeList crWhereData;
  std::string crWhereString;
  if ( !user.empty() )
  {
    crWhereData.extend( "user",
                        AttributeSpecification::typeNameForType<
                        CredentialsTable::columnTypes::user>() );
    crWhereData[ "user" ].data< CredentialsTable::columnTypes::user >() = user;

    if ( !crWhereString.empty() )
      crWhereString += " and ";
    crWhereString +=  "cr." + CredentialsTable::columnNames::user() +
      " = :user";
  }

  if ( !pcString.empty() )
  {
    crWhereData.extend( "pcString",
                        AttributeSpecification::typeNameForType<
                        LC2PCTable::columnTypes::physConStr>() );
    crWhereData[ "pcString" ].data< LC2PCTable::columnTypes::physConStr >() =
      pcString;
    if ( !crWhereString.empty() )
      crWhereString += " and ";
    crWhereString +=  "cr." + CredentialsTable::columnNames::physConStr() +
      " = :pcString";
  }

  if ( !dbRole.empty() )
  {
    crWhereData.extend( "dbRole",
                        AttributeSpecification::typeNameForType<
                        CredentialsTable::columnTypes::dbRole>() );
    crWhereData[ "dbRole" ].data< CredentialsTable::columnTypes::dbRole >() =
      dbRole;

    if ( !crWhereString.empty() )
      crWhereString += " and ";
    crWhereString +=  "cr." + CredentialsTable::columnNames::dbRole() +
      " = :dbRole";
  }

  // alias subquery
  std::string subquery=std::string(" ( select ")
    + "lc2pc."+LC2PCTable::columnNames::logConStr() + " as lcs "
    + " from "+m_lc2pcTable.defaultTableName()+" as lc2pc "
    + " where cr." + CredentialsTable::columnNames::physConStr()
    + " = " + " lc2pc." + LC2PCTable::columnNames::physConStr();
  if ( !lcString.empty() )
  {
    crWhereData.extend( "lcString",
                        AttributeSpecification::typeNameForType<
                        LC2PCTable::columnTypes::logConStr>() );
    crWhereData[ "lcString" ].data< LC2PCTable::columnTypes::logConStr >() =
      lcString;

    if ( !crWhereString.empty() )
      crWhereString += " and ";

    //subquery +=  "lc2pc."+ LC2PCTable::columnNames::logConStr() +
    crWhereString +=  "lcs  = :lcString";
  }
  subquery += " ) ";

  query->addToOutputList( subquery, "lcs" );

  // permissions
  std::string subquery1=std::string(" ( select ")
    + "perm."+PermissionsTable::columnNames::vomsFQAN() + " as fqan "
    //    + "perm."+PermissionsTable::columnNames::distinguishedName() + " as dn "
    + " from "+m_permTable.defaultTableName()+" as perm "
    + " where cr." + CredentialsTable::columnNames::physConStr()
    + " = " + " perm." + PermissionsTable::columnNames::physConStr()
    + " and  cr." + CredentialsTable::columnNames::dbRole()
    + " = " + " perm." + PermissionsTable::columnNames::dbRole();
  /*  if ( !lcString.empty() )
     {
       crWhereData.extend( "lcString",
           AttributeSpecification::typeNameForType<
           LC2PCTable::columnTypes::logConStr>() );
       crWhereData[ "lcString" ].data< LC2PCTable::columnTypes::logConStr >() =
         lcString;
       subquery += " and ";
       subquery +=  "lc2pc."+ LC2PCTable::columnNames::logConStr() +
         " = :lcString";
         }*/
  subquery1 += " ) ";
  query->addToOutputList( subquery1, "fqan" );

  // permissions
  std::string subquery2=std::string(" ( select ")
    //    + "perm."+PermissionsTable::columnNames::vomsFQAN() + " as fqan, "
    + "perm."+PermissionsTable::columnNames::distinguishedName() + " as dn "
    + " from "+m_permTable.defaultTableName()+" as perm "
    + " where cr." + CredentialsTable::columnNames::physConStr()
    + " = " + " perm." + PermissionsTable::columnNames::physConStr()
    + " and  cr." + CredentialsTable::columnNames::dbRole()
    + " = " + " perm." + PermissionsTable::columnNames::dbRole();
  /*  if ( !lcString.empty() )
     {
       crWhereData.extend( "lcString",
           AttributeSpecification::typeNameForType<
           LC2PCTable::columnTypes::logConStr>() );
       crWhereData[ "lcString" ].data< LC2PCTable::columnTypes::logConStr >() =
         lcString;
       subquery += " and ";
       subquery +=  "lc2pc."+ LC2PCTable::columnNames::logConStr() +
         " = :lcString";
         }*/
  subquery2 += " ) ";
  query->addToOutputList( subquery2, "dn" );

#if 0
  IQueryDefinition &queryLC = query->defineSubQuery("lc2pc");
  {
    queryLC.addToTableList( m_lc2pcTable.defaultTableName(), "lc2pc" );

    queryLC.addToOutputList("lc2pc."+LC2PCTable::columnNames::logConStr(),
                            "lcs");

    coral::AttributeList whereData;
    std::string whereString = // "cr." + CredentialsTable::columnNames::physConStr()
      std::string("physConStr ")
      + " = " + " lc2pc." + LC2PCTable::columnNames::physConStr();
    if ( !lcString.empty() )
    {
      whereData.extend( "lcString",
                        AttributeSpecification::typeNameForType<
                        LC2PCTable::columnTypes::logConStr>() );
      whereData[ "lcString" ].data< LC2PCTable::columnTypes::logConStr >() =
        lcString;
      if ( !whereString.empty() )
        whereString += " and ";
      whereString +=  "lc2pc."+ LC2PCTable::columnNames::logConStr() +
        " = :lcString";
    }
    queryLC.setCondition( whereString, whereData);
  }
  query->addToTableList("lc2pc");
#endif
  if ( !crWhereString.empty() )
    query->setCondition( crWhereString, crWhereData);

  ICursor &cursor=query->execute();

  while ( cursor.next() ) {
    const AttributeList &curr( cursor.currentRow() );
    //std::cout << " connection: '" << curr << std::endl;
    std::cout << std::left;
    std::cout << std::setw(10) << curr[ "lcs" ].data< std::string >() << "  ";
    std::cout << std::setw(30) << curr[ "physConStr" ].data< std::string >() << "  ";
    std::cout << std::setw(10) << curr[ "user" ].data< std::string >() << "  ";
    std::cout << std::setw(10) << curr[ "role" ].data< std::string >() << "  ";
    std::cout << std::setw(30) << curr[ "fqan" ].data< std::string >() << "  ";
    std::cout << std::setw(30) << curr[ "dn" ].data< std::string >() << "  ";
    std::cout << std::endl;
    /*[ columnNames::physConStr() ].data<
        columnTypes::physConStr>()
        << "' User: '" << curr[ columnNames::user() ].data<
        columnTypes::user>()
        << "' passwd: '" << curr[ columnNames::passwd() ].data<
        columnTypes::passwd>()
        << "' dbRole: '" << curr[ columnNames::dbRole() ].data<
        columnTypes::dbRole>()
        << "'" << std::endl;*/
  }
}

//----------------------------------------------------------------------------

const AuthenticationCredentials QueryMgr::getCredentials( const std::string& physConStr,
                                                          const std::string& dbRole_, const ICertificateData* cert )
{
  std::string dbRole( dbRole_ );
  if ( dbRole_.empty() )
    dbRole = IAuthenticationCredentials::defaultRole();

  if ( cert == 0 )
    return m_crTable.getCredentials( physConStr, dbRole );

  std::auto_ptr<IQuery> query( m_schema.newQuery() );

  query->addToTableList( m_crTable.defaultTableName(), "cr" );
  query->addToTableList( m_permTable.defaultTableName(), "perm" );

  query->addToOutputList("cr."+CredentialsTable::columnNames::user(),
                         "user");
  query->addToOutputList("cr."+CredentialsTable::columnNames::passwd(),
                         "password");

  coral::AttributeList whereData;
  std::string whereString=
    // join
    " cr."+CredentialsTable::columnNames::physConStr()+" = "
    +" perm."+PermissionsTable::columnNames::physConStr()+ " and "
    + " cr."+CredentialsTable::columnNames::dbRole()+" = "
    +" perm."+PermissionsTable::columnNames::dbRole()+ " and "
    // where role & physConstr
    + " cr."+CredentialsTable::columnNames::dbRole() + " = "
    + " :role  and "
    + " cr."+CredentialsTable::columnNames::physConStr() + " = "
    + " :physConStr ";

  whereData.extend( "role",
                    AttributeSpecification::typeNameForType<
                    CredentialsTable::columnTypes::dbRole>() );
  whereData[ "role" ].data< CredentialsTable::columnTypes::dbRole >() = dbRole;

  whereData.extend( "physConStr",
                    AttributeSpecification::typeNameForType<
                    CredentialsTable::columnTypes::physConStr>() );
  whereData[ "physConStr" ].data< CredentialsTable::columnTypes::physConStr >() = physConStr;


  // dn and fqans
  // if we got a certificate data, the credentials have to have a permisssion based on dn or voms,
  // otherwise we don't return it
  whereString += " and ( perm."+PermissionsTable::columnNames::distinguishedName()+ " !=\"\" or ";
  whereString += "  perm."+PermissionsTable::columnNames::vomsFQAN()+" != \"\" ) ";

  // empty dn in permission or matching dn
  whereString += " and ( (  perm."+PermissionsTable::columnNames::distinguishedName()+ "=\"\" or ";
  whereString += " perm."+PermissionsTable::columnNames::distinguishedName()+
    " = :dn ) ";
  whereData.extend( "dn",
                    AttributeSpecification::typeNameForType<
                    PermissionsTable::columnTypes::distinguishedName>() );
  whereData[ "dn" ].data< PermissionsTable::columnTypes::distinguishedName >() = cert->distinguishedName();
  if ( cert->distinguishedName().empty() )
    throw coral::Exception("Empty distinguished name is not allowed!", "QueryMgr::getCredentials",
                           "CoralAuthenticationService");

  whereString += " and ( perm."+PermissionsTable::columnNames::vomsFQAN()+"=\"\" ";
  int fqanCount = 0;
  if ( cert->FQANs().size() > 0 ) {
    whereString += " or perm."+PermissionsTable::columnNames::vomsFQAN()+" in ( ";
    std::vector< std::string >::const_iterator it=cert->FQANs().begin();
    if ( it->empty() )
      throw coral::Exception("Empty FQAN is not allowed!", "QueryMgr::getCredentials",
                             "CoralAuthenticationService");

    std::stringstream name;
    name << "fqan" << fqanCount;

    whereString += " :"+name.str()+" ";
    whereData.extend( name.str(),
                      AttributeSpecification::typeNameForType<
                      PermissionsTable::columnTypes::vomsFQAN>() );
    whereData[ name.str() ].data< PermissionsTable::columnTypes::vomsFQAN >() = *it;
    ++it;
    fqanCount++;

    while ( it != cert->FQANs().end() )
    {
      if ( it->empty() )
        throw coral::Exception("Empty FQAN is not allowed!", "QueryMgr::getCredentials",
                               "CoralAuthenticationService");
      // whereString += ", \""+ *it + "\" ";
      std::stringstream name2;
      name2 << "fqan" << fqanCount;

      whereString += ", :"+name2.str()+" ";
      whereData.extend( name2.str(),
                        AttributeSpecification::typeNameForType<
                        PermissionsTable::columnTypes::vomsFQAN>() );
      whereData[ name2.str() ].data< PermissionsTable::columnTypes::vomsFQAN >() = *it;

      ++it;
      fqanCount++;
    }
    whereString += " )";
  }
  whereString += "  ) ) ";

  query->setCondition( whereString, whereData);

  ICursor &cursor=query->execute();

  if ( ! cursor.next() ) {
    throw AuthenticationServiceException("no credentials found", "CredentialsTable::getCredentials",
                                         "CoralAuthentication");
  };

  AuthenticationCredentials credentials("CoralAuthentication");
  const AttributeList &curr( cursor.currentRow() );

  credentials.registerItem( IAuthenticationCredentials::userItem(),
                            curr[ "user" ].data<
                            CredentialsTable::columnTypes::user >() );
  credentials.registerItem( IAuthenticationCredentials::passwordItem(),
                            m_crTable.decryptPasswd( curr[ "password" ].data<
                                                     CredentialsTable::columnTypes::passwd >() ) );
  return credentials;
}
