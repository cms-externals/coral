// Include files
#include <cstdlib>
//#include <limits>
//#include <map>
//#include <stdexcept>
#include "CoralBase/boost_datetime_headers.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/MessageStream.h"
//#include "CoralBase/../src/isNaN.h"
#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"
#include "CoralCommon/Utilities.h"
//#include "CoralKernel/Context.h"
//#include "RelationalAccess/ConnectionService.h"
//#include "RelationalAccess/IConnectionService.h"
//#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
//#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/TableDescription.h"

//-----------------------------------------------------------------------------

namespace coral
{

  /** @class CoralMiscellaneousBugsTest
   *
   *  Test for miscellaneous CORAL bugs filed in Savannah.
   *
   *  @author Andrea Valassi
   *  @date   2010-11-11
   */

  class CoralMiscellaneousBugsTest : public CoralCppUnitDBTest
  {

    CPPUNIT_TEST_SUITE( CoralMiscellaneousBugsTest );

    // Cursor::next() returns false for SQL errors on a single row
    CPPUNIT_TEST( test_bug78178 );

    // 'SELECT *' in queries on many tables with no columns specified
    CPPUNIT_TEST( test_bug78574 );

    // Improve exception handling when selecting the same column twice
    CPPUNIT_TEST( test_bug79765 );

    // Tests of the expression parser
    CPPUNIT_TEST( test_bug91075_select );
    CPPUNIT_TEST( test_bug91075_SELECT );
    CPPUNIT_TEST( test_bug91075_group );
    CPPUNIT_TEST( test_bug91075_GROUP );
    CPPUNIT_TEST( test_bug91075_systimestamp );
    CPPUNIT_TEST( test_bug91075_SYSTIMESTAMP );

    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Cursor::next() returns false for errors in SQL during row retrieval
    void test_bug78178()
    {
      if ( ! ::getenv( "ENABLE_TEST_BUG78178" ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG78178 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Readonly
      {
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        std::auto_ptr<coral::ISessionProxy>
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::auto_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          query->addToOutputList( "ID" );
          coral::AttributeList rowBuffer;
          rowBuffer.extend( "ID", typeid(std::string)); // ID is a string!
          query->defineOutput( rowBuffer );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::auto_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        query->addToOutputList( "ID" );
        coral::AttributeList rowBuffer;
        rowBuffer.extend( "ID", typeid(int)); // But ID is a string "Row #1"!
        query->defineOutput( rowBuffer );
        coral::ICursor& cursor = query->execute();
        try
        {
          bool next = cursor.next(); // should fail (not return false!)
          std::cout << "*** ICursor::next() returned "
                    << ( next ? "True" : "False" )
                    << "? This should throw!" << std::endl;
          if ( next ) std::cout << rowBuffer["ID"] << std::endl;
          CPPUNIT_FAIL( "ICursor::next() with wrong column type must throw!" );
        }
        catch( coral::Exception& e )
        {
          std::cout << "*** OK! Exception caught: " << e.what() << std::endl;
        }
        sessionRO->transaction().commit();
        sessionRO.reset();
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Plugins do not select '*' in many-table queries with no columns selected
    void test_bug78574()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG78574" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG78574 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Readonly
      {
        std::auto_ptr<coral::ISessionProxy>
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        sessionRO->transaction().start(true);
        std::string tableName1 = testTableName(1);
        std::string tableName2 = testTableName(2);
        std::string tableName3 = testTableName(3);
        {
          std::auto_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName1 );
          query->addToTableList( tableName2 );
          std::string whereClause = tableName1+".ID="+tableName2+".ID";
          coral::AttributeList dummyWhereData;
          query->setCondition( whereClause, dummyWhereData ); // join
          try
          {
            coral::ICursor& cursor = query->execute();
            std::cout << "*** Query2 executed? Should throw!" << std::endl;
            cursor.next();
            std::cout << "Retrieved: " << cursor.currentRow() << std::endl;
            CPPUNIT_FAIL( "'select *' join with same colum name should fail" );
          }
          catch( coral::Exception& e )
          {
            std::cout << "Exception caught as expected: "
                      << e.what() << std::endl;
          }
        }
        {
          std::auto_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName1 );
          query->addToTableList( tableName3 );
          std::string whereClause = tableName1+".ID="+tableName3+".ID3";
          coral::AttributeList dummyWhereData;
          query->setCondition( whereClause, dummyWhereData ); // join
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          std::cout << "Retrieved: " << cursor.currentRow() << std::endl;
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          std::cout << "Retrieved: " << cursor.currentRow() << std::endl;
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          //std::cout << "*** Query1 executed" << std::endl;
        }
        sessionRO->transaction().commit();
        sessionRO.reset();
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Improve exception message when selecting twice the same column
    void test_bug79765()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG79765" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG79765 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Readonly
      {
        std::auto_ptr<coral::ISessionProxy>
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        sessionRO->transaction().start(true);
        std::string tableName1 = testTableName(1);
        coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
        coral::MessageStream::setMsgVerbosity( coral::Verbose );
        {
          std::auto_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName1 );
          query->addToOutputList( "ID" );
          query->addToOutputList( "ID" );
          try
          {
            coral::ICursor& cursor = query->execute();
            std::cout << "*** Query2 executed? Should throw!" << std::endl;
            cursor.next();
            std::cout << "Retrieved: " << cursor.currentRow() << std::endl;
            CPPUNIT_FAIL( "selecting the same colum twice should fail" );
          }
          catch( coral::QueryException& e )
          {
            std::cout << "QueryException caught as expected: "
                      << e.what() << std::endl;
          }
          catch( coral::Exception& e )
          {
            std::cout << "QueryException expected, another Exception caught: "
                      << e.what() << std::endl;
            throw;
          }
        }
        coral::MessageStream::setMsgVerbosity( oldLvl );
        sessionRO->transaction().commit();
        sessionRO.reset();
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void test_bug91075_select() { _test_reservedWord("select"); }
    void test_bug91075_SELECT() { _test_reservedWord("SELECT"); }
    void test_bug91075_group()  { _test_reservedWord("group"); }
    void test_bug91075_GROUP()  { _test_reservedWord("GROUP"); }
    void test_bug91075_systimestamp()  { _test_reservedWord("systimestamp"); }
    void test_bug91075_SYSTIMESTAMP()  { _test_reservedWord("SYSTIMESTAMP"); }

    // Test for reserved words used as column names
    void _test_reservedWord( std::string colName ) // convert char* to string
    {
      //if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
      //     ( ! ::getenv( "ENABLE_TEST_BUG91075" ) ) )
      //{
      //  std::cout << "Skip test (ENABLE_TEST_BUG91075 not set)" << std::endl;
      //  return;
      //}
      coral::ConnectionService connSvc;
      std::string tableName0 = testTableName(0);
      coral::AttributeList rowBuffer;
      // Update
      {
        std::auto_ptr<coral::ISessionProxy>
          sessionRW( connSvc.connect( UrlRW(), coral::Update ) );
        sessionRW->transaction().start(false);
        sessionRW->nominalSchema().dropIfExistsTable(tableName0);
        coral::TableDescription desc;
        desc.setName(tableName0);
        desc.insertColumn("ID","string");
        desc.insertColumn(colName,"int");
        coral::ITable& table = sessionRW->nominalSchema().createTable(desc);
        table.dataEditor().rowBuffer( rowBuffer );
        rowBuffer["ID"].setValue<std::string>( "Row #1" );
        rowBuffer[colName].setValue<int>( 1 );
        table.dataEditor().insertRow( rowBuffer );
        rowBuffer["ID"].setValue<std::string>( "Row #2" );
        rowBuffer[colName].setValue<int>( 2 );
        table.dataEditor().insertRow( rowBuffer );
        table.privilegeManager().grantToPublic
          ( coral::ITablePrivilegeManager::Select );
        sessionRW->transaction().commit();
        sessionRW.reset();
        coral::sleepSeconds(1); // Avoid ORA-01466
      }
      // Readonly
      {
        std::auto_ptr<coral::ISessionProxy>
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        sessionRO->transaction().start(true);
        {
          std::auto_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName0 );
          query->addToOutputList( "ID" );
          query->addToOutputList( colName );
          query->defineOutput( rowBuffer );
          coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
          coral::MessageStream::setMsgVerbosity( coral::Debug );
          try
          {
            coral::ICursor& cursor = query->execute();
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "next1", true, cursor.next() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "data1", 1, cursor.currentRow()[colName].data<int>() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "next2", true, cursor.next() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "data2", 2, cursor.currentRow()[colName].data<int>() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "next3", false, cursor.next() );
          }
          catch( ... )
          {
            coral::MessageStream::setMsgVerbosity( oldLvl );
            if ( colName == "SYSTIMESTAMP" ) std::cout << "Cannot do anything about SYSTIMESTAMP: ignore the error" << std::endl;
            else throw;
          }
          coral::MessageStream::setMsgVerbosity( oldLvl );
        }
        {
          std::auto_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName0, "T" );
          query->addToOutputList( "T.ID" );
          query->addToOutputList( "T."+colName );
          query->defineOutput( rowBuffer );
          coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
          coral::MessageStream::setMsgVerbosity( coral::Debug );
          try
          {
            coral::ICursor& cursor = query->execute();
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "next1", true, cursor.next() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "data1", 1, cursor.currentRow()[colName].data<int>() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "next2", true, cursor.next() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "data2", 2, cursor.currentRow()[colName].data<int>() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "next3", false, cursor.next() );
          }
          catch( ... )
          {
            coral::MessageStream::setMsgVerbosity( oldLvl );
            //if ( colName == "SYSTIMESTAMP" ) std::cout << "Cannot do anything about SYSTIMESTAMP: ignore the error" << std::endl;
            //else throw;
            throw;
          }
          coral::MessageStream::setMsgVerbosity( oldLvl );
        }
        sessionRO->transaction().commit();
        sessionRO.reset();
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    CoralMiscellaneousBugsTest()
    {
      static bool first = true;
      if ( first ) s_ntests = 0;
      m_itest = ++s_ntests;
      //std::cout << "Creating test #" << m_itest << std::endl;
      //std::cout << "Total number of tests: " << s_ntests << std::endl;
      // INITIAL SETUP FOR ALL TESTS (DO IT ONLY ONCE)
      if ( first )
      {
        first = false;
        std::cout << "Creating FIRST test" << std::endl;
        createTestTables(3);
      }
    }

    ~CoralMiscellaneousBugsTest()
    {
      //std::cout << "Deleting test #" << m_itest << std::endl;
      if ( m_itest==s_ntests )
      {
        std::cout << "Deleting LAST test" << std::endl;
        // --- DROP THE RELEVANT TEST TABLES ---
        std::cout << "*** Drop the test tables" << std::endl;
        coral::ConnectionService connSvc;
        std::auto_ptr<coral::ISessionProxy>
          sessionRW( connSvc.connect( UrlRW(), coral::Update ) );
        std::string tableName1 = testTableName(1);
        std::string tableName2 = testTableName(2);
        std::string tableName3 = testTableName(3);
        sessionRW->transaction().start(false);
        sessionRW->nominalSchema().dropIfExistsTable(tableName1);
        sessionRW->nominalSchema().dropIfExistsTable(tableName2);
        sessionRW->nominalSchema().dropIfExistsTable(tableName3);
        sessionRW->transaction().commit();
        sessionRW.reset();
      }
    }

    void setUp()
    {
      std::cout << std::endl
                << "---------------------------------------"
                << "---------------------------------------"
                << std::endl;
      //if ( m_itest==1 )
      //  std::cout << "Setting up the FIRST test" << std::endl;
    }

    void tearDown()
    {
      //if ( m_itest==s_ntests )
      //  std::cout << "Tearing down the LAST test" << std::endl;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void createTestTables( int nTables=1 )
    {
      coral::ConnectionService connSvc;
      // --- CREATE THE RELEVANT TEST TABLES ---
      std::cout << "*** Create " << nTables << " test tables" << std::endl;
      std::auto_ptr<coral::ISessionProxy>
        sessionRW( connSvc.connect( UrlRW(), coral::Update ) );
      std::string tableName1 = testTableName(1);
      std::string tableName2 = testTableName(2);
      std::string tableName3 = testTableName(3);
      sessionRW->transaction().start(false);
      sessionRW->nominalSchema().dropIfExistsTable(tableName1);
      sessionRW->nominalSchema().dropIfExistsTable(tableName2);
      sessionRW->nominalSchema().dropIfExistsTable(tableName3);
      if ( nTables >= 1 )
      {
        coral::TableDescription desc;
        desc.setName(tableName1);
        desc.insertColumn("ID","string");
        desc.insertColumn("I","int");
        coral::ITable& table = sessionRW->nominalSchema().createTable(desc);
        coral::AttributeList rowBuffer;
        table.dataEditor().rowBuffer( rowBuffer );
        rowBuffer["ID"].setValue<std::string>( "Row #1" ); // not a literal
        rowBuffer["I"].setValue<int>( 1 );
        table.dataEditor().insertRow( rowBuffer );
        rowBuffer["ID"].setValue<std::string>( "Row #2" ); // not a literal
        rowBuffer["I"].setValue<int>( 2 );
        table.dataEditor().insertRow( rowBuffer );
        table.privilegeManager().grantToPublic
          ( coral::ITablePrivilegeManager::Select );
      }
      if ( nTables >= 2 )
      {
        coral::TableDescription desc;
        desc.setName(tableName2);
        desc.insertColumn("ID","string");
        desc.insertColumn("LL","long long");
        coral::ITable& table = sessionRW->nominalSchema().createTable(desc);
        coral::AttributeList rowBuffer;
        table.dataEditor().rowBuffer( rowBuffer );
        rowBuffer["ID"].setValue<std::string>( "Row #1" ); // not a literal
        rowBuffer["LL"].setValue<long long>( 9223372036854775807LL );
        table.dataEditor().insertRow( rowBuffer );
        rowBuffer["ID"].setValue<std::string>( "Row #2" ); // not a literal
        rowBuffer["LL"].setValue<long long>( 9223372036854775807LL );
        table.dataEditor().insertRow( rowBuffer );
        table.privilegeManager().grantToPublic
          ( coral::ITablePrivilegeManager::Select );
      }
      if ( nTables >= 3 )
      {
        coral::TableDescription desc;
        desc.setName(tableName3);
        desc.insertColumn("ID3","string");
        desc.insertColumn("LL3","long long");
        coral::ITable& table = sessionRW->nominalSchema().createTable(desc);
        coral::AttributeList rowBuffer;
        table.dataEditor().rowBuffer( rowBuffer );
        rowBuffer["ID3"].setValue<std::string>( "Row #1" ); // not a literal
        rowBuffer["LL3"].setValue<long long>( 9223372036854775807LL );
        table.dataEditor().insertRow( rowBuffer );
        rowBuffer["ID3"].setValue<std::string>( "Row #2" ); // not a literal
        rowBuffer["LL3"].setValue<long long>( 9223372036854775807LL );
        table.dataEditor().insertRow( rowBuffer );
        table.privilegeManager().grantToPublic
          ( coral::ITablePrivilegeManager::Select );
      }
      sessionRW->transaction().commit();
      sessionRW.reset();
      coral::sleepSeconds(1); // Avoid ORA-01466
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    const std::string testTableName( int iTable=1 )
    {
      if ( iTable==1 )
        return BuildUniqueTableName( "MIXEDBUGS1" ); // shared
      else if ( iTable==2 )
        return BuildUniqueTableName( "MIXEDBUGS2" ); // shared
      else if ( iTable==3 )
        return BuildUniqueTableName( "MIXEDBUGS3" ); // shared
      else
        return BuildUniqueTableName( "MIXEDBUGS0" ); // managed by each test
    }

  private:

    unsigned m_itest;
    static unsigned int s_ntests;

  };

  unsigned int CoralMiscellaneousBugsTest::s_ntests;

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  CPPUNIT_TEST_SUITE_REGISTRATION( CoralMiscellaneousBugsTest );

}

//-----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( CoralMiscellaneousBugsTest )
