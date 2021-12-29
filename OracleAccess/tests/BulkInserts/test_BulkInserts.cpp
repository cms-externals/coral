// Include files
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "CoralCommon/Utilities.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/IColumn.h"
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
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

namespace coral
{

  class BulkInsertsTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( BulkInsertsTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_BI_T1" );
      std::string T2 = BuildUniqueTableName( "ORA_UT_BI_T2" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

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
      description.insertColumn( "Y",
                                coral::AttributeSpecification::typeNameForId( typeid(double) ) );
      description.insertColumn( "Z",
                                coral::AttributeSpecification::typeNameForId( typeid(double) ) );

      description.insertColumn( "data1",
                                coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                                100,
                                false );

      description.insertColumn( "data2",
                                coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );
      std::cout << "About to create the table" << std::endl;
      coral::ITable& table = schema.createTable( description );

      coral::AttributeList rowBuffer;
      table.dataEditor().rowBuffer( rowBuffer );



      coral::IBulkOperation* bulkInserter = table.dataEditor().bulkInsert( rowBuffer, 3 );

      for ( int i = 0; i < 5; ++i )
      {
        rowBuffer["ID"].data<int>() = i;
        rowBuffer["x"].data<float>() = (float)( i + 0.1 * i );

        if ( i%2 == 1 )
        {
          rowBuffer["Y"].setNull( true );
        }
        else
        {
          rowBuffer["Y"].setNull( false );
          rowBuffer["Y"].data<double>() = i + 0.01 * i;
        }
        rowBuffer["Z"].data<double>() = i + 0.001 * i;

        std::ostringstream os;
        os << "Data for " << i;
        rowBuffer["data1"].data<std::string>() = os.str();

        if ( i%2 == 1 )
        {
          rowBuffer["data2"].setNull( true );
        }
        else
        {
          rowBuffer["data2"].setNull( false );

          coral::Blob& blob = rowBuffer["data2"].data<coral::Blob>();
          int blobSize = ( i + 1 ) * 1000;
          blob.resize( blobSize );
          unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
          for ( int j = 0; j < blobSize; ++j, ++p ) *p = j%256;
        }

        bulkInserter->processNextIteration();
      }

      bulkInserter->flush();

      delete bulkInserter;


      std::cout << "About to drop previous table (2nd)" << std::endl;
      schema.dropIfExistsTable( T2 );

      std::cout << "Describing new table (2nd)" << std::endl;
      coral::TableDescription description2;
      description2.setName( T2 );
      description2.insertColumn( "ID",
                                 coral::AttributeSpecification::typeNameForId( typeid(int) ) );
      description2.setPrimaryKey( "ID" );
      description2.insertColumn( "X",
                                 coral::AttributeSpecification::typeNameForId( typeid(float) ) );
      coral::ITable& table2 = schema.createTable( description2 );

      coral::AttributeList buffer2;
      buffer2.extend<int>( "ID" );
      buffer2.extend<float>( "X" );

      bulkInserter = table2.dataEditor().bulkInsert( buffer2, 100000 );

      for ( int i = 0; i < 120000; ++i )
      {
        buffer2["ID"].data<int>() = i;
        buffer2["X"].data<float>() = (float)( i + 0.1 * i );
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
      std::string T1 = BuildUniqueTableName( "ORA_UT_BI_T1" );
      std::string T2 = BuildUniqueTableName( "ORA_UT_BI_T2" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start( true );

      coral::ISchema& schema = session->nominalSchema();

      coral::IQuery* query = schema.tableHandle(T1).newQuery();
      coral::ICursor& cursor = query->execute();
      int i = 0;
      while ( cursor.next() )
      {
        const coral::AttributeList& currentRow = cursor.currentRow();
        currentRow.toOutputStream( std::cout ) << std::endl;
        const coral::Attribute& blobAttribute = currentRow["data2"];
        if ( ! blobAttribute.isNull() )
        {
          const coral::Blob& blob = blobAttribute.data<coral::Blob>();
          int blobSize = ( i + 1 ) * 1000;
          if ( blob.size() != blobSize )
            throw std::runtime_error( "Unexpected blob size" );
          const unsigned char* p = static_cast<const unsigned char*>( blob.startingAddress() );
          for ( int j = 0; j < blobSize; ++j, ++p )
            if( *p != j%256 )
              throw std::runtime_error( "Unexpected blob data" );
        }
        ++i;
      }

      if ( i != 5 )
        throw std::runtime_error( "Unexpected number of rows" );

      delete query;

      query = schema.tableHandle(T2).newQuery();
      coral::AttributeList buffer2;

      buffer2.extend<int>( "ID" );
      query->addToOutputList( "ID" );
      query->addToOrderList( "ID ASC" );
      query->setRowCacheSize( 1000 );
      query->defineOutput( buffer2 );
      coral::ICursor& cursor2 = query->execute();
      int j = 0;
      while ( cursor2.next() )
      {
        if ( buffer2["ID"].data<int>() != j )
        {
          throw std::runtime_error( "unexpected result" );
        }
        j++;
      }

      if ( j != 120000 ) throw std::runtime_error( "unexpected returned rows" );

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_BI_T1" );
      std::string T2 = BuildUniqueTableName( "ORA_UT_BI_T2" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      schema.dropTable( T1 );
      schema.dropTable( T2 );

      session->transaction().commit();
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( BulkInsertsTest );

}

CORALCPPUNITTEST_MAIN( BulkInsertsTest )
