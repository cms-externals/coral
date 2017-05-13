// IMPORTANT! Execute the unit test form the directory where the test
// XML files are placed!!!

#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/AuthenticationServiceException.h"

#include "CoralBase/Exception.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/Service.h"
#include "CoralKernel/IPropertyManager.h"

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

#include <iostream>
#include <stdexcept>
#include <vector>

// Forward declaration (for easier indentation)
namespace coral
{
  class XMLAuthenticationServiceTest;
}

// The test class
class coral::XMLAuthenticationServiceTest : public coral::CoralCppUnitTest
{
  CPPUNIT_TEST_SUITE (XMLAuthenticationServiceTest);
  CPPUNIT_TEST (testLoadService);
  CPPUNIT_TEST (testAuthFileChange);
  CPPUNIT_TEST_SUITE_END ();

public:

  void setUp()
  {
    coral::Context& ctx = coral::Context::instance();
    pm = &ctx.PropertyManager();
    ctx.loadComponent( "CORAL/Services/XMLAuthenticationService" );
    m_authSvc = ctx.query<coral::IAuthenticationService>();
  }

  void tearDown() {}

protected:

  void testAuthFileChange()
  {
    CPPUNIT_ASSERT(m_authSvc.isValid());
    coral::IProperty* p1 = pm->property("AuthenticationFile");
    CPPUNIT_ASSERT(p1);
    CPPUNIT_ASSERT(p1->set("authentication_for_property_test_1.xml"));
    const coral::IAuthenticationCredentials& credentials1 = m_authSvc->credentials("connection_1");
    CPPUNIT_ASSERT_EQUAL(2, credentials1.numberOfItems());
    CPPUNIT_ASSERT_EQUAL(std::string("testuser_1"), credentials1.valueForItem("user"));
    CPPUNIT_ASSERT_EQUAL(std::string("testpasswd_1"), credentials1.valueForItem("password"));
    // Set another property file
    CPPUNIT_ASSERT(p1->set("authentication_for_property_test_2.xml"));
    const coral::IAuthenticationCredentials& credentials2 = m_authSvc->credentials("connection_2");
    CPPUNIT_ASSERT_EQUAL(2, credentials2.numberOfItems());
    CPPUNIT_ASSERT_EQUAL(std::string("testuser_2"), credentials2.valueForItem("user"));
    CPPUNIT_ASSERT_EQUAL(std::string("testpasswd_2"), credentials2.valueForItem("password"));
  }

  void testLoadService()
  {
    try
    {
      std::vector< std::string> connections;
      connections.push_back( "oracle://devdb9/IOANNIS" );
      connections.push_back( "oracle://devdb10/IOANNIS" );
      CPPUNIT_ASSERT(m_authSvc.isValid());

      for ( std::vector< std::string>::const_iterator iConnection = connections.begin();
            iConnection != connections.end(); ++iConnection ) {
        std::cout << "Connection \"" << *iConnection << "\" :" << std::endl;
        const coral::IAuthenticationCredentials& credentials1 = m_authSvc->credentials( *iConnection );

        std::cout << "  default role : " << std::endl;
        int numberOfItems1 = credentials1.numberOfItems();
        for ( int i = 0; i < numberOfItems1; ++i ) {
          std::string itemName = credentials1.itemName( i );
          std::cout << "    " << itemName << " : " << credentials1.valueForItem( itemName ) << std::endl;
        }

        std::string role = "writer";
        try {
          std::cout << "  role \"" << role << "\" : " << std::endl;
          const coral::IAuthenticationCredentials& credentials2 = m_authSvc->credentials( *iConnection, role );
          if ( iConnection != connections.begin() )
            throw std::runtime_error( "Fetched invalid role, and UnknownRoleException was not thrown!" );
          int numberOfItems2 = credentials2.numberOfItems();
          for ( int i = 0; i < numberOfItems2; ++i ) {
            std::string itemName = credentials2.itemName( i );
            std::cout << "    " << itemName << " : " << credentials2.valueForItem( itemName ) << std::endl;
          }
        }
        catch( coral::UnknownRoleException& ) {
          std::cout << "    ...does not exist. Exception is has been thrown (and caught)" << std::endl;
          if ( iConnection == connections.begin() )
            throw std::runtime_error( "Received unexpected exception when retrieving a role" );
        }
      }
    }
    catch ( coral::Exception& se ) {
      std::cerr << "CORAL Exception : " << se.what() << std::endl;
      CPPUNIT_ASSERT(0);
    }
    catch ( std::exception& e ) {
      std::cerr << "Standard C++ exception : " << e.what() << std::endl;
      CPPUNIT_ASSERT(0);
    }
    catch ( ... ) {
      std::cerr << "Exception caught (...)" << std::endl;
      CPPUNIT_ASSERT(0);
    }
    CPPUNIT_ASSERT(1);
  }

private:

  coral::IHandle<coral::IAuthenticationService> m_authSvc;
  coral::IPropertyManager* pm;

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::XMLAuthenticationServiceTest );

CORALCPPUNITTEST_MAIN( XMLAuthenticationServiceTest )
