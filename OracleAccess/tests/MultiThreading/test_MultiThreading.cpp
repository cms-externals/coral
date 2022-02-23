// Include files
#include <iostream>
#include <exception>
#include <stdexcept>
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Exception.h"
#include "CoralBase/boost_thread_headers.h"
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

    ThreadBody( coral::ISchema& schema, int threadID, const std::string& tableprefix )
    : m_schema( schema )
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
      std::cout << "TableCreateAndQueryThread no " << m_threadID << " about to create and fill a table" << std::endl;

      // The table name
      std::ostringstream osTableName;
      osTableName << m_tableprefix << m_threadID;
      const std::string tableName = osTableName.str();

      m_schema.dropIfExistsTable( tableName );

      coral::TableDescription description;
      description.setName( tableName );
      description.insertColumn( "I", "int" );
      description.insertColumn( "F", "float" );
      description.insertColumn( "D", "double" );
      description.setPrimaryKey( "I" );
      description.setNotNullConstraint( "F" );

      coral::ITable& table = m_schema.createTable( description );
      coral::AttributeList rowBuffer;
      rowBuffer.extend<int>( "I" );
      rowBuffer.extend<float>( "F" );
      rowBuffer.extend<double>( "D" );

      int& i = rowBuffer[0].data<int>();
      float& f = rowBuffer[1].data<float>();
      double& d = rowBuffer[2].data<double>();

      for ( int row01 = 0; row01 < 100; ++row01 ) {
        i = row01;
        f = (float)( row01 + 0.001 * m_threadID );
        d = row01 + 0.000001 * m_threadID;
        table.dataEditor().insertRow( rowBuffer );
      }

      std::cout << "TableCreateAndQueryThread no " << m_threadID << " about to perform a query" << std::endl;

      std::unique_ptr<coral::IQuery> query( table.newQuery() );

      coral::AttributeList outputBuffer;
      outputBuffer.extend<double>( "RES" );

      query->addToOutputList( "F+D", "RES" );
      query->addToOrderList( "I" );
      query->defineOutput( outputBuffer );

      coral::ICursor& cursor = query->execute();
      int row02 = 0;
      double& res = outputBuffer[0].data<double>();
      while ( cursor.next() ) {
        if ( ::fabs( res - ( 2* row02 + 0.001001 * m_threadID ) ) > 0.00001 )
          throw std::runtime_error( "Unexpected data" );
        ++row02;
      }

      if ( row02 != 100 )
        throw std::runtime_error( "Unexpected number of rows" );

      m_schema.dropTable( tableName );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  private:

    coral::ISchema& m_schema;

    int m_threadID;

    std::string m_tableprefix;

  };

}

namespace coral
{

  class MultiThreadingTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( MultiThreadingTest );
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
      std::string T1 = BuildUniqueTableName( "ORA_UT_MT_T_" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      // Launch the parallel threads here that share the same connection...
      std::vector< ThreadBody* > threadBodies;

      for ( int i = 0; i < 10; ++i )
        threadBodies.push_back( new ThreadBody( schema, i, T1 ) );

      std::vector< boost::thread* > threads;
      for ( int i = 0; i < 10; ++i )
        threads.push_back( new boost::thread( *( threadBodies[i] ) ) );

      for ( int i = 0; i < 10; ++i )
        threads[i]->join();

      for ( int i = 0; i < 10; ++i )
      {
        delete threads[i];
        delete threadBodies[i];
      }

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( MultiThreadingTest );

}

CORALCPPUNITTEST_MAIN( MultiThreadingTest )
