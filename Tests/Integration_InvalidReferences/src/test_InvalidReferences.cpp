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
#include "CoralKernel/Context.h"
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

  /** @class CoralInvalidReferencesTest
   *
   *  Test for CORAL crashes observed due to the use of invalid references.
   *
   *  @author Andrea Valassi
   *  @date   2010-11-11
   */

  class CoralInvalidReferencesTest : public CoralCppUnitDBTest
  {

    CPPUNIT_TEST_SUITE( CoralInvalidReferencesTest );

    // Crashes due to stale session references inside the plugins
    CPPUNIT_TEST( test_bug73834 );
    CPPUNIT_TEST( test_bug79983 ); // formerly test_bug73834b
    CPPUNIT_TEST( test_bug83601 ); // formerly test_bug76501
#ifdef __linux
    CPPUNIT_TEST( test_bug80065 ); // formerly test_bug73834c
    CPPUNIT_TEST( test_bug80097 ); // formerly test_bug80065b
    CPPUNIT_TEST( test_bug80098 );
    CPPUNIT_TEST( test_bug80174 ); // formerly test_bug80098b
    CPPUNIT_TEST( test_bug80178 ); // formerly test_bug80174b
    CPPUNIT_TEST( test_bug81112 ); // formerly test_bug80178b
#endif

    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Crash if execute query in session deleted by user in same thread
    void test_bug73834()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG73834" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG73834 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        std::unique_ptr<coral::ISessionProxy>
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        std::cout << "*** Disconnect RO" << std::endl;
        sessionRO->transaction().commit();
        sessionRO.reset();
        std::cout << "*** Disconnected, now execute query2" << std::endl;
        try
        {
          query->execute(); // crashes in CORAL_2_3_14 (bug #73834)
          std::cout << "*** Query2 executed? This should throw!" << std::endl;
          CPPUNIT_FAIL( "IQuery::execute in deleted session must throw!" );
        }
        catch( coral::QueryException& e ) // derived from SessionException
        {
          std::cout << "SessionException expected, QueryException caught: "
                    << e.what() << std::endl;
          throw;
        }
        catch( coral::SessionException& e ) // catch this AFTER QueryException
        {
          std::cout << "SessionException caught as expected: "
                    << e.what() << std::endl;
        }
        catch( coral::Exception& e )
        {
          std::cout << "SessionException expected, another Exception caught: "
                    << e.what() << std::endl;
          throw;
        }
      }
      catch(...)
      {
        coral::MessageStream::setMsgVerbosity( oldLvl );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Crash if increment cursor connected to session explicity deleted by user
    void test_bug79983() // formerly test_bug73834b
    {
      if ( ! ::getenv( "ENABLE_TEST_BUG79983" ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG79983 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        std::unique_ptr<coral::ISessionProxy>
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 done; create/execute query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        coral::ICursor& cursor = query->execute();
        std::cout << "*** Got cursor from query2, disconnect RO" << std::endl;
        sessionRO->transaction().commit();
        sessionRO.reset();
        std::cout << "*** Disconnected, now increment cursor" << std::endl;
        try
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Cursor next ok? This should throw!" << std::endl;
          CPPUNIT_FAIL( "ICursor::next() in deleted session must throw!" );
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
      catch(...)
      {
        coral::MessageStream::setMsgVerbosity( oldLvl );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Crash if delete query after user deletes session in same thread
    void test_bug83601()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG76501" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG76501 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure connection service as in ObjectConsistency test (bug #76501)
      // [Note: maybe one call is enough as disable/enable <==> timeout =/> 0?]
      // - enable CORAL connection pool cleanup (crash does NOT show up in ST)
      // - conn timeout = 10 (crash does NOT show up if this is 0!)
      connSvc.configuration().enablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(10);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        std::unique_ptr<coral::ISessionProxy>
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        std::cout << "*** Execute query2 without fetching rows" << std::endl;
        query->execute();
        std::cout << "*** Disconnect RO" << std::endl;
        sessionRO->transaction().commit();
        sessionRO.reset();
        std::cout << "*** Disconnected, now delete query2" << std::endl;
        query.reset(); // crashes in ~CORAL_2_3_16-pre11 (bug #83601)
        std::cout << "*** Succesfully deleted query2" << std::endl;
      }
      catch(...)
      {
        coral::MessageStream::setMsgVerbosity( oldLvl );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // A simple class to delete a session in another thread after a time delay
    class SessionDeleterThread
    {
    public:
      // ctor
      SessionDeleterThread( coral::ISessionProxy* session, int delay )
        : m_session( session ), m_delay( delay ) {}
      // dtor
      ~SessionDeleterThread() {}
      // operator()
      void operator()()
      {
        std::string time1 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
        std::cout << "__SessionDeleterThread::operator() @"
                  << time1 << " sleep " << m_delay << "s" << std::endl;
        coral::sleepSeconds(m_delay);
        std::string time2 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
        std::cout << "__SessionDeleterThread::operator() @"
                  << time2 << " slept " << m_delay << "s" << std::endl;
        if ( m_session ) delete m_session;
        std::string time3 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
        std::cout << "__SessionDeleterThread::operator() @"
                  << time3 << " session deleted" << std::endl;
      }
    private:
      coral::ISessionProxy* m_session;
      int m_delay;
    };

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef __linux
    // Crash if execute query in session deleted by user in another thread
    void test_bug80065()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG80065" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG80065 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      std::unique_ptr<boost::thread> pThread;
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        coral::ISessionProxy*
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        std::cout << "*** Disconnect RO" << std::endl;
        pThread.reset( new boost::thread(SessionDeleterThread(sessionRO,2)) );
        coral::sleepSeconds(1);
        std::cout << "*** Launched deleter, now execute query2" << std::endl;
        try
        {
          ::setenv( "CORAL_ORA_TEST_BUG80065_SLEEP5S", "1", 1 );
          query->execute(); // crashes in CORAL_2_3_16-pre5 (bug #80065)
          std::cout << "*** Query2 executed? This should throw!" << std::endl;
          CPPUNIT_FAIL( "IQuery::execute in deleted session must throw!" );
        }
        catch( coral::QueryException& e ) // a SessionException
        {
          std::cout << "ConnectionNotActiveException expected, "
                    << "QueryException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::TransactionException& e ) // a SessionException
        {
          std::cout << "ConnectionNotActiveException expected, "
                    << "TransactionException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::ConnectionNotActiveException& e ) // a SessionException
        {
          // Session dtor started and marked isUserSessionActive = false;
          // but dtor did not yet invalidateSession, so query proceeds;
          // query however asks for Session::schema() and throws this
          // ConnectionNotActiveException because isUserSessionActive = false
          std::cout << "ConnectionNotActiveException caught as expected: "
                    << e.what() << std::endl;
        }
        catch( coral::SessionException& e ) // catch this AFTER the others...
        {
          std::cout << "ConnectionNotActiveException expected, "
                    << "another SessionException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::Exception& e )
        {
          std::cout << "ConnectionNotActiveException expected, "
                    << "another Exception caught: " << e.what()
                    << std::endl;
          throw;
        }
      }
      catch(...)
      {
        if ( pThread.get() ) pThread->join();
        pThread.reset();
        coral::MessageStream::setMsgVerbosity( oldLvl );
        ::unsetenv( "CORAL_ORA_TEST_BUG80065_SLEEP5S" );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
      ::unsetenv( "CORAL_ORA_TEST_BUG80065_SLEEP5S" );
    }
#endif

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef __linux
    // Crash if execute query in session deleted by user in another thread
    void test_bug80097()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG80097" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG80097 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      std::unique_ptr<boost::thread> pThread;
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        coral::ISessionProxy*
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        std::cout << "*** Disconnect RO" << std::endl;
        pThread.reset( new boost::thread(SessionDeleterThread(sessionRO,2)) );
        coral::sleepSeconds(1);
        std::cout << "*** Launched deleter, now execute query2" << std::endl;
        try
        {
          ::setenv( "CORAL_ORA_TEST_BUG80097_SLEEP5S", "1", 1 );
          query->execute(); // crashes in CORAL_2_3_16-pre5 (bug #80065)
          std::cout << "*** Query2 executed? This should throw!" << std::endl;
          CPPUNIT_FAIL( "IQuery::execute in deleted session must throw!" );
        }
        catch( coral::QueryException& e ) // a SessionException
        {
          // OracleStatement::defineOutput throws after fixing bug #80099
          std::cout << "QueryException caught as expected: "
                    << e.what() << std::endl;
        }
        catch( coral::SessionException& e ) // catch this AFTER the others...
        {
          std::cout << "QueryException expected, "
                    << "another SessionException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::Exception& e )
        {
          std::cout << "QueryException expected, "
                    << "another Exception caught: " << e.what()
                    << std::endl;
          throw;
        }
      }
      catch(...)
      {
        if ( pThread.get() ) pThread->join();
        pThread.reset();
        coral::MessageStream::setMsgVerbosity( oldLvl );
        ::unsetenv( "CORAL_ORA_TEST_BUG80097_SLEEP5S" );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
      ::unsetenv( "CORAL_ORA_TEST_BUG80097_SLEEP5S" );
    }
#endif

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef __linux
    // Crash if execute query in session deleted by user in another thread
    void test_bug80098()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG80098" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG80098 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      std::unique_ptr<boost::thread> pThread;
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        coral::ISessionProxy*
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        std::cout << "*** Disconnect RO" << std::endl;
        pThread.reset( new boost::thread(SessionDeleterThread(sessionRO,2)) );
        coral::sleepSeconds(1);
        std::cout << "*** Launched deleter, now execute query2" << std::endl;
        try
        {
          ::setenv( "CORAL_ORA_TEST_BUG80098_SLEEP5S", "1", 1 );
          query->execute(); // crashes in CORAL_2_3_16-pre5 (bug #80065)
          std::cout << "*** Query2 executed? This should throw!" << std::endl;
          CPPUNIT_FAIL( "IQuery::execute in deleted session must throw!" );
        }
        catch( coral::QueryException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "QueryException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::TransactionException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "TransactionException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::ConnectionNotActiveException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "ConnectionNotActiveException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::SessionException& e ) // catch this AFTER the others...
        {
          // Session::transaction finds isUserSessionActive = true, but
          // m_transaction was deleted even if endUserSession was not called
          std::cout << "Generic SessionException caught as expected: "
                    << e.what() << std::endl;
        }
        catch( coral::Exception& e )
        {
          std::cout << "Generic SessionException expected, "
                    << "another Exception caught: " << e.what()
                    << std::endl;
          throw;
        }
      }
      catch(...)
      {
        if ( pThread.get() ) pThread->join();
        pThread.reset();
        coral::MessageStream::setMsgVerbosity( oldLvl );
        ::unsetenv( "CORAL_ORA_TEST_BUG80098_SLEEP5S" );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
      ::unsetenv( "CORAL_ORA_TEST_BUG80098_SLEEP5S" );
    }
#endif

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef __linux
    // Crash if execute query in session deleted by user in another thread
    void test_bug80174()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG80174" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG80174 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      std::unique_ptr<boost::thread> pThread;
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        coral::ISessionProxy*
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        std::cout << "*** Disconnect RO" << std::endl;
        pThread.reset( new boost::thread(SessionDeleterThread(sessionRO,2)) );
        coral::sleepSeconds(1);
        std::cout << "*** Launched deleter, now execute query2" << std::endl;
        try
        {
          ::setenv( "CORAL_ORA_TEST_BUG80174_SLEEP5S", "1", 1 );
          query->execute(); // crashes in CORAL_2_3_16-pre7? (bug #80174)
          std::cout << "*** Query2 executed? This should throw!" << std::endl;
          CPPUNIT_FAIL( "IQuery::execute in deleted session must throw!" );
        }
        catch( coral::QueryException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "QueryException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::TransactionException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "TransactionException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::ConnectionNotActiveException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "ConnectionNotActiveException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::SessionException& e ) // catch this AFTER the others...
        {
          // Thrown by Transacton::isActive (endUserSession was already called)
          std::cout << "Generic SessionException caught as expected: "
                    << e.what() << std::endl;
        }
        catch( coral::Exception& e )
        {
          std::cout << "Generic SessionException expected, "
                    << "another Exception caught: " << e.what()
                    << std::endl;
          throw;
        }
      }
      catch(...)
      {
        if ( pThread.get() ) pThread->join();
        pThread.reset();
        coral::MessageStream::setMsgVerbosity( oldLvl );
        ::unsetenv( "CORAL_ORA_TEST_BUG80174_SLEEP5S" );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
      ::unsetenv( "CORAL_ORA_TEST_BUG80174_SLEEP5S" );
    }
#endif

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef __linux
    // Crash if execute query in session deleted by user in another thread
    void test_bug80178()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG80178" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG80178 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      std::unique_ptr<boost::thread> pThread;
      try
      {
        // Readonly
        std::cout << "*** Connect RO to " << UrlRO() << std::endl;
        coral::ISessionProxy*
          sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->nominalSchema().newQuery() );
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->nominalSchema().newQuery() );
        query->addToTableList( tableName );
        std::cout << "*** Disconnect RO" << std::endl;
        pThread.reset( new boost::thread(SessionDeleterThread(sessionRO,2)) );
        coral::sleepSeconds(1);
        std::cout << "*** Launched deleter, now execute query2" << std::endl;
        try
        {
          ::setenv( "CORAL_ORA_TEST_BUG80178_SLEEP5S", "1", 1 );
          query->execute(); // crashes in CORAL_2_3_16-pre7?
          std::cout << "*** Query2 executed? This should throw!" << std::endl;
          CPPUNIT_FAIL( "IQuery::execute in deleted session must throw!" );
        }
        catch( coral::QueryException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "QueryException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::TransactionException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "TransactionException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::ConnectionNotActiveException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "ConnectionNotActiveException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::SessionException& e ) // catch this AFTER the others...
        {
          // Thrown by Schema::tableHandle (endUserSession was already called)
          std::cout << "Generic SessionException caught as expected: "
                    << e.what() << std::endl;
        }
        catch( coral::Exception& e )
        {
          std::cout << "Generic SessionException expected, "
                    << "another Exception caught: " << e.what()
                    << std::endl;
          throw;
        }
      }
      catch(...)
      {
        if ( pThread.get() ) pThread->join();
        pThread.reset();
        coral::MessageStream::setMsgVerbosity( oldLvl );
        ::unsetenv( "CORAL_ORA_TEST_BUG80178_SLEEP5S" );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
      ::unsetenv( "CORAL_ORA_TEST_BUG80178_SLEEP5S" );
    }
#endif

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef __linux
    // Crash if execute query (on non-default schema) in session deleted by user in another thread
    void test_bug81112()
    {
      if ( ( UrlRO() != BuildUrl( "Oracle", true ) ) &&
           ( ! ::getenv( "ENABLE_TEST_BUG81112" ) ) )
      {
        std::cout << "Skip test (ENABLE_TEST_BUG81112 not set)" << std::endl;
        return;
      }
      coral::ConnectionService connSvc;
      // Configure the connection service (see bug #71449)
      // - disable the CORAL connection pool cleanup
      // - conn timeout = 0: "idle" connections become immediately "expired"
      connSvc.configuration().disablePoolAutomaticCleanUp();
      connSvc.configuration().setConnectionTimeOut(0);
      // Then increase verbosity to keep track of connecting and disconnecting
      coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
      coral::MessageStream::setMsgVerbosity( coral::Info );
      std::unique_ptr<boost::thread> pThread;
      try
      {
        std::string schemaName;
        {
          // Readonly
          std::cout << "*** Connect RO to " << UrlRO() << std::endl;
          coral::ISessionProxy*
            sessionRO( connSvc.connect( UrlRO(), coral::ReadOnly ) );
          std::cout << "*** Connected; get schema name" << std::endl;
          schemaName = sessionRO->nominalSchema().schemaName(); // schema1
          std::cout << "*** Disconnect RO" << std::endl;
          delete sessionRO;
        }
        // Readonly
        std::string connectString = UrlRO() + "2"; // nominal is schema2
        std::cout << "*** Connect RO to " << connectString << std::endl;
        coral::ISessionProxy*
          sessionRO( connSvc.connect( connectString, coral::ReadOnly ) );
        std::cout << "*** Connected; create and execute query1" << std::endl;
        sessionRO->transaction().start(true);
        std::string tableName = testTableName();
        {
          std::unique_ptr<coral::IQuery>
            query( sessionRO->schema( schemaName ).newQuery() ); // schema1
          query->addToTableList( tableName );
          coral::ICursor& cursor = query->execute();
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row1 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "Row2 exists", true, cursor.next() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "No Row3", false, cursor.next() );
          std::cout << "*** Query1 executed; create query2" << std::endl;
        }
        std::unique_ptr<coral::IQuery>
          query( sessionRO->schema( schemaName ).newQuery() ); // schema1
        query->addToTableList( tableName );
        std::cout << "*** Disconnect RO" << std::endl;
        pThread.reset( new boost::thread(SessionDeleterThread(sessionRO,2)) );
        coral::sleepSeconds(1);
        std::cout << "*** Launched deleter, now execute query2" << std::endl;
        try
        {
          ::setenv( "CORAL_ORA_TEST_BUG81112_SLEEP5S", "1", 1 );
          query->execute(); // crashes in CORAL_2_3_16-pre7?
          std::cout << "*** Query2 executed? This should throw!" << std::endl;
          CPPUNIT_FAIL( "IQuery::execute in deleted session must throw!" );
        }
        catch( coral::QueryException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "QueryException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::TransactionException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "TransactionException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::ConnectionNotActiveException& e ) // a SessionException
        {
          std::cout << "Generic SessionException expected, "
                    << "ConnectionNotActiveException caught: " << e.what()
                    << std::endl;
          throw;
        }
        catch( coral::SessionException& e ) // catch this AFTER the others...
        {
          // Thrown by Schema::tableHandle (endUserSession was already called)
          std::cout << "Generic SessionException caught as expected: "
                    << e.what() << std::endl;
        }
        catch( coral::Exception& e )
        {
          std::cout << "Generic SessionException expected, "
                    << "another Exception caught: " << e.what()
                    << std::endl;
          throw;
        }
      }
      catch(...)
      {
        if ( pThread.get() ) pThread->join();
        pThread.reset();
        coral::MessageStream::setMsgVerbosity( oldLvl );
        ::unsetenv( "CORAL_ORA_TEST_BUG81112_SLEEP5S" );
        throw;
      }
      coral::MessageStream::setMsgVerbosity( oldLvl );
      ::unsetenv( "CORAL_ORA_TEST_BUG81112_SLEEP5S" );
    }
#endif

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    CoralInvalidReferencesTest()
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
        coral::ConnectionService connSvc;
        // --- CONFIGURE THE CONNECTION SERVICE ---
        // First load services quietly
        try { connSvc.connect( "" ); } catch(...) {}
        coral::Context& ctx = coral::Context::instance();
        ctx.loadComponent( "CORAL/RelationalPlugins/oracle" );
        ctx.loadComponent( "CORAL/RelationalPlugins/frontier" );
        ctx.loadComponent( "CORAL/RelationalPlugins/mysql" );
        ctx.loadComponent( "CORAL/RelationalPlugins/sqlite" );
        ctx.loadComponent( "CORAL/Services/XMLAuthenticationService" );
        // Configure the connection service (see bug #71449)
        // - disable the CORAL connection pool cleanup
        // - conn timeout = 0: "idle" connections become immediately "expired"
        connSvc.configuration().disablePoolAutomaticCleanUp();
        connSvc.configuration().setConnectionTimeOut(0);
        // Then increase verbosity to keep track of connecting/disconnecting
        coral::MsgLevel oldLvl = coral::MessageStream::msgVerbosity();
        coral::MessageStream::setMsgVerbosity( coral::Info );
        // --- TEST VERBOSE CONNECT/DISCONNECT
        std::cout << "*** Test verbose connect/disconnect" << std::endl;
        try
        {
          const std::string connectString = UrlRW();
          coral::AccessMode accessMode = coral::Update;
          std::cout << "*** Connect RW to " << connectString << std::endl;
          std::unique_ptr<coral::ISessionProxy>
            session( connSvc.connect( connectString, accessMode ) );
          std::cout << "*** Connected, now disconnect" << std::endl;
          session.reset();
          std::cout << "*** Disconnected" << std::endl;
        }
        catch(...)
        {
          coral::MessageStream::setMsgVerbosity( oldLvl );
          throw;
        }
        coral::MessageStream::setMsgVerbosity( oldLvl );
        // --- CREATE THE RELEVANT TEST TABLES ---
        std::cout << "*** Create the test tables" << std::endl;
        //std::cout << "*** Connect RW" << std::endl;
        std::unique_ptr<coral::ISessionProxy>
          sessionRW( connSvc.connect( UrlRW(), coral::Update ) );
        //std::cout << "*** Connected; create tables" << std::endl;
        std::string tableName1 = testTableName();
        sessionRW->transaction().start(false);
        sessionRW->nominalSchema().dropIfExistsTable(tableName1);
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
        sessionRW->transaction().commit();
        //std::cout << "*** Disconnect RW" << std::endl;
        sessionRW.reset();
        coral::sleepSeconds(1); // Avoid ORA-01466
      }
    }

    ~CoralInvalidReferencesTest()
    {
      //std::cout << "Deleting test #" << m_itest << std::endl;
      if ( m_itest==s_ntests )
      {
        std::cout << "Deleting LAST test" << std::endl;
        // --- DROP THE RELEVANT TEST TABLES ---
        std::cout << "*** Drop the test tables" << std::endl;
        //std::cout << "*** Connect RW" << std::endl;
        coral::ConnectionService connSvc;
        std::unique_ptr<coral::ISessionProxy>
          sessionRW( connSvc.connect( UrlRW(), coral::Update ) );
        //std::cout << "*** Connected; drop tables" << std::endl;
        std::string tableName1 = testTableName();
        sessionRW->transaction().start(false);
        sessionRW->nominalSchema().dropIfExistsTable(tableName1);
        //std::cout << "*** Disconnect RW" << std::endl;
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

    const std::string testTableName()
    {
      return BuildUniqueTableName( "INVALIDREF" );
    }

  private:

    unsigned m_itest;
    static unsigned int s_ntests;

  };

  unsigned int CoralInvalidReferencesTest::s_ntests;

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  CPPUNIT_TEST_SUITE_REGISTRATION( CoralInvalidReferencesTest );

}

//-----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( CoralInvalidReferencesTest )
