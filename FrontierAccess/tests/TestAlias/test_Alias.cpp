#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IWebCacheControl.h"

#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"

#include "CoralCommon/Sleep.h"

#include <iostream>
#include <sstream>

void readWithAlias( std::auto_ptr<coral::ISessionProxy>& sessionR, const std::string& alias )
{
  std::cout << "__TEST Read with table alias '" << alias << "'" << std::endl;
  try {
    std::auto_ptr<coral::IQuery> query( sessionR->nominalSchema().newQuery() );

    if( alias.empty() )
    {
      query->addToTableList( "TEST" );
      query->addToOutputList( "VALUE" );
    }
    else
    {
      query->addToTableList( "TEST", alias );
      query->addToOutputList( alias + ".VALUE" );
    }

    std::string whereClause = "ID='1'";
    if ( alias != "" ) whereClause = alias + "." + whereClause;
    coral::AttributeList whereData;
    query->setCondition( whereClause, whereData );
    coral::ICursor& cursor = query->execute();
    int nRows = 0;
    while ( cursor.next() ) {
      nRows++;
      std::stringstream out;
      cursor.currentRow().toOutputStream( out );
      std::cout << "Row #" << nRows << " found: " << out.str() << std::endl;
    }
    std::cout << "Retrieved " << nRows << " rows" << std::endl;
  }
  catch ( std::exception& e ) {
    std::cerr << "Standard C++ exception caught: " << e.what() << std::endl;
  }
}

void readCmsAlias( std::auto_ptr<coral::ISessionProxy>& sessionR, const std::string& alias )
{
  std::cout << "__TEST Read with table alias '" << alias << "'" << std::endl;
  try
  {
    std::string treetablename = alias;
    std::auto_ptr<coral::IQuery> query( sessionR->nominalSchema().newQuery() );

    query->addToTableList( treetablename, "p1" );
    query->addToTableList( treetablename, "p2" );
    query->addToOutputList( "p1.tagid" );
    query->setRowCacheSize( 100 );

    coral::AttributeList bindData;

    bindData.extend( "nodelabel",typeid(std::string) );
    bindData["nodelabel"].data<std::string>()= "Node 2";

    bindData.extend( "tagid",typeid(unsigned int) );
    bindData["tagid"].data<unsigned int>()=0;

    query->setCondition( "p1.lft BETWEEN p2.lft AND p2.rgt AND p2.nodelabel = :nodelabel AND p1.tagid <> :tagid", bindData );

    coral::AttributeList qresult;
    qresult.extend("tagid", typeid(unsigned int));
    query->defineOutput(qresult);
    std::vector<unsigned int> leaftagids;
    leaftagids.reserve(100);
    coral::ICursor& cursor = query->execute();
    while( cursor.next() )
    {
      const coral::AttributeList& row = cursor.currentRow();
      leaftagids.push_back(row["tagid"].data<unsigned int>());
    }

    cursor.close();
  }
  catch ( std::exception& e )
  {
    std::cerr << "Standard C++ exception caught: " << e.what() << std::endl;
  }
}

int main( int argc, char* argv[] )
{

  bool skipWrite  = false;
  bool skipGrant  = false;
  bool readOracle = false;
  bool refresh    = false;

  if ( argc > 1 ) {
    for ( int iarg=1; iarg<argc; iarg++ ) {
      std::string theArg = std::string( argv[iarg] );
      if ( theArg == "skipWrite"  ) skipWrite  = true;
      if ( theArg == "skipGrant"  ) skipGrant  = true;
      if ( theArg == "readOracle" ) readOracle = true;
      if ( theArg == "refresh"    ) refresh    = true;
    }
  }

  try
  {
    coral::Context& ctx = coral::Context::instance();

    ctx.loadComponent( "CORAL/Services/ConnectionService" );
    ctx.loadComponent( "CORAL/Services/XMLAuthenticationService" );

    coral::IHandle<coral::IConnectionService>
      connSvc = ctx.query<coral::IConnectionService>( "CORAL/Services/ConnectionService" );

    connSvc->configuration().setAuthenticationService( "CORAL/Services/XMLAuthenticationService" );

    //    std::string connectW = "/coral/cooldev";
    //    std::string connectR = "/coral/CoolFrontier";
    std::string connectW = "Frontier-lcgspi";
    std::string connectR = "FrontierR-lcgspi";

    if ( readOracle )
      connectR = connectW;

    // 1. Write sample data and grant read privileges to public
    if ( ! skipWrite )
    {
      // COOL bug report due to an table alias error
      std::cout << "_TEST Write sample data" << std::endl;
      coral::AccessMode accessModeW = coral::Update;
      std::auto_ptr<coral::ISessionProxy>
        sessionW( connSvc->connect( connectW, accessModeW ) );
      sessionW->transaction().start( false );
      coral::ISchema& schema = sessionW->nominalSchema();

      schema.dropIfExistsTable( "TEST" );
      {
        coral::TableDescription description;
        description.setName( "TEST" );
        description.insertColumn
          ( "ID", coral::AttributeSpecification::typeNameForId
            ( typeid(std::string) ) );
        description.insertColumn
          ( "VALUE", coral::AttributeSpecification::typeNameForId
            ( typeid(std::string) ) );
        coral::ITable& table = schema.createTable( description );
        coral::AttributeList rowBuffer;
        table.dataEditor().rowBuffer( rowBuffer );
        rowBuffer["ID"].data<std::string>() = "1";
        rowBuffer["VALUE"].data<std::string>() = "Value for 1";
        table.dataEditor().insertRow( rowBuffer );
        rowBuffer["ID"].data<std::string>() = "2";
        rowBuffer["VALUE"].data<std::string>() = "Value for 2";
        table.dataEditor().insertRow( rowBuffer );
        if ( ! skipGrant )
          table.privilegeManager().grantToUser
            ( "PUBLIC", coral::ITablePrivilegeManager::Select );
      }

      // CMS table self-alias bug report, see: https://savannah.cern.ch/bugs/index.php?34576
      schema.dropIfExistsTable( "TAGTREE_TABLE" );
      {
        coral::TableDescription ttd;
        ttd.setName( "TAGTREE_TABLE" );
        ttd.insertColumn(         "nodeid",      coral::AttributeSpecification::typeNameForId( typeid(unsigned) ) );
        ttd.setPrimaryKey(        "nodeid" );

        ttd.insertColumn(         "nodelabel",   coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
        ttd.setNotNullConstraint( "nodelabel",   true );
        ttd.setUniqueConstraint(  "nodelabel" );

        ttd.insertColumn(         "lft",         coral::AttributeSpecification::typeNameForId( typeid(unsigned) ) );
        ttd.setNotNullConstraint( "lft",         true );

        ttd.insertColumn(         "rgt",         coral::AttributeSpecification::typeNameForId( typeid(unsigned) ) );
        ttd.setNotNullConstraint( "rgt",         true );

        ttd.insertColumn(         "parentid",    coral::AttributeSpecification::typeNameForId( typeid(unsigned) ) );
        ttd.setNotNullConstraint( "parentid",    true );

        ttd.insertColumn(         "tagid",       coral::AttributeSpecification::typeNameForId( typeid(unsigned) ) );
        ttd.insertColumn(         "globalsince", coral::AttributeSpecification::typeNameForId( typeid(unsigned long long ) ) );
        ttd.insertColumn(         "globaltill",  coral::AttributeSpecification::typeNameForId( typeid(unsigned long long ) ) );

        coral::ITable& tabttd = schema.createTable( ttd );

        coral::AttributeList rowBuffer;

        tabttd.dataEditor().rowBuffer( rowBuffer );

        rowBuffer["nodeid"   ].data<int>() = 1;
        rowBuffer["nodelabel"].data<std::string>() = "root";
        rowBuffer["lft"      ].data<int>() = 2;
        rowBuffer["rgt"      ].data<int>() = 3;
        rowBuffer["parentid" ].data<int>() = 0;
        tabttd.dataEditor().insertRow( rowBuffer );

        rowBuffer["nodeid"   ].data<int>() = 2;
        rowBuffer["nodelabel"].data<std::string>() = "node 2";
        rowBuffer["lft"      ].data<int>() = 0;
        rowBuffer["rgt"      ].data<int>() = 0;
        rowBuffer["parentid" ].data<int>() = 1;
        tabttd.dataEditor().insertRow( rowBuffer );

        rowBuffer["nodeid"   ].data<int>() = 3;
        rowBuffer["nodelabel"].data<std::string>() = "node 3";
        rowBuffer["lft"      ].data<int>() = 0;
        rowBuffer["rgt"      ].data<int>() = 0;
        rowBuffer["parentid" ].data<int>() = 1;
        tabttd.dataEditor().insertRow( rowBuffer );

        if ( ! skipGrant )
          tabttd.privilegeManager().grantToUser( "PUBLIC", coral::ITablePrivilegeManager::Select );
      }

      sessionW->transaction().commit();
    }

    if( refresh )
    {
      // Setup refresh
      coral::IWebCacheControl& webcache = connSvc->webCacheControl();
      // Set refresh for the whole schema, e.g. each query in this schema will skip Squid cache
      // NOTE! One must specify the physical connection string as the plugins don't know about the logical ones!!!
      webcache.refreshSchemaInfo( "http://atlasfrontier.cern.ch:8080/Frontier" );
    }

    // 2. Read sample data
    if ( readOracle )
      coral::sys::sleep(1);  // Workaround for ORA-01466

    std::cout << "_TEST Read sample data" << std::endl;

    coral::AccessMode accessModeR = coral::ReadOnly;
    std::auto_ptr<coral::ISessionProxy> sessionR( connSvc->connect( connectR, accessModeR ) );

    sessionR->transaction().start( true );
    readWithAlias( sessionR, "" );
    readWithAlias( sessionR, "t" );
    readWithAlias( sessionR, "T" );
    readCmsAlias( sessionR, "TAGTREE_TABLE" );
    sessionR->transaction().commit();

    std::cout << "_TEST Completed successfully" << std::endl;

  }

  catch ( std::exception& e ) {
    std::cerr << "Standard C++ exception caught: " << e.what() << std::endl;
    return 1;
  }

  catch ( ... ) {
    std::cerr << "Unknown exception caught" << std::endl;
    return 1;
  }

  return 0;

}
