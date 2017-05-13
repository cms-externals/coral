// Include files
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralCommon/Utilities.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/TableDescription.h"

namespace coral
{

  class GroupByTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( GroupByTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_GB_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      std::cout << "About to drop previous table" << std::endl;
      schema.dropIfExistsTable( T1 );

      std::cout << "Describing new table" << std::endl;
      coral::TableDescription description;
      description.setName( T1 );
      description.insertColumn( "ID",
                                coral::AttributeSpecification::typeNameForId( typeid(int) ) );
      description.setPrimaryKey( "ID" );
      description.insertColumn( "x",
                                coral::AttributeSpecification::typeNameForId( typeid(float) ) );
      description.setNotNullConstraint( "x" );
      description.insertColumn( "GrB",
                                coral::AttributeSpecification::typeNameForId( typeid(int) ) );

      std::cout << "About to create the table" << std::endl;
      coral::ITable& table = schema.createTable( description );

      coral::AttributeList rowBuffer;
      rowBuffer.extend<int>( "ID" );
      rowBuffer.extend<float>( "x" );
      rowBuffer.extend<int>( "GrB" );

      for ( int i = 0; i < 100; ++i )
      {
        rowBuffer["ID"].data<int>() = i;
        rowBuffer["GrB"].data<int>() = i%10;
        rowBuffer["x"].data<float>() = (float)( ( i%14 ) + 0.1 * ( i%17 ) );

        table.dataEditor().insertRow( rowBuffer );
      }

      session->transaction().commit();

      coral::sleepSeconds(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_Read()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_GB_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start( true );

      coral::ISchema& schema = session->nominalSchema();

      std::auto_ptr<coral::IQuery> query( schema.tableHandle(T1).newQuery() );

      coral::AttributeList rowBuffer;
      rowBuffer.extend<int>( "GrB" );
      rowBuffer.extend<float>( "RES" );

      query->addToOutputList( "GrB" );
      query->addToOrderList( "GrB" );
      query->addToOutputList( "AVG(x)", "RES" );
      query->groupBy( "GrB" );

      query->defineOutput( rowBuffer );

      coral::ICursor& cursor = query->execute();

      int i = 0;
      while ( cursor.next() ) {
        rowBuffer.toOutputStream( std::cout );
        std::cout << std::endl;
        if ( i != rowBuffer["GrB"].data<int>() )
          throw std::runtime_error( "Unexpected data" );
        ++i;
      }

      if ( i != 10 )
        throw std::runtime_error( "Unexpected number of rows returned" );

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_GB_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      schema.dropTable( T1 );

      session->transaction().commit();
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( GroupByTest );

}

CORALCPPUNITTEST_MAIN( GroupByTest )
