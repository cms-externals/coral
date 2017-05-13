#include <iostream>
#include <string>
#include "CoralKernel/Context.h"
#include "CoralKernel/IHandle.h"

#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IDatabaseServiceSet.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/TableDescription.h"

#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"

namespace coral
{
  class LookupServiceTest;
}

//----------------------------------------------------------------------------

class coral::LookupServiceTest : public coral::CoralCppUnitDBTest
{
  CPPUNIT_TEST_SUITE( LookupServiceTest );
  CPPUNIT_TEST( test_run );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_run()
  {
    coral::IHandle<coral::ILookupService> handle;
    handle = coral::Context::instance().query<coral::ILookupService>("CORAL/Services/XMLLookupService");

    if(!handle.isValid())
    {
      coral::Context::instance().loadComponent("CORAL/Services/XMLLookupService");
      // Try find out if timing was enabled
      handle = coral::Context::instance().query<coral::ILookupService>("CORAL/Services/XMLLookupService");

      if(!handle.isValid())
      {
        throw std::runtime_error("Can't get handle for XMLLookupService");
      }
    }

    IDatabaseServiceSet* servicesRW = handle->lookup( UrlRW(), coral::Update );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of replicas RW", servicesRW->numberOfReplicas(), 1 );

    IDatabaseServiceSet* servicesRO = handle->lookup( UrlRO(), coral::ReadOnly );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of replicas RO", servicesRO->numberOfReplicas(), 1 );

    // change the config file to something useless
    handle->setInputFileName( "/dev/null" );

    IDatabaseServiceSet* servicesNULL = handle->lookup( UrlRW(), coral::Update );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected number of replicas RW", servicesNULL->numberOfReplicas(), 0 );

  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  LookupServiceTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  ~LookupServiceTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void setUp(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void tearDown(){}

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::LookupServiceTest );

//----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( LookupServiceTest )
