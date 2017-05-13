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
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

#define TABLENAME1 "ORA_UT_DD_T1"
#define TABLENAME2 "ORA_UT_DD_T2"

namespace coral
{

  class DataDictionaryTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( DataDictionaryTest );
    CPPUNIT_TEST( test_setPrimaryKey );
    CPPUNIT_TEST( test_createForeignKey );
    CPPUNIT_TEST( test_createIndex );
    CPPUNIT_TEST_SUITE_END();

  private:
    std::string connectionString;

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    DataDictionaryTest():CoralCppUnitTest()
    {
      char *coralTestDb=getenv("CORALTESTDB");
      if ( coralTestDb )
        connectionString = std::string( coralTestDb );
      else
        connectionString = "CORAL-Oracle-lcgnight/admin";
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( connectionString ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      //std::cout << "About to drop previous table" << std::endl;
      schema.dropIfExistsTable( TABLENAME1 );

      {
        //std::cout << "Describing new tables" << std::endl;
        coral::TableDescription description;
        description.setName( TABLENAME1 );
        description.insertColumn( "ID",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );
        description.setPrimaryKey( "ID" );
        description.insertColumn( "X",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );
        description.setNotNullConstraint( "X" );
        description.insertColumn( "Y",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );
        description.insertColumn( "Z",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );

        //std::cout << "About to create the tables" << std::endl;
        coral::ITableDataEditor& editor = schema.createTable( description ).dataEditor();

        coral::AttributeList rowBuffer;
        editor.rowBuffer( rowBuffer );

        for ( int i = 0; i < 10; ++i )
        {
          rowBuffer["ID"].data<int>() = i;
          rowBuffer["X"].data<int>() = i;
          rowBuffer["Y"].data<int>() = 10*i;
          rowBuffer["Z"].data<int>() = 100 - i;

          editor.insertRow( rowBuffer );
        }
      }

      {
        //std::cout << "About to drop previous table" << std::endl;
        schema.dropIfExistsTable( TABLENAME2 );

        //std::cout << "Describing new tables" << std::endl;
        coral::TableDescription description;
        description.setName( TABLENAME2 );
        description.insertColumn( "ID",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );
        description.setPrimaryKey( "ID" );
        description.insertColumn( "X",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );
        description.setNotNullConstraint( "X" );
        description.insertColumn( "Y",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );
        description.insertColumn( "Z",
                                  coral::AttributeSpecification::typeNameForId( typeid(int) ) );

        //std::cout << "About to create the tables" << std::endl;
        coral::ITableDataEditor& editor = schema.createTable( description ).dataEditor();

        coral::AttributeList rowBuffer;
        editor.rowBuffer( rowBuffer );

        for ( int i = 0; i < 10; ++i )
        {
          rowBuffer["ID"].data<int>() = i;
          rowBuffer["X"].data<int>() = 2*i+4;
          rowBuffer["Y"].data<int>() = 10*i;
          rowBuffer["Z"].data<int>() = 100 - i;

          editor.insertRow( rowBuffer );
        }
      }
      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( connectionString ) );

      session->transaction().start();
      coral::ISchema& schema = session->nominalSchema();

      schema.dropIfExistsTable( TABLENAME1 );
      schema.dropIfExistsTable( TABLENAME2 );
      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_setPrimaryKey()
    {
      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( connectionString ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      coral::ITable& table = schema.tableHandle( TABLENAME1 );

      coral::ITableSchemaEditor& schemaEditor = table.schemaEditor();
      const coral::ITableDescription& description = table.description();

      CPPUNIT_ASSERT_EQUAL(true, description.hasPrimaryKey() );

      // drop the primary key
      schemaEditor.dropPrimaryKey();
      CPPUNIT_ASSERT_EQUAL(false, description.hasPrimaryKey() );

      // drop the primary key again, expect exception
      CPPUNIT_ASSERT_THROW( schemaEditor.dropPrimaryKey(),
                            coral::NoPrimaryKeyException );
      CPPUNIT_ASSERT_EQUAL(false, description.hasPrimaryKey() );

      // create a uniqueConstraint for ID
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfUniqueConstraints() );
      schemaEditor.setUniqueConstraint("ID",
                                       "UNIQUE_ID",
                                       true);
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfUniqueConstraints() );
      // create primary key will fail because of unique constraint
      CPPUNIT_ASSERT_THROW(schemaEditor.setPrimaryKey("ID"),
                           coral::UniqueConstraintAlreadyExistingException );
      CPPUNIT_ASSERT_EQUAL(false, description.hasPrimaryKey() );
      // remove unique constraint again
      schemaEditor.setUniqueConstraint("ID",
                                       "UNIQUE_ID",
                                       false);
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfUniqueConstraints() );

      // create it again
      schemaEditor.setPrimaryKey("ID");
      CPPUNIT_ASSERT_EQUAL(true, description.hasPrimaryKey() );

      // create it again, even though it already exists
      CPPUNIT_ASSERT_THROW(schemaEditor.setPrimaryKey("ID"),
                           coral::ExistingPrimaryKeyException );
      CPPUNIT_ASSERT_EQUAL(true, description.hasPrimaryKey() );

    }


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_createForeignKey()
    {
      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( connectionString ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      coral::ITable& table = schema.tableHandle( TABLENAME1 );

      coral::ITableSchemaEditor& schemaEditor = table.schemaEditor();
      const coral::ITableDescription& description = table.description();

      // create a foreign key without errors
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfForeignKeys() );
      schemaEditor.createForeignKey( "foreignKey1",
                                     "X",
                                     TABLENAME2,
                                     "ID");
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfForeignKeys() );

      // creating the same foreign key again should fail
      CPPUNIT_ASSERT_THROW(schemaEditor.createForeignKey( "foreignKey1",
                                                          "X",
                                                          TABLENAME2,
                                                          "ID"),
                           coral::InvalidForeignKeyIdentifierException);
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfForeignKeys() );

      // drop a non existent foreign key
      CPPUNIT_ASSERT_THROW(schemaEditor.dropForeignKey( "foreignKey2" ),
                           coral::InvalidForeignKeyIdentifierException);
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfForeignKeys() );
      // drop a foreign key without errors
      schemaEditor.dropForeignKey( "foreignKey1" );
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfForeignKeys() );
      // drop the same key again
      CPPUNIT_ASSERT_THROW(schemaEditor.dropForeignKey( "foreignKey1" ),
                           coral::InvalidForeignKeyIdentifierException);
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfForeignKeys() );

      // see what happens if I create a foreign key with an invalid
      // table name, a SchemaException should be thrown
      CPPUNIT_ASSERT_THROW(schemaEditor.createForeignKey( "foreignKey1",
                                                          "X",
                                                          "NONEXISTENT",
                                                          "ID"),
                           coral::SchemaException);
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfForeignKeys() );

      // see what happens if I create a foreign key with an invalid
      // foreign column name, a SchemaException should be thrown
      CPPUNIT_ASSERT_THROW(schemaEditor.createForeignKey( "foreignKey1",
                                                          "X",
                                                          TABLENAME2,
                                                          "NONEXISTENT"),
                           coral::SchemaException);
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfForeignKeys() );
      // see what happens if I create a foreign key with an invalid
      // source column name, a SchemaException should be thrown
      CPPUNIT_ASSERT_THROW(schemaEditor.createForeignKey( "foreignKey1",
                                                          "NONEXISTENT",
                                                          TABLENAME2,
                                                          "ID"),
                           coral::InvalidColumnNameException);
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfForeignKeys() );
      session->transaction().commit();
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_createIndex()
    {
      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( connectionString ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      coral::ITable& table = schema.tableHandle( TABLENAME1 );

      coral::ITableSchemaEditor& schemaEditor = table.schemaEditor();
      const coral::ITableDescription& description = table.description();

      // create a foreign key without errors
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfIndices() );
      schemaEditor.createIndex("index1",
                               "X",
                               true);
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfIndices() );

      // creating the same foreign key again should fail
      CPPUNIT_ASSERT_THROW( schemaEditor.createIndex("index1",
                                                     "X",
                                                     true),
                            coral::InvalidIndexIdentifierException);
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfIndices() );

      // drop the index
      schemaEditor.dropIndex("index1");
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfIndices() );

      // drop the index a second time expecting the exception
      CPPUNIT_ASSERT_THROW( schemaEditor.dropIndex("index1"),
                            coral::InvalidIndexIdentifierException );
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfIndices() );

      // creating an index with nonexistent columns fails
      CPPUNIT_ASSERT_THROW( schemaEditor.createIndex("index1",
                                                     "XYZ",
                                                     true),
                            coral::InvalidColumnNameException );
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfIndices() );


      // create a uniqueConstraint for X
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfUniqueConstraints() );
      schemaEditor.setUniqueConstraint("X",
                                       "UNIQUE_X",
                                       true);
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfUniqueConstraints() );

      // creating the index now should fail
      CPPUNIT_ASSERT_THROW( schemaEditor.createIndex("index1",
                                                     "X",
                                                     true),
                            coral::UniqueConstraintAlreadyExistingException );
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfIndices() );
#if 0
      // doesn't work in oracle?
      // unless it is not unique
      schemaEditor.createIndex("index1",
                               "X",
                               false);
      CPPUNIT_ASSERT_EQUAL(1, description.numberOfIndices() );
      schemaEditor.dropIndex("index1");
#endif
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfIndices() );
      schemaEditor.setUniqueConstraint("X",
                                       "UNIQUE_X",
                                       false);
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfUniqueConstraints() );

      // try to create  uniqueConstraint for ID (primary key)
      CPPUNIT_ASSERT_THROW(schemaEditor.createIndex("index1",
                                                    "ID",
                                                    true),
                           coral::UniqueConstraintAlreadyExistingException );
      CPPUNIT_ASSERT_EQUAL(0, description.numberOfIndices() );

    }


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  };

  CPPUNIT_TEST_SUITE_REGISTRATION( DataDictionaryTest );

}

CORALCPPUNITTEST_MAIN( DataDictionaryTest )
