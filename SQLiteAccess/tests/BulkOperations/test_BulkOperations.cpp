#include <climits> // fix bug #58581
#include <float.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
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
#include "RelationalAccess/IBulkOperationWithQuery.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IOperationWithQuery.h"
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

#ifdef _WIN32
#undef max //solve windows compilation
#undef min
#endif

namespace coral
{
  class BulkOperationsTest;
}

class coral::BulkOperationsTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( BulkOperationsTest );
  CPPUNIT_TEST( test_All );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  setUp()
  {
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  test_All()
  {
    // source table
    std::string T_SRC = BuildUniqueTableName( "SQL_UT_BO_SRC" );
    // destination table
    std::string T_DST = BuildUniqueTableName( "SQL_UT_BO_DST" );

    coral::ConnectionService connSvc;

    std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

    session->transaction().start(false) ;

    coral::ISchema& schema = session->nominalSchema();

    // source table
    coral::TableDescription desc;
    desc.setName( T_SRC );
    desc.insertColumn( "id", coral::AttributeSpecification::typeNameForId(typeid(int)) );
    desc.insertColumn( "myfloat", coral::AttributeSpecification::typeNameForId(typeid(float)) );

    schema.dropIfExistsTable( T_SRC );
    coral::ITable& table = schema.createTable( desc );

    // dest table
    desc.setName( T_DST );
    schema.dropIfExistsTable( T_DST );
    schema.createTable( desc );

    coral::AttributeList data;
    table.dataEditor().rowBuffer(data);
    std::cout << "Inserting new rows into the source table" << std::endl;
    coral::IBulkOperation* rowInserter = table.dataEditor().bulkInsert(data,10);
    for ( int i = 0; i < 50; ++i )
    {
      data["id"].data<int>()=i ;
      data["myfloat"].data<float>()=i*(float)3.1415;
      rowInserter->processNextIteration();
    }
    rowInserter->flush();
    session->transaction().commit();
    session->transaction().start(false);
    coral::ITable& dest = session->nominalSchema().tableHandle( T_DST );

    coral::IOperationWithQuery* op = dest.dataEditor().insertWithQuery();
    coral::IQueryDefinition& query = op->query();
    query.addToTableList( T_SRC );
    query.setCondition( "id<10", coral::AttributeList() );
    long numberOfRowsInserted = op->execute();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong number of inserted rows", numberOfRowsInserted, (long)10 );

    delete op;

    coral::IBulkOperationWithQuery* bulkOperation = dest.dataEditor().bulkInsertWithQuery( 3 );
    coral::IQueryDefinition& query2 = bulkOperation->query();
    query2.addToTableList( T_SRC );
    coral::AttributeList condition;
    condition.extend<long>( "idmin");
    condition.extend<long>( "idmax");
    query2.setCondition( "id>:idmin AND id<:idmax", condition );
    long& idmin = condition[0].data<long>();
    long& idmax = condition[1].data<long>();
    idmin = 10;
    idmax = 13;
    for ( int i = 0; i < 5; ++i )
    {
      bulkOperation->processNextIteration();
      idmin += 4;
      idmax += 4;
    }
    bulkOperation->flush();
    delete bulkOperation;
    // Bulk delete some rows from source
    std::string deleteCondition = "id<:idmax AND id>:idmin ";
    coral::AttributeList deleteData;
    deleteData.extend<long>( "idmax" );
    deleteData.extend<long>( "idmin" );
    coral::ITable& source = session->nominalSchema().tableHandle( T_SRC );
    coral::IBulkOperation* bulkDeleter=source.dataEditor().bulkDeleteRows( deleteCondition,deleteData,3 );
    for ( int i = 0; i < 5; ++i )
    {
      deleteData["idmin"].data<long>()=i;
      deleteData["idmax"].data<long>()=i+2;
      bulkDeleter->processNextIteration();
    }
    bulkDeleter->flush();
    delete bulkDeleter;

    // clean up
    schema.dropTable( T_SRC );
    schema.dropTable( T_DST );

    session->transaction().commit();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  tearDown()
  {
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::BulkOperationsTest );

CORALCPPUNITTEST_MAIN( BulkOperationsTest )
