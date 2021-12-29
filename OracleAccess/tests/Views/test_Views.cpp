// Include files
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
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
#include "RelationalAccess/IView.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

namespace coral
{

  class ViewsTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( ViewsTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_VT_T1" );
      std::string V1 = BuildUniqueTableName( "ORA_UT_VT_V1" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      std::cout << "About to drop previous tables and views" << std::endl;
      schema.dropIfExistsView( V1 );
      schema.dropIfExistsTable( T1 );

      std::cout << "Creating a new table" << std::endl;

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
      schema.createTable( description );

      std::cout << "About to create a view" << std::endl;
      coral::IViewFactory* factory = schema.viewFactory();
      factory->addToOutputList( "tt.x", "x" );
      factory->addToOutputList( "tt.Y", "y" );
      factory->addToTableList( T1, "tt" );
      factory->setCondition( "ID > 2", coral::AttributeList() );
      factory->create( V1 );

      delete factory;

      session->transaction().commit();

      coral::sleepSeconds(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_Read()
    {
      std::string V1 = BuildUniqueTableName( "ORA_UT_VT_V1" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start( true );

      coral::IView& view = session->nominalSchema().viewHandle( V1 );
      std::cout << "View definition : \"" << view.definition() << "\"" << std::endl;

      int numberOfColumns = view.numberOfColumns();
      for ( int i = 0; i < numberOfColumns; ++i )
      {
        const coral::IColumn& column = view.column( i );
        std::cout << "  " << column.name() << " (" << column.type() << ")";
        if ( column.isNotNull() ) std::cout << " NOT NULL";
        std::cout << std::endl;
      }

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
      std::string T1 = BuildUniqueTableName( "ORA_UT_VT_T1" );
      std::string V1 = BuildUniqueTableName( "ORA_UT_VT_V1" );

      coral::ConnectionService connSvc;

      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      schema.dropView( V1 );
      schema.dropTable( T1 );

      session->transaction().commit();
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( ViewsTest );

}

CORALCPPUNITTEST_MAIN( ViewsTest )
