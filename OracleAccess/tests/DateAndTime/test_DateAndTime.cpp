// Include files
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/TableDescription.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "CoralCommon/Utilities.h"

namespace coral
{

  class DateAndTimeTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( DateAndTimeTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_DAT_T1" );

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
      description.insertColumn( "TheDate",
                                coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
      description.insertColumn( "TheTime",
                                coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ),
                                6 );

      std::cout << "About to create the table" << std::endl;
      coral::ITable& table = schema.createTable( description );

      coral::AttributeList rowBuffer;
      table.dataEditor().rowBuffer( rowBuffer );

      for ( int i = 0; i < 5; ++i )
      {
        rowBuffer["ID"].data<int>() = i;
        rowBuffer["TheDate"].data<coral::Date>() = coral::Date( 2005, 11, i+1 );
        rowBuffer["TheTime"].data<coral::TimeStamp>() = coral::TimeStamp::now();
        table.dataEditor().insertRow( rowBuffer );
      }

      coral::IBulkOperation* bulkInserter = table.dataEditor().bulkInsert( rowBuffer, 3 );

      long fraction = 111111111;
      for ( int i = 5; i < 10; ++i, fraction /= 10 )
      {
        rowBuffer["ID"].data<int>() = i;
        rowBuffer["TheDate"].data<coral::Date>() = coral::Date( 2005, 11, i+1 );
        coral::TimeStamp thisMoment = coral::TimeStamp::now();
        rowBuffer["TheTime"].data<coral::TimeStamp>() = coral::TimeStamp( thisMoment.year(),
                                                                          thisMoment.month(),
                                                                          thisMoment.day(),
                                                                          thisMoment.hour(),
                                                                          thisMoment.minute(),
                                                                          thisMoment.second(),
                                                                          (i-4) * fraction );
        bulkInserter->processNextIteration();
      }
      bulkInserter->flush();
      delete bulkInserter;

      session->transaction().commit();

      coral::sleepSeconds(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_Read()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_DAT_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start( true );

      coral::ISchema& schema = session->nominalSchema();

      std::auto_ptr<coral::IQuery> query( schema.tableHandle(T1).newQuery() );

      query->setRowCacheSize( 20 );
      coral::ICursor& cursor = query->execute();
      int i = 0;
      while ( cursor.next() )
      {
        const coral::AttributeList& currentRow = cursor.currentRow();
        currentRow.toOutputStream( std::cout ) << std::endl;
        ++i;
      }

      if ( i != 10 )
        throw std::runtime_error( "Unexpected number of rows" );

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_DAT_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      schema.dropTable( T1 );

      session->transaction().commit();
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( DateAndTimeTest );

}

CORALCPPUNITTEST_MAIN( DateAndTimeTest )
