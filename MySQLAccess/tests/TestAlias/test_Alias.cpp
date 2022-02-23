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
#include "RelationalAccess/ConnectionService.h"

#include <iostream>
#include <sstream>
#include <memory>

void readWithAlias( std::unique_ptr<coral::ISessionProxy>& sessionR, const std::string& alias )
{
  std::cout << "__TEST Read with table alias '" << alias << "'" << std::endl;
  try {
    std::unique_ptr<coral::IQuery> query( sessionR->nominalSchema().newQuery() );
    query->addToTableList( "TEST", alias );

    if( alias.empty() )
      query->addToOutputList( "VALUE" );
    else
      query->addToOutputList( alias + ".VALUE" );

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

int main( int argc, char* argv[] )
{

  bool skipWrite  = false;
  bool skipGrant  = false;
  bool readtoo    = false;

  if ( argc > 1 )
  {
    for ( int iarg=1; iarg<argc; iarg++ )
    {
      std::string theArg = std::string( argv[iarg] );
      if ( theArg == "skipWrite" ) skipWrite = true;
      if ( theArg == "skipGrant" ) skipGrant = true;
      if ( theArg == "readtoo"   ) readtoo = true;
    }
  }

  try
  {
    coral::ConnectionService connSvc;

    std::string connectW = "/coral/cooldev";
    std::string connectR = "/coral/cooldevr";

    if ( readtoo )
      connectR = connectW;

    // 1. Write sample data and grant read privileges to public
    if ( ! skipWrite ) {
      std::cout << "_TEST Write sample data" << std::endl;
      coral::AccessMode accessModeW = coral::Update;
      std::unique_ptr<coral::ISessionProxy>
        sessionW( connSvc.connect( connectW, accessModeW ) );
      sessionW->transaction().start( false );
      coral::ISchema& schema = sessionW->nominalSchema();
      schema.dropIfExistsTable( "TEST" );
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
      sessionW->transaction().commit();
    }

    // 2. Read sample data

    std::cout << "_TEST Read sample data" << std::endl;
    coral::AccessMode accessModeR = coral::ReadOnly;
    std::unique_ptr<coral::ISessionProxy>
      sessionR( connSvc.connect( connectR, accessModeR ) );
    sessionR->transaction().start( true );
    readWithAlias( sessionR, "" );
    readWithAlias( sessionR, "t" );
    readWithAlias( sessionR, "T" );
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
