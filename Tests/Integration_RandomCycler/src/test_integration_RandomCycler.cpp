#include <iostream>
#include <string>
#include "CoralBase/boost_thread_headers.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/Blob.h"
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
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/TableDescription.h"

#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"

#define __RANDOM_CYCLER_THREADS 8

namespace coral
{
  class RandomCycler;
  class RandomCyclerThread;
}

//----------------------------------------------------------------------------

class coral::RandomCyclerThread : public coral::CoralCppUnitDBTest
{

public:

  RandomCyclerThread() {}

  ~RandomCyclerThread() {}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_r01(std::auto_ptr<coral::ISessionProxy>& session)
  {
    std::string T1 = BuildUniqueTableName( "RANDOM_T1" );
    // open a read only transaction
    session->transaction().start(true);

    coral::ISchema& schema = session->nominalSchema();
    coral::ITable& table = schema.tableHandle(T1);
    std::auto_ptr<coral::IQuery> query( table.newQuery() );

    query->addToOrderList( "id" );

    //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
    // coral::MessageStream::setMsgVerbosity( coral::Verbose );

    query->addToOutputList( "id" );
    query->addToOutputList( "content" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );

    query->defineOutput( rowBuffer );

    coral::ICursor& cursor = query->execute();

    unsigned long long counter = 0;
    while(cursor.next())
    {
      counter++;
      // compare the content
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'id'", rowBuffer["id"].data<unsigned long long>(), counter );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'content'", rowBuffer["content"].data<std::string>().compare("hello world"), 0 );
    }
    session->transaction().commit();

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of rows fetched", (unsigned long long)1000, counter );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_r02(std::auto_ptr<coral::ISessionProxy>& session)
  {
    std::string T1 = BuildUniqueTableName( "RANDOM_T1" );
    // open a read only transaction
    session->transaction().start(true);

    coral::ISchema& schema = session->nominalSchema();
    coral::ITable& table = schema.tableHandle(T1);
    std::auto_ptr<coral::IQuery> query( table.newQuery() );

    query->addToOrderList( "id" );

    //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
    // coral::MessageStream::setMsgVerbosity( coral::Verbose );

    /*
    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );

    query->defineOutput( rowBuffer );
    */

    coral::ICursor& cursor = query->execute();

    long long counter = 0;
    while(cursor.next())
    {
      counter++;
      const coral::AttributeList& rowBuffer = cursor.currentRow();
      // compare the content
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'id'", rowBuffer["id"].data<long long>(), counter );

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'content'", rowBuffer["content"].data<std::string>().compare("hello world"), 0 );
    }
    session->transaction().commit();

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of rows fetched", (long long)1000, counter );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_r03(std::auto_ptr<coral::ISessionProxy>& session)
  {
    std::string T2 = BuildUniqueTableName( "RANDOM_T2" );

    //std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRO(), coral::ReadOnly ) );
    // open a read only transaction
    session->transaction().start(true);

    coral::ISchema& schema = session->nominalSchema();
    coral::ITable& table = schema.tableHandle(T2);
    std::auto_ptr<coral::IQuery> query( table.newQuery() );

    //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
    // coral::MessageStream::setMsgVerbosity( coral::Verbose );

    query->addToOutputList( "id" );
    query->addToOutputList( "s01" );
    query->addToOutputList( "s02" );
    query->addToOutputList( "d01" );
    query->addToOutputList( "d02" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "s01", typeid(std::string) );
    rowBuffer.extend( "s02", typeid(std::string) );
    rowBuffer.extend( "d01", typeid(coral::Blob) );
    rowBuffer.extend( "d02", typeid(coral::Blob) );

    query->defineOutput( rowBuffer );
    query->addToOrderList( "id" );

    coral::ICursor& cursor = query->execute();

    unsigned long long counter = 0;
    while(cursor.next())
    {
      counter++;
      const coral::AttributeList& row = cursor.currentRow();
      // compare the content
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T2 in column 'id'", row["id"].data<unsigned long long>(), counter );
    }
    session->transaction().commit();

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of rows fetched", (unsigned long long)1000, counter );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void operator()()
  {
    std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRO(), coral::ReadOnly ) );

    for( size_t i = 0; i < 10; ++i )
    {
      coral::sleepSeconds(rand() % 5);
      // get a random test
      size_t rnumber = (rand() % 3);
      switch(rnumber)
      {
        case 0:
          test_r01( session );
        break;
        case 1:
          test_r02( session );
        break;
        case 2:
          test_r03( session );
        break;
      }
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};

//----------------------------------------------------------------------------

class coral::RandomCycler : public coral::CoralCppUnitDBTest
{
  CPPUNIT_TEST_SUITE( RandomCycler );
  CPPUNIT_TEST( test_run_threads );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  fillBlob(coral::Blob& blob, size_t maxc)
  {
    blob.resize( maxc + 1 );
    unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
    for ( size_t j = 0; j < maxc; ++j, ++p ) *p = rand() % 256;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  fillString(std::string& s, size_t maxc, size_t offset)
  {
    s.clear();

    size_t pos = 0;
    while(pos < maxc)
    {
      char c = ((pos + offset) % 256);
      s = s + c;
      pos++;
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_setup_T01()
  {
    size_t numberOfFilledRows = 1000;

    std::string T1 = BuildUniqueTableName( "RANDOM_T1" );
    // create the session
    std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );

    // open an update transaction
    session->transaction().start(false);
    // drop the table if it already exists
    session->nominalSchema().dropIfExistsTable(T1);

    coral::TableDescription description( T1 + "_description" );
    description.setName(T1);

    description.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(unsigned long long) ) );
    description.setPrimaryKey( "id" );
    description.insertColumn( "content", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );

    coral::ITable& table = session->nominalSchema().createTable( description );
    // set permissions (for frontier)
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

    coral::AttributeList rowBuffer;
    // see bug #74867
    // table.dataEditor().rowBuffer( rowBuffer );
    // define the attribute list manually

    rowBuffer.extend( "id", typeid(unsigned long long) );
    rowBuffer.extend( "content", typeid(std::string) );

    coral::IBulkOperation* ins = table.dataEditor().bulkInsert( rowBuffer, 100 );

    for(size_t i = 0; i < numberOfFilledRows; ++i )
    {
      rowBuffer["id"].data<unsigned long long>() = (i + 1);
      // all characters
      rowBuffer["content"].data<std::string>() = "hello world";
      // add row
      ins->processNextIteration();
    }
    ins->flush();

    delete ins;

    session->transaction().commit();

    coral::sleepSeconds( 4 ); // Avoid ORA-01466
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_setup_T02()
  {
    size_t numberOfFilledRows = 1000;

    std::string T2 = BuildUniqueTableName( "RANDOM_T2" );
    // create the session
    std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );

    // open an update transaction
    session->transaction().start(false);
    // drop the table if it already exists
    session->nominalSchema().dropIfExistsTable(T2);

    coral::TableDescription description( T2 + "_description" );
    description.setName(T2);

    description.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(unsigned long long) ) );
    description.setPrimaryKey( "id" );
    description.insertColumn( "s01", coral::AttributeSpecification::typeNameForId(typeid(std::string)), 255, false );
    description.insertColumn( "s02", coral::AttributeSpecification::typeNameForId(typeid(std::string)), 255, false );
    description.insertColumn( "d01", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
    description.insertColumn( "d02", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );

    coral::ITypeConverter& typec = session->typeConverter();

    std::set<std::string> types = typec.supportedSqlTypes();
    for(std::set<std::string>::iterator i = types.begin(); i != types.end(); i++ )
    {
      std::cout << *i << std::endl;
    }
    if(types.find("CLOB") != types.end())
    {
      //we can use a clob here
    }
    // create table
    coral::ITable& table = session->nominalSchema().createTable( description );
    // set permissions (for frontier)
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

    coral::AttributeList rowBuffer;
    // see bug #74867
    // table.dataEditor().rowBuffer( rowBuffer );
    // define the attribute list manually
    rowBuffer.extend( "id", typeid(unsigned long long) );
    rowBuffer.extend( "s01", typeid(std::string) );
    rowBuffer.extend( "s02", typeid(std::string) );
    rowBuffer.extend( "d01", typeid(coral::Blob) );
    rowBuffer.extend( "d02", typeid(coral::Blob) );
    // Get an bulk operator
    coral::IBulkOperation* ins = table.dataEditor().bulkInsert( rowBuffer, 100 );

    for(size_t i = 0; i < numberOfFilledRows; ++i )
    {
      rowBuffer["id"].data<unsigned long long>() = (i + 1);
      //all characters
      rowBuffer["s01"].data<std::string>() = "hello world";
      //all viewable characters
      fillString(rowBuffer["s02"].data<std::string>(), 94, 32);
      //fill a blob with random data
      fillBlob(rowBuffer["d01"].data<coral::Blob>(), 5632);
      //fill a blob with random data
      fillBlob(rowBuffer["d02"].data<coral::Blob>(), 5631);

      ins->processNextIteration();
    }
    ins->flush();

    delete ins;
    // done
    session->transaction().commit();

    coral::sleepSeconds( 4 ); // Avoid ORA-01466
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_run_threads()
  {
    std::vector<RandomCyclerThread*> tests;
    std::vector<boost::thread*> threads;
    // start the threads
    for( size_t i = 0; i < __RANDOM_CYCLER_THREADS; ++i )
    {
      RandomCyclerThread* test = new RandomCyclerThread;
      tests.push_back( test );
      threads.push_back( new boost::thread( *test ) );
    }
    // join all threads
    for( size_t i = 0; i < __RANDOM_CYCLER_THREADS; ++i )
    {
      threads[i]->join();
    }
    // clean up threads
    for( size_t i = 0; i < __RANDOM_CYCLER_THREADS; ++i )
    {
      delete threads[i];
      delete tests[i];
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  RandomCycler(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  ~RandomCycler(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void setUp()
  {
    test_setup_T01();
    test_setup_T02();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void tearDown(){}

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::RandomCycler );

//----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( RandomCycler )
