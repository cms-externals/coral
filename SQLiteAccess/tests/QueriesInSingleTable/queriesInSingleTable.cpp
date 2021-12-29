#include <iostream>
#include <stdexcept>
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Exception.h"
#include "CoralKernel/Context.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/TableDescription.h"

namespace coral
{
  class QueriesInSingleTableTest;
}

class coral::QueriesInSingleTableTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( QueriesInSingleTableTest );
  CPPUNIT_TEST( test_All );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  setUp()
  {
    std::string T1 = BuildUniqueTableName( "SQL_UT_ST_T1" );

    coral::ConnectionService connSvc;
    std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

    session->transaction().start(false);
    coral::ISchema& schema = session->nominalSchema();

    schema.dropIfExistsTable( T1 );

    coral::TableDescription description0;
    description0.setName( T1 );
    description0.insertColumn( "ID",coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    std::cout<<"typenameforid int "<<coral::AttributeSpecification::typeNameForId( typeid(int) )<<std::endl;
    description0.insertColumn( "X",coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    description0.insertColumn( "Y",coral::AttributeSpecification::typeNameForId( typeid(double) ) );
    description0.insertColumn( "Z",coral::AttributeSpecification::typeNameForId( typeid(double) ) );
    description0.insertColumn( "data1",coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description0.insertColumn( "data2",coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );
    description0.insertColumn( "data3",coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );

    coral::ITable& table = schema.createTable( description0 );
    coral::AttributeList rowBuffer;
    table.dataEditor().rowBuffer( rowBuffer );
    for ( int i = 0; i < 5; ++i )
    {
      std::cout << i << std::endl;

      rowBuffer["ID"].data<int>() = i;
      rowBuffer["X"].data<float>() = (float)(i + 0.1 * i);
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
      rowBuffer["data1"].data<std::string>() = "Data";

      if ( i%2 == 1 )
      {
        rowBuffer["data2"].setNull( true );
      }
      else
      {
        rowBuffer["data2"].setNull( false );
        // new blob
        coral::Blob& blob = rowBuffer["data2"].data<coral::Blob>();
        int blobSize = ( i + 1 ) * 1000;
        blob.resize( blobSize );
        // fill the blob
        unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
        for ( int j = 0; j < blobSize; ++j, ++p ) *p = j%256;
      }
      coral::Blob& zeroblob = rowBuffer["data3"].data<coral::Blob>();
      zeroblob.resize( 0 );
      table.dataEditor().insertRow( rowBuffer );
    }
    session->transaction().commit();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  test_All()
  {
    std::string T1 = BuildUniqueTableName( "SQL_UT_ST_T1" );

    coral::ConnectionService connSvc;
    std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

    coral::ISchema& schema = session->nominalSchema();

    session->transaction().start();

    coral::ITable& table = schema.tableHandle( T1 );

    // Performing a query without output spec
    std::cout << "Querying : SELECT * FROM mytest" << std::endl;
    coral::IQuery* query1=table.newQuery();
    query1->setRowCacheSize( 10 );
    coral::ICursor& cursor1 = query1->execute();
    unsigned int s=0;
    while( cursor1.next() )
    {
      const coral::AttributeList& row = cursor1.currentRow();

      row.toOutputStream( std::cout ) << std::endl;

      if(row["ID"].isNull())
      {
        std::cout<<s<<" row ID column is NULL"<<std::endl;
      }
      else
      {
        std::cout<<"ID column has value: "<<row["ID"].data<int>()<<std::endl;
      }

      if(row["Y"].isNull())
      {
        std::cout<<s<<" row Y column is NULL"<<std::endl;
      }

      if(row["data2"].isNull())
      {
        std::cout<<s<<" row data2 Blob column is NULL"<<std::endl;
      }

      if(row["data3"].isNull())
      {
        std::cout<<s<<" row data3 Blob column is NULL!!!!!"<<std::endl;
      }
      ++s;
    }
    cursor1.close();
    std::cout << s << " row(s) selected." << std::endl;
    delete query1;

    std::cout << "Querying : SELECT X, Y as alias_y FROM mytest WHERE ID > 1" << std::endl;
    coral::IQuery* query2=table.newQuery() ;
    query2->addToOutputList( "X" );
    query2->addToOutputList( "Y", "alias_y");
    query2->setMemoryCacheSize( 1 );
    coral::AttributeList emptyBindVariableList2;
    query2->setCondition( "ID > 1", emptyBindVariableList2 );
    coral::ICursor& cursor2 = query2->execute();
    s=0;
    while( cursor2.next() )
    {
      const coral::AttributeList& row = cursor2.currentRow();
      row.toOutputStream( std::cout ) << std::endl;
      ++s;
    }
    std::cout << s << " row(s) selected." << std::endl;
    delete query2;

    std::cout << "Querying : SELECT X, Y as alias_y FROM mytest WHERE ID > 1" << std::endl;
    coral::IQuery* query3=table.newQuery();
    query3->addToOutputList( "X" );
    query3->addToOutputList( "Y", "alias_y");
    query3->setMemoryCacheSize( 1 );
    coral::AttributeList BindVariableList;
    BindVariableList.extend("idvalue", typeid(int) );
    BindVariableList["idvalue"].data<int>()=1;
    query3->setCondition( "ID > :idvalue", BindVariableList );
    coral::ICursor& cursor3 = query3->execute();
    s=0;
    while( cursor3.next() ) {
      const coral::AttributeList& row = cursor3.currentRow();
      row.toOutputStream( std::cout ) << std::endl;
      ++s;
    }
    std::cout << s << " row(s) selected." << std::endl;
    delete query3;

    // Performing a query with SQL function
    std::cout << "Querying : SELECT max(ID) FROM mytest" << std::endl;
    coral::IQuery* query4=table.newQuery();
    query4->addToOutputList( "max(ID)", "max_id");
    coral::AttributeList myresult;
    myresult.extend("max(ID)", typeid(unsigned long));
    query4->defineOutput( myresult );
    coral::ICursor& cursor4 = query4->execute();
    s=0;
    while( cursor4.next() ) {
      const coral::AttributeList& row = cursor4.currentRow();
      row.toOutputStream( std::cout ) << std::endl;
      ++s;
    }
    std::cout<<s << " row(s) selected." << std::endl;
    delete query4;
    // Committing the transaction
    std::cout << "Committing..." << std::endl;

    session->transaction().commit();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  tearDown()
  {
    std::string T1 = BuildUniqueTableName( "SQL_UT_ST_T1" );

    coral::ConnectionService connSvc;
    std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

    coral::ISchema& schema = session->nominalSchema();

    session->transaction().start(false);

    schema.dropTable( T1 );

    session->transaction().commit();
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::QueriesInSingleTableTest );

CORALCPPUNITTEST_MAIN( QueriesInSingleTableTest )
