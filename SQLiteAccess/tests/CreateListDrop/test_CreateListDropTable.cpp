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
#include "RelationalAccess/IOperationWithQuery.h"
#include "RelationalAccess/IBulkOperationWithQuery.h"
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
  class CreateListDropTableTest;
}

class coral::CreateListDropTableTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( CreateListDropTableTest );
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
    std::string T1 = BuildUniqueTableName( "SQL_UT_CD_T1" );

    coral::ConnectionService connSvc;

    std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

    session->transaction().start(false);

    coral::ISchema& schema = session->nominalSchema();

    schema.dropIfExistsTable( T1 );

    coral::TableDescription description0;

    description0.setName( T1 );
    description0.insertColumn( "ID",coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description0.insertColumn( "X",coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    description0.insertColumn( "Y",coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    description0.insertColumn( "ORDER",coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    std::vector<std::string> idx_cols;
    idx_cols.push_back("ORDER");
    description0.createIndex("IDX1",idx_cols,false);
    schema.createTable( description0 );
    std::set<std::string> result = schema.listTables();
    for(std::set<std::string>::iterator it=result.begin(); it!=result.end(); ++it)
    {
      std::cout<<"table names: "<<*it<<std::endl;
      schema.dropTable(*it);
    }
    //Committing the transaction
    std::cout << "Committing..." << std::endl;
    session->transaction().commit();

    session->transaction().start();
    std::set<std::string> r=schema.listTables();
    for(std::set<std::string>::iterator it=r.begin(); it!=r.end(); ++it)
    {
      std::cout<<"table names after drop: "<<*it<<std::endl;
    }
    std::cout << "Committing..." << std::endl;

    session->transaction().commit();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  tearDown()
  {
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::CreateListDropTableTest );

CORALCPPUNITTEST_MAIN( CreateListDropTableTest )
