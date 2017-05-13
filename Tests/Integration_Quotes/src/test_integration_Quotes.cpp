#include <iostream>
#include <string>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/TableDescription.h"

#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"

namespace coral
{
  class QuotesTest;
}

//----------------------------------------------------------------------------

class coral::QuotesTest : public coral::CoralCppUnitDBTest
{
  CPPUNIT_TEST_SUITE( QuotesTest );
  CPPUNIT_TEST( test_setup01 );
  CPPUNIT_TEST( test_setup02 );
  CPPUNIT_TEST( test_run );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_setup01()
  {
    std::string T1 = BuildUniqueTableName( "QUOTES_T1" );
    // create the session
    std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );

    coral::TableDescription desc01;
    desc01.insertColumn( "ID", "string", 255, false );
    desc01.insertColumn( "Name1", "string", 255, false );
    desc01.setPrimaryKey( "ID" );
    desc01.setName(T1);

    session->transaction().start(false);

    session->nominalSchema().dropIfExistsTable( desc01.name() );
    // create table
    coral::ITable& table = session->nominalSchema().createTable( desc01 );
    // set permissions (for frontier)
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

    coral::AttributeList rowBuffer;
    table.dataEditor().rowBuffer( rowBuffer );
    rowBuffer["ID"].data<std::string>() = "1";
    rowBuffer["Name1"].data<std::string>() = "row#1";
    table.dataEditor().insertRow( rowBuffer );
    rowBuffer["ID"].data<std::string>() = "2";
    rowBuffer["Name1"].data<std::string>() = "row#2";
    table.dataEditor().insertRow( rowBuffer );
    rowBuffer["ID"].data<std::string>() = "3";
    rowBuffer["Name1"].data<std::string>() = "row#3";
    table.dataEditor().insertRow( rowBuffer );
    //  table.privilegeManager().grantToUser( "PUBLIC", coral::ITablePrivilegeManager::Select );

    session->transaction().commit();

    coral::sleepSeconds( 1 ); // Avoid ORA-01466
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_setup02()
  {
    std::string T2 = BuildUniqueTableName( "QUOTES_T2" );
    // create the session
    std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );

    coral::TableDescription desc02;
    desc02.insertColumn( "ID", "string", 255, false );
    desc02.insertColumn( "Name2", "string", 255, false );
    desc02.setPrimaryKey( "ID" );
    desc02.setName(T2);

    session->transaction().start( false ); // read-write

    session->nominalSchema().dropIfExistsTable( desc02.name() );
    // create table
    coral::ITable& table = session->nominalSchema().createTable( desc02 );
    // set permissions (for frontier)
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

    coral::AttributeList rowBuffer;
    table.dataEditor().rowBuffer( rowBuffer );
    rowBuffer["ID"].data<std::string>() = "1";
    rowBuffer["Name2"].data<std::string>() = "row#1";
    table.dataEditor().insertRow( rowBuffer );
    rowBuffer["ID"].data<std::string>() = "2";
    rowBuffer["Name2"].data<std::string>() = "row#2";
    table.dataEditor().insertRow( rowBuffer );
    rowBuffer["ID"].data<std::string>() = "3";
    rowBuffer["Name2"].data<std::string>() = "row#3";
    table.dataEditor().insertRow( rowBuffer );
    //  table.privilegeManager().grantToUser( "PUBLIC", coral::ITablePrivilegeManager::Select );


    session->transaction().commit();

    coral::sleepSeconds( 1 ); // Avoid ORA-01466
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_run()
  {
    std::string T1 = BuildUniqueTableName( "QUOTES_T1" );
    std::string T2 = BuildUniqueTableName( "QUOTES_T2" );
    // create the session
    std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRO(), coral::ReadOnly ) );

    session->transaction().start(true);
    // create the query
    std::auto_ptr<coral::IQuery> query( session->nominalSchema().newQuery() );

    unsigned int nExpRows = 3;

    query->addToOutputList( "t1.Name1" );
    query->addToOutputList( "t2.Name2" );
    query->addToOutputList( "Name1" );
    query->addToOutputList( "Name2" );
    query->addToTableList( T1, "t1" );
    query->addToTableList( T2, "t2" );

    std::string whereClause = "t1.ID = t2.ID";
    coral::AttributeList whereData;
    query->setCondition( whereClause, whereData );

    coral::ICursor& cursor = query->execute();

    unsigned int nRows = 0;
    std::cout << std::endl << "Query result:" << std::endl;
    while ( cursor.next() )
    {
      cursor.currentRow().toOutputStream( std::cout ) << std::endl;
      ++nRows;
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of rows fetched", nExpRows, nRows );

    session->transaction().commit();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  QuotesTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  ~QuotesTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void setUp()
  {

  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void tearDown(){}

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::QuotesTest );

//----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( QuotesTest )
