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

  class MultipleSchemasTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( MultipleSchemasTest );
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
      coral::ConnectionService connSvc;
      std::unique_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );

      session->transaction().start( true );

      // Use the nominal schema
      std::cout << "Tables in the nominal schema:" << std::endl;
      std::set<std::string> tableList = session->nominalSchema().listTables();
      unsigned int nTables = 0;
      for ( std::set< std::string >::const_iterator iTable = tableList.begin();
            iTable != tableList.end(); ++iTable )
      {
        if ( ++nTables <= 5 || nTables > tableList.size()-5 )
          std::cout << *iTable << std::endl;
        else if ( nTables == 6 )
          std::cout << "..." << std::endl;
      }

      // Use another schema name
      std::cout << "Tables in the other schema:" << std::endl;
      tableList = session->schema( "lcg_coral_nightly_R" ).listTables();
      nTables = 0;
      for ( std::set< std::string >::const_iterator iTable = tableList.begin();
            iTable != tableList.end(); ++iTable )
      {
        if ( ++nTables <= 5 || nTables > tableList.size()-5 )
          std::cout << *iTable << std::endl;
        else if ( nTables == 6 )
          std::cout << "..." << std::endl;
      }

      // Try to access a schema which doesn't exists
      std::cout << "Attempting to catch a specific exception for accessing an unknown schema" << std::endl;
      try
      {
        session->schema( "jkasd3y498172kjdkadkh" ).listTables();
        std::stringstream msg;
        msg << "Test failure at line " << __LINE__;
        throw std::runtime_error( msg.str() );
      }
      catch ( coral::InvalidSchemaNameException& e )
      {
        std::cout << "Exception caught: " << e.what() << std::endl;
      }
      catch ( coral::Exception& e )
      {
        std::cout << "Exception caught (expected InvalidSchemaNameException): "
                  << e.what() << std::endl;
        std::stringstream msg;
        msg << "Test failure at line " << __LINE__;
        throw std::runtime_error( msg.str() );
      }

      session->transaction().commit();

    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( MultipleSchemasTest );

}

CORALCPPUNITTEST_MAIN( MultipleSchemasTest )
