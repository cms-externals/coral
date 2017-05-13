// IMPORTANT! Execute the unit test form the directory where the test
// XML files are placed!!!

#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"

#include "CoralKernel/Context.h"
#include "CoralKernel/IPropertyManager.h"

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

void reportReplica(const coral::IDatabaseServiceDescription& replica,
                   const std::string& logicalName) {
  std::cout << logicalName << " : " << replica.connectionString() << ", "
            << replica.authenticationMechanism() << ", "
            << ( (replica.accessMode() == coral::ReadOnly ) ? std::string("Read-Only")
                 : std::string("Update") ) << std::endl;
  std::string connectionRetrialPeriodParam =
    replica.serviceParameter("ConnectionRetrialPeriod");
  std::string connectionRetrialTimeOutParam =
    replica.serviceParameter("ConnectionRetrialTimeOut");
  std::string connectionTimeOutParam =
    replica.serviceParameter("ConnectionTimeOut");
  std::string missingConnectionExclusionTimeParam =
    replica.serviceParameter("MissingConnectionExclusionTime");
  if (!connectionRetrialPeriodParam.empty())
    std::cout << "Par \"ConnectionRetrialPeriodParam\"="
              <<connectionRetrialPeriodParam<<std::endl;
  if (!connectionRetrialTimeOutParam.empty())
    std::cout << "Par \"ConnectionRetrialTimeOutParam\"="
              <<connectionRetrialTimeOutParam<<std::endl;
  if (!connectionTimeOutParam.empty())
    std::cout << "Par \"ConnectionTimeOutParam\"="<<connectionTimeOutParam
              <<std::endl;
  if (!missingConnectionExclusionTimeParam.empty())
    std::cout << "Par \"MissingConnectionExclusionTimeParam\"="
              <<missingConnectionExclusionTimeParam<<std::endl;
  std::cout << "----------------------------------------"<<std::endl;
}

// Forward declaration (for easier indentation)
namespace coral
{
  class XMLLookupServiceTest;
}

// The test class
class coral::XMLLookupServiceTest : public coral::CoralCppUnitTest
{
  CPPUNIT_TEST_SUITE (XMLLookupServiceTest);
  CPPUNIT_TEST (testLoadService);
  CPPUNIT_TEST (testLookupFileChange);
  CPPUNIT_TEST_SUITE_END ();

public:
  void setUp(void)
  {
    coral::Context& ctx = coral::Context::instance();
    pm = &ctx.PropertyManager();
    ctx.loadComponent("CORAL/Services/XMLLookupService");
    lookupSvcHandle = ctx.query<coral::ILookupService>();
  }

  void tearDown(void) {}

protected:
  void testLookupFileChange()
  {
    CPPUNIT_ASSERT(lookupSvcHandle.isValid());
    coral::IProperty* p1 = pm->property("DBLookupFile");
    CPPUNIT_ASSERT(p1);
    CPPUNIT_ASSERT(p1->set("dblookup_for_property_test_1.xml"));
    coral::IDatabaseServiceSet* dbSet = lookupSvcHandle->lookup("testservice_1");
    CPPUNIT_ASSERT(dbSet);
    CPPUNIT_ASSERT_EQUAL(1, dbSet->numberOfReplicas());
    delete dbSet;
    // Set another property file
    CPPUNIT_ASSERT(p1->set("dblookup_for_property_test_2.xml"));
    dbSet = lookupSvcHandle->lookup("testservice_2");
    CPPUNIT_ASSERT(dbSet);
    CPPUNIT_ASSERT_EQUAL(2, dbSet->numberOfReplicas());
    delete dbSet;
  }

  void testLoadService()
  {
    CPPUNIT_ASSERT(lookupSvcHandle.isValid());
    try {
      coral::ILookupService& lookupSvc = *lookupSvcHandle;
      coral::IDatabaseServiceSet* dbSet = lookupSvc.lookup( "myservice0" );
      int numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 0 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice1" );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 2 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice1" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice2" );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 1 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice2" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice1", coral::ReadOnly );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 3 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice1" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice2", coral::ReadOnly );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 3 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice2" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice2", coral::ReadOnly, "password" );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 2 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice2" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice3");
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 2 ) {
        std::cout << "Number of replica="<<numberOfReplicas<<std::endl;
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice3" );
      delete dbSet;

      std::cout << "Trying to load the dblookup.xml via HTTP..." << std::endl;

      // Try the HTTP based file access

      lookupSvc.setInputFileName( std::string( "http://rado.web.cern.ch/rado/dblookup.xml" ) );

      dbSet = lookupSvc.lookup( "myservice0" );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 0 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice1" );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 2 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice1" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice2" );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 1 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice2" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice1", coral::ReadOnly );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 3 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice1" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice2", coral::ReadOnly );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 3 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice2" );
      delete dbSet;

      dbSet = lookupSvc.lookup( "myservice2", coral::ReadOnly, "password" );
      numberOfReplicas = dbSet->numberOfReplicas();
      if ( numberOfReplicas != 2 ) {
        delete dbSet;
        throw std::runtime_error( "Unexpected number of replicas retrieved" );
      }
      for ( int i = 0; i < numberOfReplicas; ++i ) reportReplica( dbSet->replica( i ), "myservice2" );
      delete dbSet;

    } catch ( std::exception& e ) {
      std::cerr << "Standard C++ exception : " << e.what() << std::endl;
      CPPUNIT_ASSERT(0);
    }
    catch ( ... ) {
      std::cerr << "Exception caught (...)" << std::endl;
      CPPUNIT_ASSERT(0);
    }
    std::cout << "[OVAL] Success" << std::endl;
    CPPUNIT_ASSERT(1);
  }

private:
  coral::IHandle<coral::ILookupService> lookupSvcHandle;
  coral::IPropertyManager* pm;
};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::XMLLookupServiceTest );

CORALCPPUNITTEST_MAIN( XMLLookupServiceTest )
