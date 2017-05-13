// Include files
#include <iostream>
#include <exception>
#include <stdexcept>
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Exception.h"
#include "CoralCommon/Utilities.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/TableDescription.h"

//----------------------------------------------------------------------------

namespace
{

  class ThreadBody
  {

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    ThreadBody( coral::ConnectionService& connsvc,
                int threadID,
                const std::string& tableprefix )
      : m_connscv( connsvc )
      , m_threadID( threadID )
      , m_tableprefix( tableprefix )
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    ~ThreadBody()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The thread body
    void operator()()
    {
      //std::cout << "Entering thread no " << m_threadID << std::endl;
      // Write data
      std::auto_ptr<coral::ISessionProxy> session( m_connscv.connect( "CORAL-Oracle-lcgnight/admin" ) );
      session->transaction().start();
      coral::ISchema& schema = session->nominalSchema();
      std::ostringstream osTableName;
      osTableName << m_tableprefix << m_threadID;
      const std::string tableName = osTableName.str();
      schema.dropIfExistsTable( tableName );
      coral::TableDescription description;
      description.setName( tableName );
      description.insertColumn( "I", "int" );
      description.insertColumn( "F", "float" );
      description.insertColumn( "D", "double" );
      description.setPrimaryKey( "I" );
      description.setNotNullConstraint( "F" );
      coral::ITable& table = schema.createTable( description );
      coral::AttributeList rowBuffer;
      rowBuffer.extend<int>( "I" );
      rowBuffer.extend<float>( "F" );
      rowBuffer.extend<double>( "D" );
      int& i = rowBuffer[0].data<int>();
      float& f = rowBuffer[1].data<float>();
      double& d = rowBuffer[2].data<double>();
      for ( int row01 = 0; row01 < 100; ++row01 )
      {
        i = row01;
        f = (float)( row01 + 0.001 * m_threadID );
        d = row01 + 0.000001 * m_threadID;
        table.dataEditor().insertRow( rowBuffer );
      }
      session->transaction().commit();
      coral::sleepSeconds(1);
      // Read back data
      session->transaction().start( true );
      std::auto_ptr<coral::IQuery> query( schema.tableHandle( tableName ).newQuery() );
      coral::AttributeList outputBuffer;
      outputBuffer.extend<double>( "RES" );
      query->addToOutputList( "F+D", "RES" );
      query->addToOrderList( "I" );
      query->defineOutput( outputBuffer );
      coral::ICursor& cursor = query->execute();
      int row02 = 0;
      double& res = outputBuffer[0].data<double>();
      while ( cursor.next() )
      {
        if ( ::fabs( res - ( 2* row02 + 0.001001 * m_threadID ) ) > 0.00001 )
          throw std::runtime_error( "Unexpected data" );
        ++row02;
      }
      if ( row02 != 100 )
        throw std::runtime_error( "Unexpected number of rows" );
      session->transaction().commit();
      // Clean up
      session->transaction().start();
      schema.dropIfExistsTable( tableName );
      session->transaction().commit();
      //std::cout << "Exiting thread no " << m_threadID << std::endl;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  private:

    coral::ConnectionService& m_connscv;
    int m_threadID;
    std::string m_tableprefix;

  };

}

//----------------------------------------------------------------------------

namespace coral
{

  class MultipleSessionsTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( MultipleSessionsTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_Read()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_MS_T_" );
      coral::ConnectionService connSvc;

#ifdef __APPLE__
      // Workaround for bug #81005 on MacOSX (related to lazy symbol binding)
      // Load the OracleAccess plugin in the main thread so that its symbols
      // are available to all threads (loading it in a thread causes BPT trap)
      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );
      session->transaction().start();
      session->transaction().commit();
#endif

      // Launch the parallel threads here that share the same connection...
      std::cout << std::endl << "__main: launch the threads" << std::endl;
      std::vector< ThreadBody* > threadBodies;
      for ( int i = 0; i < 10; ++i )
        threadBodies.push_back( new ThreadBody( connSvc, i, T1 ) );
      std::vector< boost::thread* > threads;
      for ( int i = 0; i < 10; ++i )
        threads.push_back( new boost::thread( *( threadBodies[i] ) ) );
      std::cout << "__main: join the threads" << std::endl;
      for ( int i = 0; i < 10; ++i )
        threads[i]->join();
      for ( int i = 0; i < 10; ++i )
      {
        delete threads[i];
        delete threadBodies[i];
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( MultipleSessionsTest );

}

//----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( MultipleSessionsTest )
