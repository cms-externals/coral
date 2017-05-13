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
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/TableDescription.h"

namespace coral
{
  class DescribeTableTest;
}

class coral::DescribeTableTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( DescribeTableTest );
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
    std::string T1 = BuildUniqueTableName( "SQL_UT_DT_T1" );
    std::string T2 = BuildUniqueTableName( "SQL_UT_DT_T2" );
    std::string T3 = BuildUniqueTableName( "SQL_UT_DT_T3" );
    coral::ConnectionService connSvc;
    std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );
    coral::ISchema& schema = session->nominalSchema();
    //
    // create test table x1
    //
    session->transaction().start(false);
    schema.dropIfExistsTable( T1 );
    std::cout << "create table " << T1 << " (id INT PRIMARY KEY, name STRING NOT NULL UNIQUE, address STRING UNIQUE, order INT)"<<std::endl;
    coral::TableDescription description1;
    description1.setName(T1);
    description1.insertColumn("id", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description1.insertColumn("name", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description1.insertColumn("address", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description1.insertColumn("order", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description1.setPrimaryKey( "id" );
    description1.setNotNullConstraint( "name" );
    description1.setUniqueConstraint( "name" );
    description1.setUniqueConstraint( "address" );
    session->nominalSchema().createTable(description1 );
    session->transaction().commit();
    //
    // create test table x2 (combined PK and indices)
    //
    session->transaction().start(false);
    schema.dropIfExistsTable( T2 );
    std::cout<<"create table " << T2 << " (id INT, name STRING NOT NULL, address STRING) primary key(id,name)"<<std::endl;
    coral::TableDescription description2;
    description2.setName(T2);
    description2.insertColumn("id", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description2.insertColumn("name", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description2.insertColumn("address", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    std::vector<std::string> pks;
    pks.push_back("id");
    pks.push_back("name");
    description2.setPrimaryKey( pks, "" );
    description2.setNotNullConstraint( "name" );
    session->nominalSchema().createTable(description2 );
    session->transaction().commit();
    //
    // create test table x3 (foreign keys)
    //
    session->transaction().start(false);
    schema.dropIfExistsTable( T3 );
    std::cout<<"create table " << T3 << " (id INT primary key, name STRING NOT NULL, address STRING) foreign key(id) references x2(id)" << std::endl;
    coral::TableDescription description3;
    description3.setName(T3);
    description3.insertColumn("id", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description3.insertColumn("name", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description3.insertColumn("address", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description3.setPrimaryKey( "id", "" );
    description3.setNotNullConstraint( "name" );
    description3.createForeignKey("","id",T2,"id");
    session->nominalSchema().createTable(description3 );
    session->transaction().commit();
    //
    // describe tables
    //
    session->transaction().start();
    std::set<std::string> tables = schema.listTables();
    int ntables = tables.size();
    std::cout << "Number of tables: " << ntables << std::endl;
    for( std::set<std::string>::iterator it=tables.begin();
         it!=tables.end(); ++it )
    {
      coral::ITable& tab = schema.tableHandle(*it);
      const coral::ITableDescription& desc = tab.description();
      std::cout << "Table name: " << desc.name() << std::endl;
      std::cout << "Number of columns: "
                << desc.numberOfColumns() << std::endl;
      for( int i=0; i<desc.numberOfColumns(); ++i )
      {
        std::cout << "   columnName  "
                  << desc.columnDescription(i).name() << std::endl;
      }
      std::cout << "Has primary key " << desc.hasPrimaryKey() << std::endl;
      try
      {
        const coral::IPrimaryKey& mypk = desc.primaryKey();
        const std::vector<std::string>& colNames = mypk.columnNames();
        for( std::vector<std::string>::const_iterator it2=colNames.begin();
             it2!=colNames.end(); ++it2 )
        {
          std::cout << "   PK columnName  "<<*it2<<std::endl;
        }
      }
      catch( const coral::NoPrimaryKeyException& )
      {
        std::cout << "   PK does not exist" << std::endl;
      }
      std::cout << "Number of indices: "
                << desc.numberOfIndices() << std::endl;
      std::cout << "Number of foreign keys: "
                << desc.numberOfForeignKeys() << std::endl;
      try
      {
        const coral::IForeignKey& fk = desc.foreignKey(0);
        std::cout << "   FK 0 name " << fk.name() << std::endl;
        std::cout << "   FK 0 references table name "
                  << fk.referencedTableName() << std::endl;
      }
      catch( const coral::InvalidForeignKeyIdentifierException& )
      {
        std::cout << "   FK 0 does not exist" << std::endl;
      }
      std::cout << "Number of unique constraints: "
                << desc.numberOfUniqueConstraints() << std::endl;
    }
    std::cout << "Committing..." << std::endl;
    session->transaction().commit();

    // re-check #tables for coral_sqlite_fk in another session (bug #82563)
    {
      std::auto_ptr<coral::ISessionProxy> session2( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );
      std::cout << "Rechecking #tables, was: " << ntables << std::endl;
      session2->transaction().start(true);
      int ntables2 = session2->nominalSchema().listTables().size();
      std::cout << "Number of tables: " << ntables2 << std::endl;
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "#tables", ntables, ntables2 );
      std::cout << "Expected number of tables: 3" << std::endl;
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "#tables", 3, ntables2 );
      session2->transaction().commit();
    }

    // clean up
    std::cout << "Cleaning up..." << std::endl;
    session->transaction().start();
    schema.dropTable( T3 );
    schema.dropTable( T2 );
    schema.dropTable( T1 );
    session->transaction().commit();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void
  tearDown()
  {
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::DescribeTableTest );

CORALCPPUNITTEST_MAIN( DescribeTableTest )
