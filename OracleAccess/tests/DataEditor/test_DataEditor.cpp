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

namespace coral
{

  class DataEditorTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( DataEditorTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_Read()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_DE_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      std::cout << "About to drop previous table" << std::endl;
      schema.dropIfExistsTable( T1 );

      std::cout << "Describing new tables" << std::endl;
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
      description.insertColumn( "data0",
                                coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
      description.insertColumn( "data1",
                                coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                                2,
                                true );
      description.insertColumn( "data2",
                                coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                                321,
                                false );

      std::cout << "About to create the tables" << std::endl;
      coral::ITableDataEditor& editor = schema.createTable( description ).dataEditor();

      coral::AttributeList rowBuffer;
      editor.rowBuffer( rowBuffer );

      for ( int i = 0; i < 10; ++i )
      {
        rowBuffer["ID"].data<int>() = i;
        rowBuffer["x"].data<float>() = (float)( i + 0.01*i );
        if ( i%3 == 1 )
        {
          rowBuffer["Y"].setNull(); // the problem of the test was here
        }
        else
        {
          rowBuffer["Y"].setNull( false );
          rowBuffer["Y"].data<double>() = i + 0.001*i;
        }
        rowBuffer["Z"].data<double>() = i + 0.0001*i;
        std::ostringstream os0;
        os0 << "The data0 is " << i;
        rowBuffer["data0"].data<std::string>() = os0.str();
        std::ostringstream os1;
        os1 << i;
        rowBuffer["data1"].data<std::string>() = os1.str();
        std::ostringstream os2;
        os2 << "Hello " << 10*(i-1) + i;
        rowBuffer["data2"].data<std::string>() = os2.str();

        editor.insertRow( rowBuffer );
      }

      // Now try to insert a duplicate entry
      try
      {
        editor.insertRow( rowBuffer );
        // Throw exception if no exception was thrown
        throw std::runtime_error( "DuplicateEntryInUniqueKeyException not thrown !!!" );
      }
      catch ( coral::DuplicateEntryInUniqueKeyException& )
      {
        // Good
      }

      // Remove a few entries...
      coral::AttributeList inputData;
      inputData.extend<int>( "id" );
      inputData.begin()->data<int>() = 5;
      int rowsDeleted = editor.deleteRows( "ID<:id", inputData );
      std::cout << "Removed " << rowsDeleted << " rows" << std::endl;

      // Udpate the entries;
      inputData.extend<double>("increment");
      inputData[1].data<double>() = 1.111;
      inputData[0].data<int>() += 2;
      int rowsUpdated = editor.updateRows( "Y   = Y+ :increment", "ID>:id", inputData );
      std::cout << "Updated " << rowsUpdated << " rows" << std::endl;

      // Clean up
      schema.dropTable( T1 );

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( DataEditorTest );

}

CORALCPPUNITTEST_MAIN( DataEditorTest )
