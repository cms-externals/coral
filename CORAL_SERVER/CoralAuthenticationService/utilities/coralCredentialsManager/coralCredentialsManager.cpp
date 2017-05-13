#include <iostream>
#include "CoralCommon/CommandLine.h"
#include "CoralCommon/DatabaseServiceDescription.h"
#include "CoralKernel/Context.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProxy.h"

#include "../../src/PhysConTable.h"
#include "../../src/LogConTable.h"
#include "../../src/LC2PCTable.h"
#include "../../src/CredentialsTable.h"
#include "../../src/PermissionsTable.h"
#include "../../src/QueryMgr.h"
#include "../../src/XMLAuthenticationFileParser.h"
#include "../../src/XMLLookupFileParser.h"
#include "../../src/AuthenticationCredentialSet.h"
#include "../../src/Config.h"

#define DEBUG(out) do { coral::MessageStream log("coral_authentication_manager"); log << coral::Debug << out << coral::MessageStream::endmsg; } while (0)

using namespace coral;

//-----------------------------------------------------------------------

void initDatabase( ISchema & schema )
{
  PhysConTable::createTable( schema );
  LogConTable::createTable( schema );
  LC2PCTable::createTable( schema );
  CredentialsTable::createTable( schema );
  PermissionsTable::createTable( schema );
}

//-----------------------------------------------------------------------

void deleteDatabase( ISchema & schema )
{
  PermissionsTable::deleteTable( schema );
  CredentialsTable::deleteTable( schema );
  LC2PCTable::deleteTable( schema );
  PhysConTable::deleteTable( schema );
  LogConTable::deleteTable( schema );
}

//-----------------------------------------------------------------------

CommandLine& parseCommandLine( int argc, char** argv)
{
  std::vector<Option> secondaryOptions;

  coral::Option lcsPar("lcs");
  lcsPar.flag = "-l";
  lcsPar.helpEntry = "the logical connection string";
  lcsPar.type = coral::Option::STRING;
  secondaryOptions.push_back(lcsPar);
  //
  coral::Option pcsPar("pcs");
  pcsPar.flag = "-c";
  pcsPar.helpEntry = "the physical connection string";
  pcsPar.type = coral::Option::STRING;
  secondaryOptions.push_back(pcsPar);
  //
  coral::Option rolePar("role");
  rolePar.flag = "-r";
  rolePar.helpEntry ="the role associated to the credentials";
  rolePar.type = coral::Option::STRING;
  secondaryOptions.push_back(rolePar);
  //
  coral::Option userPar("username");
  userPar.flag = "-u";
  userPar.helpEntry = "the username of the database account";
  userPar.type = coral::Option::STRING;
  secondaryOptions.push_back(userPar);
  //
  coral::Option pwdPar("password");
  pwdPar.flag = "-p";
  pwdPar.helpEntry = "the password of the database account";
  pwdPar.type = coral::Option::STRING;
  secondaryOptions.push_back(pwdPar);
  //
  coral::Option mcPar("accessMode");
  mcPar.flag = "-mc";
  mcPar.helpEntry = "access mode of the connection";
  mcPar.type = coral::Option::STRING;
  secondaryOptions.push_back(mcPar);
  //
  coral::Option amPar("authenticationMechanism");
  amPar.flag = "-am";
  amPar.helpEntry = "authentication mechanism of the connection";
  amPar.type = coral::Option::STRING;
  secondaryOptions.push_back(amPar);
  //
  coral::Option authFilePar("authentication_file");
  authFilePar.flag = "-af";
  authFilePar.helpEntry = "authentication file name, for example authentication.xml";
  authFilePar.type = coral::Option::STRING;
  secondaryOptions.push_back(authFilePar);
  //
  coral::Option lookFilePar("lookup_file");
  lookFilePar.flag = "-lf";
  lookFilePar.helpEntry = "lookup file name, for example dblookup.xml";
  lookFilePar.type = coral::Option::STRING;
  secondaryOptions.push_back(lookFilePar);
  //
  coral::Option dnPar("distinguished_name");
  dnPar.flag = "-dn";
  dnPar.helpEntry = "the distinguished name of the certificate";
  dnPar.type = coral::Option::STRING;
  secondaryOptions.push_back(dnPar);
  //
  coral::Option fqanPar("voms_fqan");
  fqanPar.flag = "-fqan";
  fqanPar.helpEntry = "the fully qualified attribute name of the certificate";
  fqanPar.type = coral::Option::STRING;
  secondaryOptions.push_back(fqanPar);
  //


  std::vector<coral::Command> mainSet;
  //
  coral::Command addRep("add");
  addRep.flag = "-add";
  addRep.helpEntry = "adding a new connection";
  addRep.type = coral::Option::BOOLEAN;
  addRep.exclusive = true;
  addRep.addOption(lcsPar.name);
  addRep.addOption(pcsPar.name);
  addRep.addOption(rolePar.name);
  addRep.addOption(userPar.name);
  addRep.addOption(pwdPar.name);
  addRep.addOption(mcPar.name);
  addRep.addOption(amPar.name);
  mainSet.push_back(addRep);
  //
  coral::Command listRep("list");
  listRep.flag = "-ls";
  listRep.helpEntry = "listing the connection entries";
  listRep.type = coral::Option::BOOLEAN;
  listRep.exclusive = true;
  listRep.addOption(lcsPar.name);
  listRep.addOption(pcsPar.name);
  listRep.addOption(rolePar.name);
  listRep.addOption(userPar.name);
  listRep.addOption(mcPar.name);
  listRep.addOption(amPar.name);
  mainSet.push_back(listRep);
  //
  coral::Command delRep("delete");
  delRep.flag = "-del";
  delRep.helpEntry = "delete connection entries. Carefull, will delete everything which matches the filter!";
  delRep.type = coral::Option::BOOLEAN;
  delRep.exclusive = true;
  delRep.addOption(lcsPar.name);
  delRep.addOption(pcsPar.name);
  delRep.addOption(rolePar.name);
  delRep.addOption(userPar.name);
  mainSet.push_back(delRep);
  //
  coral::Command addPermRep("addPermission");
  addPermRep.flag = "-addPerm";
  addPermRep.helpEntry = "grant permission to use credentials to certificates with certain dn or fqan.";
  addPermRep.type = coral::Option::BOOLEAN;
  addPermRep.exclusive = true;
  addPermRep.addOption(lcsPar.name);
  addPermRep.addOption(pcsPar.name);
  addPermRep.addOption(rolePar.name);
  addPermRep.addOption(fqanPar.name);
  addPermRep.addOption(dnPar.name);
  mainSet.push_back(addPermRep);
  //
  coral::Command delPermRep("deletePermission");
  delPermRep.flag = "-delPerm";
  delPermRep.helpEntry = "delete permission entries. Carefull, will delete everything which matches the filter!";
  delPermRep.type = coral::Option::BOOLEAN;
  delPermRep.exclusive = true;
  delPermRep.addOption(pcsPar.name);
  delPermRep.addOption(rolePar.name);
  delPermRep.addOption(fqanPar.name);
  delPermRep.addOption(dnPar.name);
  mainSet.push_back(delPermRep);
  //
  coral::Command impRep("import");
  impRep.flag = "-imp";
  impRep.helpEntry = "Import available replicas and associated credential information from xml files,"
    " in the format supported by XMLLookupService and XMLAuthenticationService.";
  impRep.type = coral::Option::BOOLEAN;
  impRep.exclusive = true;
  impRep.addOption(authFilePar.name);
  impRep.addOption(lookFilePar.name);
  mainSet.push_back(impRep);
  //
  coral::Command initRep("init");
  initRep.flag = "-init";
  initRep.helpEntry = "init database with the necessary tables";
  initRep.type = coral::Option::BOOLEAN;
  initRep.exclusive = true;
  mainSet.push_back(initRep);

  static coral::CommandLine cmd(secondaryOptions,mainSet);
  try {
    cmd.parse(argc,argv);
  } catch ( Exception &e) {
    std::cout << "Error : " << e.what() << std::endl << std::endl;
    cmd.help(std::cout);
    exit(-1);
  };
  const std::map<std::string,std::string>& ops = cmd.userOptions();
  if(cmd.userCommand()==coral::CommandLine::helpOption().name || ops.size()==0) {
    cmd.help(std::cout);
    exit(0);
  }

  std::map<std::string,std::string>::const_iterator iO =
    ops.find(coral::CommandLine::helpOption().name);
  if(iO!=ops.end()) {
    cmd.help(cmd.userCommand(),std::cout);
    exit(0);
  }
  return cmd;
}

//-----------------------------------------------------------------------

std::string getOption( CommandLine& userOptions,
                       const std::string& Option )
{
  if ( userOptions.find( Option ) )
    return std::string( userOptions.optionValue( Option ) );
  return std::string("");
}

//-----------------------------------------------------------------------

void addConnection( ISchema& schema, CommandLine & userOptions )
{
  LogConTable lcTable( schema );
  PhysConTable pcTable( schema );
  LC2PCTable lc2pcTable (schema );
  CredentialsTable crTable( schema );
  PermissionsTable permTable( schema );

  std::string lcString = getOption( userOptions, "lcs" );
  std::string pcString = getOption( userOptions, "pcs" );
  std::string user = getOption( userOptions, "username" );
  std::string passwd = getOption( userOptions, "password" );
  std::string role = getOption( userOptions, "role" );
  std::string accessMode = getOption( userOptions, "accessMode" );
  std::string authMechanism = getOption( userOptions, "authenticationMechanism" );

  QueryMgr mgr( schema );
  mgr.addConnection( lcString, pcString, user, passwd, role, accessMode, authMechanism);

  /*
  AccessMode aMode=Update;
  if ( !accessMode.empty() ) {
    if (accessMode == "update" || accessMode == "Update" || accessMode == "UPDATE" )
        aMode = Update;
    else if (accessMode == "read-only" || accessMode == "Read-Only" || accessMode == "READ-ONLY")
       aMode = ReadOnly;
    else throw coral::Exception("access mode should either be 'update' or 'read-only'",
            "coral_authentication_manager","");
  }
  if ( !lcString.empty() && !pcString.empty() )
  {
    // add lc -> pc mapping
    if ( !lcTable.logConExists( lcString ) )
      lcTable.addLogCon( lcString );

    if (  !pcTable.physConExists( pcString ) )
      pcTable.addPhysCon( pcString );

    if ( !lc2pcTable.LC2PCExists( false, lcString, pcString, accessMode, authMechanism ) )
      lc2pcTable.addLC2PC( lcString, pcString, 0, aMode, authMechanism );
    // FIXME order, accessmode
  }

  if ( !user.empty() || !passwd.empty() ) {
    if ( pcString.empty() || user.empty() || passwd.empty() )
      throw Exception("to add credentials, pcs, user and password are necessary",
          "coral_authentication_manager","");
    // add credentials to pc
    if ( role.empty() )
      role = IAuthenticationCredentials::defaultRole();

    crTable.addCredentials( pcString, user, passwd, role );
    }*/
}

//-----------------------------------------------------------------------

void impConnections( ISchema& schema, CommandLine & userOptions )
{
  LogConTable lcTable( schema );
  PhysConTable pcTable( schema );
  LC2PCTable lc2pcTable (schema );
  CredentialsTable crTable( schema );
  PermissionsTable permTable( schema );

  std::string authFile = getOption( userOptions, "authentication_file" );
  std::string lookFile = getOption( userOptions, "lookup_file" );

  if ( !authFile.empty() ) {
    std::map< std::string, AuthenticationCredentialSet* > data;
    XMLAuthenticationFileParser parser("CoralAuthenticationService");

    if ( !parser.parse( authFile, data ) )
      throw Exception("Error parsing authentication file!", "coral_authentication_manager",
                      "CoralAuthenticationService" );

    for ( std::map< std::string, AuthenticationCredentialSet*>::iterator it=data.begin();
          it!=data.end(); ++it )
    {
      const std::string& connection=it->first;

      // add default role if exists
      if ( it->second->hasDefault() )
        crTable.addCredentials( connection,
                                it->second->credentials().valueForItem( IAuthenticationCredentials::userItem() ),
                                it->second->credentials().valueForItem( IAuthenticationCredentials::passwordItem() ),
                                IAuthenticationCredentials::defaultRole() );

      for ( std::map< std::string, AuthenticationCredentials*>::const_iterator crIt=
              it->second->roleMap().begin();
            crIt!=it->second->roleMap().end(); ++crIt)
      {
        crTable.addCredentials( connection,
                                crIt->second->valueForItem( IAuthenticationCredentials::userItem() ),
                                crIt->second->valueForItem( IAuthenticationCredentials::passwordItem() ),
                                crIt->first );
        DEBUG("adding auth: " << connection << ", " <<
              crIt->second->valueForItem( IAuthenticationCredentials::userItem() ) << ", "
              << crIt->second->valueForItem( IAuthenticationCredentials::passwordItem() )
              << crIt->first );
      };
      // FIXME free everything
    };
  };

  if ( !lookFile.empty() )
  {
    std::map< std::string, std::vector< DatabaseServiceDescription* > > data;
    XMLLookupFileParser parser("CoralAuthenticationService");


    if ( !parser.parse( lookFile, data ) )
      throw Exception("Error parsing lookup file!", "coral_authentication_manager",
                      "CoralAuthenticationService" );

    for ( std::map< std::string, std::vector< DatabaseServiceDescription* > >::iterator it = data.begin();
          it != data.end(); ++it )
    {
      const std::string& lcs = it->first;
      // add lc -> pc mapping
      if ( !lcTable.logConExists( lcs ) )
        lcTable.addLogCon( lcs );

      for ( std::vector< DatabaseServiceDescription* >::iterator dbIt = it->second.begin();
            dbIt != it->second.end(); ++dbIt )
      {
        const std::string& pcs = (**dbIt).connectionString();

        if (  !pcTable.physConExists( pcs ) )
          pcTable.addPhysCon( pcs );

        if ( !lc2pcTable.LC2PCExists( false, lcs, pcs,
                                      lc2pcTable.AccessMode2Str( (*dbIt)->accessMode()),
                                      (*dbIt)->authenticationMechanism() ) )
          lc2pcTable.addLC2PC( lcs, pcs, (*dbIt)->accessMode(), (*dbIt)->authenticationMechanism() );
        // FIXME order

      }

    }
    // FIXME free everything
  };
}

//-----------------------------------------------------------------------

void listConnection( ISchema& schema, CommandLine & userOptions )
{
  QueryMgr queryMgr( schema );

  std::string lcString = getOption( userOptions, "lcs" );
  std::string pcString = getOption( userOptions, "pcs" );
  std::string user = getOption( userOptions, "username" );
  std::string role = getOption( userOptions, "role");
  //std::string accessMode = getOption( userOptions, "accessMode" );
  //std::string authMechanism = getOption( userOptions, "authenticationMechanism" );
  queryMgr.queryConnections( lcString, pcString, user, role );
}

//-----------------------------------------------------------------------

void addPermission( ISchema& schema, CommandLine & userOptions )
{
  QueryMgr mgr( schema );

  std::string lcString = getOption( userOptions, "lcs" );
  std::string pcString = getOption( userOptions, "pcs" );
  std::string role     = getOption( userOptions, "role");
  std::string dn       = getOption( userOptions, "distinguished_name");
  std::string vomsFQAN = getOption( userOptions, "voms_fqan");

  mgr.addPermission( lcString, pcString, role, dn, vomsFQAN );
}

//-----------------------------------------------------------------------

void delPermission( ISchema& schema, CommandLine & userOptions )
{
  QueryMgr mgr( schema );

  std::string lcString = getOption( userOptions, "lcs" );
  std::string pcString = getOption( userOptions, "pcs" );
  std::string role     = getOption( userOptions, "role");
  std::string vomsFQAN = getOption( userOptions, "voms_fqan");
  std::string dn       = getOption( userOptions, "distinguished_name");

  int count=mgr.delPermission( lcString, pcString, role, dn, vomsFQAN );
  std::cout << "Removed " << count <<" permissions." << std::endl;
}

//-----------------------------------------------------------------------

void delConnections( ISchema& schema, CommandLine & userOptions )
{
  QueryMgr mgr( schema );

  std::string lcString = getOption( userOptions, "lcs" );
  std::string pcString = getOption( userOptions, "pcs" );
  std::string user = getOption( userOptions, "username" );
  std::string role = getOption( userOptions, "role");

  mgr.delConnection( lcString, pcString, user, role );
  /*
  if (!lcString.empty() ) {
      lc2pcTable.delLC2PC( lcString, pcString, accessMode, authMechanism);

      // if we deleted the last lc2pc, we also delete the lc
      if ( !lc2pcTable.LC2PCExists( true, lcString, "", "", "" ) )
        lcTable.delLogCon( lcString );

  }
   // if we got a user name, delete the credentials
   if ( !user.empty() ) {
      if ( pcString.empty() )
        throw Exception("to delete credentials, pcs and user are necessary",
            "coral_authentication_manager","");

      if ( role.empty() )
        role = IAuthenticationCredentials::defaultRole();

      crTable.delCredentials( pcString, user, role );
  };
  */
  /*
  long no=dbTable.delConnections( connectionString, user, role );
  std::cout << "Deleted " << no << " connections." << std::endl;
  */
}

//-----------------------------------------------------------------------

int main( int argc, char** argv )
{
  CommandLine & userOptions(
                            parseCommandLine( argc, argv ));

  coral::IConnectionService *connService = new coral::ConnectionService();
  bool sharing = true;
  if(!sharing) {
    coral::IConnectionServiceConfiguration& config = connService->configuration();
    config.disableConnectionSharing();
  }

  const std::string& serviceName = getConfig().dbConnectionString();
  coral::ISessionProxy *session = connService->connect( serviceName,
                                                        coral::Update );
  if ( !session )
    throw coral::Exception("Could not connect to " + serviceName,"main()","demo" );
  // start an update transaction
  session->transaction().start( false /*readonly*/ );

  ISchema &schema( session->nominalSchema() );

  if ( userOptions.userCommand() == "add" )
    addConnection( schema, userOptions );
  else if ( userOptions.userCommand() == "list" )
    listConnection( schema, userOptions );
  else if ( userOptions.userCommand() == "import" )
    impConnections( schema, userOptions );
  else if ( userOptions.userCommand() == "delete" )
    delConnections( schema, userOptions );
  else if ( userOptions.userCommand() == "addPermission" )
    addPermission( schema, userOptions );
  else if ( userOptions.userCommand() == "deletePermission" )
    delPermission( schema, userOptions );
  else if ( userOptions.userCommand() == "init" )
  {
    deleteDatabase( session->nominalSchema() );
    initDatabase( session->nominalSchema() );
  }
  else{
    userOptions.help( std::cout );
    return 0;
  };
#if 0
  ConnectionsTable dbTable( session->nominalSchema() );
  PermissionsTable permTable( session->nominalSchema() );

  dbTable.addConnection("connectionString", "user1", "passwd1", "role1");
  dbTable.addConnection("connectionString", "user1a", "passwd1a", "role1a");
  dbTable.addConnection("connectionString2", "user2", "passwd2", "role1");

  dbTable.listCredentials("connectionString");

  permTable.addPermission(dbTable.getId("connectionString","role1"), "/Atlas/lcg", "nix");
  permTable.addPermission(dbTable.getId("connectionString2","role1"), "/Atlas/lcg", "nix");
#endif
#if 0
  // first drop the table if it exists, then create it
  session->nominalSchema().dropIfExistsTable( tableDesc.name() );
  coral::ITable& table = session->nominalSchema().createTable( tableDesc );

  // fill the table
  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );
  for ( size_t i = 0; i<20; i++)
  {
    std::cout << "Filling row " << i << std::endl;
    rowBuffer["id"].data<unsigned int>() = i;
    rowBuffer["name"].data<std::string>() = "name";
    table.dataEditor().insertRow( rowBuffer );
  };
#endif

  session->transaction().commit();
}
