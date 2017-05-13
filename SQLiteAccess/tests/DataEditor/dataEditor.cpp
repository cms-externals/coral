#include <iostream>
#include <stdexcept>
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Exception.h"
#include "CoralKernel/Context.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/TableDescription.h"

namespace coral
{
  class DataEditorTest;
}

class coral::DataEditorTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( DataEditorTest );
  CPPUNIT_TEST( test_All );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void setUp()
  {
    std::string T1 = BuildUniqueTableName( "SQL_UT_DE_T1" );
    coral::ConnectionService connSvc;
    std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );
    coral::ISchema& schema = session->nominalSchema();
    session->transaction().start();
    schema.dropIfExistsTable( T1 );
    //
    // Creating the table
    //
    std::cout<<"create table x1 (id UNSIGNEDLONG PRIMARY KEY, a STRING NOT NULL UNIQUE, b STRING NOT NULL UNIQUE, c STRING NOT NULL UNIQUE)"<<std::endl;
    coral::TableDescription description1;
    description1.setName(T1);
    description1.insertColumn("id", coral::AttributeSpecification::typeNameForId( typeid(unsigned int) ) );
    description1.insertColumn("a", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description1.insertColumn("b", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description1.insertColumn("c", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description1.setPrimaryKey( "id" );
    description1.setNotNullConstraint( "a" );
    description1.setNotNullConstraint( "b" );
    description1.setNotNullConstraint( "c" );
    description1.setUniqueConstraint( "a" );
    description1.setUniqueConstraint( "b" );
    //description1.setUniqueConstraint( "c" );
    schema.createTable(description1 );
    session->transaction().commit();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_All()
  {
    std::string T1 = BuildUniqueTableName( "SQL_UT_DE_T1" );
    coral::ConnectionService connSvc;
    std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );
    coral::ISchema& schema = session->nominalSchema();
    // common variables
    coral::AttributeList emptybinddata;
    //
    // insert new row
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      coral::AttributeList newdata;
      table.dataEditor().rowBuffer( newdata );
      newdata["id"].data<unsigned int>()= 1 ;
      newdata["a"].data<std::string>()= "avalue" ;
      newdata["b"].data<std::string>()= "bvalue" ;
      newdata["c"].data<std::string>()= "cvalue";
      table.dataEditor().insertRow( newdata );
      session->transaction().commit();
    }
    //
    // insert new duplicate row
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      coral::AttributeList newdata2;
      table.dataEditor().rowBuffer( newdata2 );
      newdata2["id"].data<unsigned int>()= 1 ;
      newdata2["a"].data<std::string>()= "avalue" ;
      newdata2["b"].data<std::string>()= "bvalue" ;
      newdata2["c"].data<std::string>()= "cvalue";
      // filter the expected exception
      try
      {
        table.dataEditor().insertRow( newdata2 );
        session->transaction().commit();
      }
      catch(const coral::DuplicateEntryInUniqueKeyException&)
      {
        session->transaction().rollback();
      }
      catch(...)
      {
        throw;
      }
    }
    //
    // update row
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      long nrow = table.dataEditor().updateRows( "a='anewvalue'", "a='avalue'", emptybinddata);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Could not update the row in the table", nrow == 0, false );
      std::cout<<"numberOfRowsUpdated: "<<nrow<<std::endl;
      session->transaction().commit() ;
    }
    //
    // check updated a value
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      std::auto_ptr<coral::IQuery> query( table.newQuery() );
      query->addToOutputList( "a" );
      coral::AttributeList outBuffer;
      outBuffer.extend( "a",typeid(std::string)  );
      query->defineOutput( outBuffer );
      query->setForUpdate();
      coral::ICursor& cursor = query->execute();
      while( cursor.next() )
      {
        const coral::AttributeList& row = cursor.currentRow();
        std::cout<<"query result: "<<std::endl;
        row.toOutputStream( std::cout ) << std::endl;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", row["a"].data<std::string>(), std::string("anewvalue" ) );
      }
      cursor.close();
      session->transaction().commit() ;
    }
    //
    // delete row
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      long drow = table.dataEditor().deleteRows( "", emptybinddata );
      std::cout << "Committing..." << std::endl;
      session->transaction().commit();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong number of effected rows", drow, (long)1 );
    }
    //
    // insert new row
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      coral::AttributeList newdata;
      table.dataEditor().rowBuffer( newdata );
      newdata["id"].data<unsigned int>()=1;
      newdata["a"].data<std::string>()= "avalue1";
      newdata["b"].data<std::string>()= "bvalue1";
      newdata["c"].data<std::string>()= "cvalue1";
      table.dataEditor().insertRow( newdata ) ;
      newdata["id"].data<unsigned int>()= 2 ;
      newdata["a"].data<std::string>()= "avalue2";
      newdata["b"].data<std::string>()= "bvalue2";
      newdata["c"].data<std::string>()= "cvalue2";
      table.dataEditor().insertRow( newdata );
      session->transaction().commit();
    }
    //
    // update row with binding
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      coral::AttributeList dataForUpdate;
      dataForUpdate.extend("newid", typeid(unsigned int) );
      dataForUpdate.extend( "idValue", typeid(unsigned int) );
      dataForUpdate["newid"].data<unsigned int>()=100;
      dataForUpdate["idValue"].data<unsigned int>()=1;
      long nrow = table.dataEditor().updateRows( "id = :newid", "id >:idValue", dataForUpdate );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Could not update the row in the table", nrow == 0, false );
      session->transaction().commit();
    }
    //
    // check updated id value
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      std::auto_ptr<coral::IQuery> query2( table.newQuery() );
      query2->addToOutputList( "id" );
      coral::AttributeList output;
      output.extend( "id",typeid(unsigned int)  );
      query2->defineOutput( output );
      query2->setForUpdate();
      coral::ICursor& cursor2 = query2->execute();
      while( cursor2.next() )
      {
        const coral::AttributeList& row = cursor2.currentRow();
        std::cout<<"query result: "<<std::endl;
        row.toOutputStream( std::cout ) << std::endl;
        if( row["id"].data<unsigned int>() > 1 )
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong index", row["id"].data<unsigned int>(), (unsigned int)100 );
        }
      }
      cursor2.close();
      session->transaction().commit();
    }
    //
    // check updated with not ordered binding
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      coral::AttributeList dataForUpdate2;
      dataForUpdate2.extend( "idValue", typeid(unsigned int) );
      dataForUpdate2.extend("newid", typeid(unsigned int) );
      dataForUpdate2["newid"].data<unsigned int>()=110;
      dataForUpdate2["idValue"].data<unsigned int>()=1;
      long nrow = table.dataEditor().updateRows( "id = :newid", "id >:idValue", dataForUpdate2 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Could not update the row in the table", nrow == 0, false );
      session->transaction().commit();
    }
    //
    // check updated id value
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      std::auto_ptr<coral::IQuery> query3( table.newQuery() );
      query3->addToOutputList( "id" );
      coral::AttributeList output3;
      output3.extend( "id",typeid(unsigned int)  );
      query3->defineOutput( output3 );
      query3->setForUpdate();
      coral::ICursor& cursor3 = query3->execute();
      while( cursor3.next() )
      {
        const coral::AttributeList& row = cursor3.currentRow();
        std::cout<<"query result: "<<std::endl;
        row.toOutputStream( std::cout ) << std::endl;
        if( row["id"].data<unsigned int>() > 1 )
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong id", row["id"].data<unsigned int>(), (unsigned int)110 );
        }
      }
      cursor3.close();
      session->transaction().commit();
    }
    //
    // update row without WHERE condition
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      coral::AttributeList cForUpdate;
      cForUpdate.extend("c", typeid(std::string) );
      cForUpdate["c"].data<std::string>()="hello";
      long nrow = table.dataEditor().updateRows( "c = :c","", cForUpdate );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Could not update the row in the table", nrow == 0, false );
      session->transaction().commit();
    }
    //
    // check updated c value
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      std::auto_ptr<coral::IQuery> queryc( table.newQuery() );
      queryc->addToOutputList( "c" );
      coral::AttributeList qoutput;
      qoutput.extend( "c",typeid(std::string)  );
      queryc->defineOutput( qoutput );
      queryc->setForUpdate();
      coral::ICursor& cursorc = queryc->execute();
      while( cursorc.next() )
      {
        const coral::AttributeList& row = cursorc.currentRow();
        std::cout<<"query result: "<<std::endl;
        row.toOutputStream( std::cout ) << std::endl;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong c value", row["c"].data<std::string>(), std::string("hello") );
      }
      cursorc.close();
      session->transaction().commit() ;
    }
    //
    // delete row with binding
    //
    {
      session->transaction().start();
      coral::ITable& table = schema.tableHandle(T1);
      coral::AttributeList dataForDelete;
      dataForDelete.extend( "bValue", typeid(std::string) );
      dataForDelete["bValue"].data<std::string>()= "bvalue1";
      long nrow = table.dataEditor().deleteRows( "b =:bValue", dataForDelete );
      session->transaction().commit();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong number of deleted rows", nrow, (long)1 );
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void tearDown()
  {
    std::string T1 = BuildUniqueTableName( "SQL_UT_DE_T1" );
    coral::ConnectionService connSvc;
    std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );
    coral::ISchema& schema = session->nominalSchema();
    session->transaction().start();
    schema.dropTable( T1 );
    session->transaction().commit();
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::DataEditorTest );

CORALCPPUNITTEST_MAIN( DataEditorTest )
