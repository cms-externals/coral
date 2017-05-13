#include <iostream>
#include <stdexcept>
#include <string>
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Exception.h"
#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/IHandle.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/TableDescription.h"

#define LOG( msg ){ coral::MessageStream myMsg("OBJCONSIST"); myMsg << coral::Always << "********** " << msg << " **********" << coral::MessageStream::endmsg; }

namespace coral
{
  class ObjectConsistencyTest;
}

//----------------------------------------------------------------------------

class coral::ObjectConsistencyTest : public coral::CoralCppUnitDBTest
{
  CPPUNIT_TEST_SUITE( ObjectConsistencyTest );
  CPPUNIT_TEST( test_run01 );
  CPPUNIT_TEST( test_run02 );
  CPPUNIT_TEST( test_run03 );
  CPPUNIT_TEST( test_run04 );
  CPPUNIT_TEST( test_transaction01 );
  CPPUNIT_TEST( test_transaction02 );
  CPPUNIT_TEST( test_threads );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_run01()
  {
    LOG("TEST01 START");
    std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );
    {
      coral::ISessionProxy* session( connSvc().connect( UrlRO(), coral::ReadOnly ) );
      // open a read only transaction
      LOG("TEST01 ** transaction start try **");
      session->transaction().start(true);
      LOG("TEST01 ** transaction start done **");

      LOG("TEST01 ** nominal schema try **");
      coral::ISchema& schema = session->nominalSchema();
      LOG("TEST01 ** nominal schema done **");

      LOG("TEST01 ** table handle try **");
      coral::ITable& table = schema.tableHandle(T1);
      LOG("TEST01 ** table handle done **");

      LOG("TEST01 ** new query try **");
      coral::IQuery* query = table.newQuery();
      LOG("TEST01 ** new query done **");

      query->addToOrderList( "id" );

      //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
      // coral::MessageStream::setMsgVerbosity( coral::Verbose );

      query->addToOutputList( "id" );
      query->addToOutputList( "content" );

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

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of rows fetched", (unsigned long long)10, counter );

      LOG("TEST01 DELETE SESSION");
      delete session;
      LOG("TEST01 DELETE QUERY");
      delete query;
    }
    LOG("TEST01 DONE");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_run02()
  {
    LOG("TEST02 START");

    std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );
    {
      coral::ISessionProxy* session( connSvc().connect( UrlRO(), coral::ReadOnly ) );
      // open a read only transaction
      session->transaction().start(true);

      coral::ISchema& schema = session->nominalSchema();
      coral::ITable& table = schema.tableHandle(T1);
      coral::IQuery* query = table.newQuery();

      query->addToOrderList( "id" );

      //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
      // coral::MessageStream::setMsgVerbosity( coral::Verbose );

      query->addToOutputList( "id" );
      query->addToOutputList( "content" );

      query->defineOutput( rowBuffer );

      LOG("TEST02 DELETE SESSION");
      delete session;

      try
      {
        std::cout << "About to crash?" << std::endl;
        query->execute();
        std::cout << "Did not crash?" << std::endl;
        // Here should happen an exception of not throw one
        throw std::exception();
      }
      catch ( coral::Exception& e )
      {
        LOG("TEST02 EXCEPTION");
        LOG( e.what() );
      }

      LOG("TEST02 DELETE QUERY");
      delete query;
    }
    LOG("TEST02 DONE");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_run03()
  {
    LOG("TEST03 START");

    std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );
    {
      coral::ISessionProxy* session( connSvc().connect( UrlRO(), coral::ReadOnly ) );
      // open a read only transaction
      session->transaction().start(true);

      coral::ISchema& schema = session->nominalSchema();
      coral::ITable& table = schema.tableHandle(T1);
      coral::IQuery* query = table.newQuery();

      query->addToOrderList( "id" );

      //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
      // coral::MessageStream::setMsgVerbosity( coral::Verbose );

      query->addToOutputList( "id" );
      query->addToOutputList( "content" );

      query->defineOutput( rowBuffer );

      coral::ICursor& cursor = query->execute();

      LOG("TEST03 DELETE SESSION");
      delete session;

      try
      {
        unsigned long long counter = 0;
        while(cursor.next())
        {
          counter++;
          // compare the content
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'id'", rowBuffer["id"].data<unsigned long long>(), counter );

          CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'content'", rowBuffer["content"].data<std::string>().compare("hello world"), 0 );
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of rows fetched", (unsigned long long)0, counter );
        // Here should happen an exception if not throw one
        throw std::exception();
      }
      catch ( coral::Exception& e )
      {
        LOG("TEST03 EXCEPTION");
        LOG( e.what() );
      }
      LOG("TEST03 DELETE QUERY");
      delete query;
    }
    LOG("TEST03 DONE");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_run04()
  {
    LOG("TEST04 START");

    std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );
    {
      coral::ISessionProxy* session( connSvc().connect( UrlRO(), coral::ReadOnly ) );
      // open a read only transaction
      session->transaction().start(true);

      coral::ISchema& schema = session->nominalSchema();
      coral::ITable& table = schema.tableHandle(T1);
      coral::IQuery* query = table.newQuery();

      query->addToOrderList( "id" );

      //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
      // coral::MessageStream::setMsgVerbosity( coral::Verbose );

      query->addToOutputList( "id" );
      query->addToOutputList( "content" );

      query->defineOutput( rowBuffer );

      coral::ICursor& cursor = query->execute();

      unsigned long long counter = 0;
      while(cursor.next() && counter < 5)
      {
        counter++;
        // compare the content
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'id'", rowBuffer["id"].data<unsigned long long>(), counter );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "value missmatch in T1 in column 'content'", rowBuffer["content"].data<std::string>().compare("hello world"), 0 );
      }

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of rows fetched", (unsigned long long)5, counter );
      // Here should happen an exception if not throw one

      LOG("TEST04 DELETE SESSION");
      delete session;

      LOG("TEST04 DELETE QUERY");
      delete query;
    }
    LOG("TEST04 DONE");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_transaction01()
  {
    LOG("Transaction01 START");

    std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );
    {
      coral::ISessionProxy* session( connSvc().connect( UrlRO(), coral::ReadOnly ) );

      // open a read only transaction
      session->transaction().start(true);

      coral::ISchema& schema = session->nominalSchema();
      coral::ITable& table = schema.tableHandle(T1);
      coral::IQuery* query = table.newQuery();

      query->addToOrderList( "id" );

      //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
      // coral::MessageStream::setMsgVerbosity( coral::Verbose );

      query->addToOutputList( "id" );
      query->addToOutputList( "content" );

      query->defineOutput( rowBuffer );



      // commit transaction
      session->transaction().commit();

      try
      {
        query->execute();
        // Here should happen an exception if not throw one
        throw std::exception();
      }
      catch ( coral::TransactionNotActiveException& e )
      {
        LOG("Transaction01 EXCEPTION");
        LOG( e.what() );
      }
      catch ( coral::Exception& e )
      {
        LOG("Transaction01 EXCEPTION");
        LOG( e.what() );
      }
      LOG("Transaction01 DELETE QUERY");
      delete query;
      delete session;
    }
    LOG("Transaction01 DONE");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  // AV: This is essentially a test for bug #75094 (OracleAccess::Cursor::next
  // returns false if transaction is not active, while it should throw)
  // Duplicate of another test in PyCoralBasicTest.test011_bug75094
  void test_transaction02()
  {
    LOG("Transaction02 START");

    std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );

    coral::AttributeList rowBuffer;
    rowBuffer.extend( "id", typeid(unsigned long long));
    rowBuffer.extend( "content", typeid(std::string) );
    {
      coral::ISessionProxy* session( connSvc().connect( UrlRO(), coral::ReadOnly ) );

      // open a read only transaction
      session->transaction().start(true);

      coral::ISchema& schema = session->nominalSchema();
      coral::ITable& table = schema.tableHandle(T1);
      coral::IQuery* query = table.newQuery();

      query->addToOrderList( "id" );

      //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
      // coral::MessageStream::setMsgVerbosity( coral::Verbose );

      query->addToOutputList( "id" );
      query->addToOutputList( "content" );

      query->defineOutput( rowBuffer );


      coral::ICursor& cursor = query->execute();

      // commit transaction
      session->transaction().commit();

      try
      {
        while(cursor.next())
        {

        }
        // Here should happen an exception if not throw one
        throw std::exception();
      }
      catch ( coral::TransactionNotActiveException& e )
      {
        LOG("Transaction02 EXCEPTION");
        LOG( e.what() );
      }
      catch ( coral::Exception& e )
      {
        LOG("Transaction02 EXCEPTION");
        LOG( e.what() );
      }
      LOG("Transaction02 DELETE QUERY");
      delete query;
      delete session;
    }
    LOG("Transaction02 DONE");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  class ObjectConsistencyThread : public coral::CoralCppUnitDBTest
  {

  public:

    ObjectConsistencyThread( coral::ISessionProxy* session ) 
    {
      if ( !s_session ) s_session=session;
    }

    ~ObjectConsistencyThread() {}

    void operator()()
    {
      LOG("TESTTH START");
      std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );

      coral::AttributeList rowBuffer;
      rowBuffer.extend( "id", typeid(unsigned long long));
      rowBuffer.extend( "content", typeid(std::string) );

      //sleep( 1 ); // triggers a crash (bug #83706?)
      try
      {
        coral::IQuery* query = 0;
        {
          boost::mutex::scoped_lock lock( s_mutex );
          if ( !s_session ) 
          {
            errorsInThreads( true );
            return;
          }
          LOG("TESTTH GET SCHEMA");
          coral::ISchema& schema = s_session->nominalSchema();
          LOG("TESTTH GOT SCHEMA");
          coral::ITable& table = schema.tableHandle(T1);
          query = table.newQuery();
        }

        query->addToOrderList( "id" );
        query->addToOutputList( "id" );
        query->addToOutputList( "content" );
        query->defineOutput( rowBuffer );
        sleep( 1 );

        // Might throw an exception
        coral::ICursor& cursor = query->execute();
        // If everything is ok continue
        sleep( 1 );
        // Now the session is for sure deleted
        while(cursor.next())
        {
        }
        // The test should fail if an exception was not thrown
        errorsInThreads( true );
      }
      catch ( coral::Exception& e )
      {
        LOG("TESTTH EXCEPTION");
        LOG( e.what() );
      }
    }

    static int errorsInThreads( bool error = false )
    {
      static int s_errors = 0;
      if ( error ) s_errors++;
      return s_errors;
    }

    static void deleteSession()
    {
      boost::mutex::scoped_lock lock( s_mutex );
      LOG("TESTTH DELETE SESSION");
      if ( s_session ) delete s_session;
      s_session = 0;
      LOG("TESTTH DELETED SESSION");
    }

    static boost::mutex s_mutex;
    static coral::ISessionProxy* s_session;

  };

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_threads()
  {
    coral::ISessionProxy*
      session( connSvc().connect( UrlRO(), coral::ReadOnly ) );

    // open a read only transaction
    LOG("TESTTH ** transaction start try **");
    session->transaction().start(true);
    LOG("TESTTH ** transaction start done **");

    // start the threads
#define __THREADS 20
    std::vector<ObjectConsistencyThread*> tests;
    std::vector<boost::thread*> threads;
    for( size_t i = 0; i < __THREADS; ++i )
    {
      ObjectConsistencyThread* test = new ObjectConsistencyThread( session );
      tests.push_back( test );
      threads.push_back( new boost::thread( *test ) );
    }

    sleep(1);
    LOG("TESTTH ** DELETE SESSION **");
    //delete session;
    ObjectConsistencyThread::deleteSession();
    LOG("TESTTH ** DELETED SESSION **");

    // join all threads
    for( size_t i = 0; i < __THREADS; ++i )
    {
      LOG("TESTTH ** JOIN THREAD " << i << " **" );
      threads[i]->join();
      LOG("TESTTH ** JOINED THREAD " << i << " **" );
    }
    // clean up threads
    for( size_t i = 0; i < __THREADS; ++i )
    {
      LOG("TESTTH ** DELETE THREAD " << i << " **" );
      delete threads[i];
      delete tests[i];
      LOG("TESTTH ** DELETED THREAD " << i << " **" );
    }

    if ( ObjectConsistencyThread::errorsInThreads() )
    {
      LOG( "TESTTH ** ERROR IN "
           << ObjectConsistencyThread::errorsInThreads() << " THREADS" );
      throw std::runtime_error( "CORAL exception not thrown in >=1 threads" );
    }

  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  ObjectConsistencyTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  ~ObjectConsistencyTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void setUp()
  {
    size_t numberOfFilledRows = 10;

    std::string T1 = BuildUniqueTableName( "OBJCONSIST_T1" );
    // create the session
    std::auto_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );
    // open an update transaction
    session->transaction().start(false);
    // drop the table if it already exists
    coral::ISchema& schema = session->nominalSchema();

    schema.dropIfExistsTable(T1);

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

  void tearDown(){}

};


boost::mutex coral::ObjectConsistencyTest::ObjectConsistencyThread::s_mutex;

coral::ISessionProxy* coral::ObjectConsistencyTest::ObjectConsistencyThread::s_session;

CPPUNIT_TEST_SUITE_REGISTRATION( coral::ObjectConsistencyTest );

//----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( ObjectConsistencyTest )
