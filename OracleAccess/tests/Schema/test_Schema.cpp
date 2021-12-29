// Include files
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "CoralCommon/Utilities.h"
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
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/TableDescription.h"

namespace coral
{

  class SchemaTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( SchemaTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_S_T1" );
      std::string T2 = BuildUniqueTableName( "ORA_UT_S_T2" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      std::cout << "About to drop previous tables" << std::endl;
      schema.dropIfExistsTable( T2 );
      schema.dropIfExistsTable( T1 );

      std::cout << "Describing new tables" << std::endl;

      coral::TableDescription description0;
      description0.setName( T1 );
      description0.insertColumn( "ID",
                                 coral::AttributeSpecification::typeNameForId( typeid(int) ) );
      description0.setUniqueConstraint( "ID" );
      description0.insertColumn( "X",
                                 coral::AttributeSpecification::typeNameForId( typeid(float) ) );
      description0.insertColumn( "Y",
                                 coral::AttributeSpecification::typeNameForId( typeid(float) ) );

      std::vector< std::string > cols;
      cols.push_back( "X" );
      cols.push_back( "Y" );
      description0.createIndex( T1 + "_IDX", cols );

      coral::TableDescription description1;
      description1.setName( T2 );
      description1.insertColumn( "ID",
                                 coral::AttributeSpecification::typeNameForId( typeid(int) ) );
      description1.setPrimaryKey( "ID" );
      description1.insertColumn( "x",
                                 coral::AttributeSpecification::typeNameForId( typeid(float) ) );
      description1.setNotNullConstraint( "x" );
      description1.insertColumn( "Y",
                                 coral::AttributeSpecification::typeNameForId( typeid(double) ) );
      description1.setUniqueConstraint( "Y" );
      description1.insertColumn( "Z",
                                 coral::AttributeSpecification::typeNameForId( typeid(double) ) );
      description1.insertColumn( "TheDate",
                                 coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
      description1.insertColumn( "TheTime",
                                 coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ),
                                 3 );

      std::vector< std::string > constraintColumns( 2 );
      constraintColumns[0] = "x";
      constraintColumns[1] = "Z";
      description1.setUniqueConstraint( constraintColumns, "C_" + T2 );
      description1.insertColumn( "data1",
                                 coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                                 123,
                                 true );
      description1.insertColumn( "data2",
                                 coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                                 321,
                                 false );
      description1.insertColumn( "ID_FK",
                                 coral::AttributeSpecification::typeNameForId( typeid(int) ) );
      description1.createForeignKey( T2 + "_FK", "ID_FK", T1, "ID" );

      std::cout << "About to create the tables" << std::endl;

      coral::ITable& table0 = schema.createTable( description0 );
      table0.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
      table0.schemaEditor().renameColumn( "Y", "Z" );

      coral::ITable& table1 = schema.createTable( description1 );

      std::string tableSpaceName = table1.description().tableSpaceName();
      std::cout << "Table " << T2 << " created under the table space \"" << tableSpaceName << "\"" << std::endl;

      std::cout << "Retrieving the table description." << std::endl;

      std::cout << "Description : " << std::endl;
      int numberOfColumns = table1.description().numberOfColumns();
      for ( int i = 0; i < numberOfColumns; ++i )
      {
        const coral::IColumn& column = table1.description().columnDescription( i );
        std::cout << column.name() << " : " << column.type();
        if ( column.isNotNull() ) std::cout << " NOT NULL";
        if ( column.isUnique() ) std::cout << " UNIQUE";
        std::cout << std::endl;
      }

      if ( table1.description().hasPrimaryKey() )
      {
        const coral::IPrimaryKey& pk = table1.description().primaryKey();
        std::cout << "Table has primary key defined in tablespace " << pk.tableSpaceName() << std::endl
                  << " for column(s) ";
        for ( std::vector<std::string>::const_iterator iColumn = pk.columnNames().begin();
              iColumn != pk.columnNames().end(); ++iColumn )
        {
          std::cout << "\"" << *iColumn << "\" ";
        }
        std::cout << std::endl;
      }

      std::cout << "Foreign keys defined:" << std::endl;
      for ( int i = 0; i < table1.description().numberOfForeignKeys(); ++i )
      {
        const coral::IForeignKey& fkey = table1.description().foreignKey( i );
        std::cout << "   " << fkey.name() << " ( ";
        for ( std::vector<std::string>::const_iterator iColumn = fkey.columnNames().begin();
              iColumn != fkey.columnNames().end(); ++iColumn )
        {
          if ( iColumn != fkey.columnNames().begin() ) std::cout << ", ";
          std::cout << *iColumn;
        }
        std::cout << " ) -> " << fkey.referencedTableName() << " ( ";
        for ( std::vector<std::string>::const_iterator iColumn = fkey.referencedColumnNames().begin();
              iColumn != fkey.referencedColumnNames().end(); ++iColumn )
        {
          if ( iColumn != fkey.referencedColumnNames().begin() ) std::cout << ", ";
          std::cout << *iColumn;
        }
        std::cout << " )" << std::endl;
      }

      session->transaction().commit();

      coral::sleepSeconds(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_Read()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_S_T1" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      coral::ISchema& schema = session->nominalSchema();

      session->transaction().start( true );

      std::cout << "Indices for Table " << T1 << " :" << std::endl;
      coral::ITable& table00 = schema.tableHandle( T1 );
      for ( int i = 0; i < table00.description().numberOfIndices(); ++i ) {
        const coral::IIndex& index = table00.description().index( i );
        std::cout << " " << index.name() << " -> ";
        for ( std::vector<std::string>::const_iterator iColumn = index.columnNames().begin();
              iColumn != index.columnNames().end(); ++iColumn ) {
          if ( iColumn != index.columnNames().begin() ) std::cout << ", ";
          std::cout << *iColumn;
        }
        std::cout << " (tablespace : " << index.tableSpaceName() << ")" << std::endl;
      }

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_S_T1" );
      std::string T2 = BuildUniqueTableName( "ORA_UT_S_T2" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      schema.dropTable( T2 );
      schema.dropTable( T1 );

      session->transaction().commit();
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( SchemaTest );

}

CORALCPPUNITTEST_MAIN( SchemaTest )
