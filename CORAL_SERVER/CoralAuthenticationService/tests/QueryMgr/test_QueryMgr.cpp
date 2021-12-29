// $Id: test_QueryMgr.cpp,v 1.2.2.5 2010/12/19 23:01:18 avalassi Exp $

#include <iostream>
#include "RelationalAccess/AuthenticationCredentials.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Local include files
#include "../../src/QueryMgr.h"
#include "../../src/Config.h"

// Namespace
using namespace coral;

namespace coral
{
  class CertificateData : public coral::ICertificateData
  {
  public:
    CertificateData()
      : m_distinguishedName()
      , m_FQANs()
    {};

    virtual ~CertificateData()
    {};

    /**
     * Returns the distinguished name of the certificate
     */
    virtual const std::string& distinguishedName( ) const
    {
      return m_distinguishedName;
    };

    /**
     * Returns the full qualified attribute names of the certificate
     */
    virtual const std::vector< std::string >& FQANs( ) const
    {
      return m_FQANs;
    };

    void setDistinguishedName( const std::string& dn )
    {
      m_distinguishedName = dn;
    };

    void addFQAN( const std::string& fqan )
    {
      m_FQANs.insert( m_FQANs.begin(), fqan );
    };

  protected:

    std::string m_distinguishedName;
    std::vector< std::string > m_FQANs;

  };

  class QueryMgrTest : public CoralCppUnitTest
  {

  public:

    QueryMgrTest()
      : m_schema(0)
      , m_session(0)
      , m_connService(0)
    {
    };

    CPPUNIT_TEST_SUITE( QueryMgrTest );
    CPPUNIT_TEST( test_init );
    CPPUNIT_TEST( test_addConnection_credentials );
    CPPUNIT_TEST( test_delConnection_credentials );
    CPPUNIT_TEST( test_addConnection_alias );
    CPPUNIT_TEST( test_delConnection_alias );
    CPPUNIT_TEST( test_addPermission_basic );
    CPPUNIT_TEST( test_addPermission_dn );
    CPPUNIT_TEST( test_addPermission_voms );
    CPPUNIT_TEST( test_addPermission_dn_voms );
    CPPUNIT_TEST( test_delPermission );
    CPPUNIT_TEST( test_delConnection_credentials_permissions );
    CPPUNIT_TEST_SUITE_END();

  private:

    ISchema *m_schema;
    ISessionProxy *m_session;
    ConnectionService *m_connService;

  public:

    void setUp()
    {
      if ( m_session == 0 )
      {
        m_connService = new coral::ConnectionService();
        bool sharing = false;
        if(!sharing)
        {
          coral::IConnectionServiceConfiguration& config = m_connService->configuration();
          config.disableConnectionSharing();
          config.setConnectionTimeOut( 0 );
        }

        const std::string& serviceName = getConfig().dbConnectionString();
        m_session = m_connService->connect( serviceName,
                                            coral::Update );
        if ( !m_session )
          throw coral::Exception("Could not connect to " + serviceName,"main()","demo" );
        m_schema = &m_session->nominalSchema();
      }
      // start an update transaction
      m_session->transaction().start( false /*readonly*/ );
    }

    // ------------------------------------------------------

    void tearDown()
    {
      m_session->transaction().commit();
      delete m_session;
      m_session=0;
      m_connService->purgeConnectionPool();
      delete m_connService;
      m_connService=0;
    }

    // ------------------------------------------------------

    void test_init()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );
    }

    // ------------------------------------------------------
#if 0
    void test_addConnection_connection()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      // authMechanisem and accessMode empty
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");
    }
#endif

    // ------------------------------------------------------

    void test_addConnection_alias()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      // no pcs
      try {
        mgr.addConnection("testLogConStr" /* lcString*/, "" /*pcString*/,
                          "" /*user*/, "" /*passwd*/, "" /*role*/,
                          "" /*accessMode*/,"" /*authMechanism*/ );
        CPPUNIT_FAIL("Expected exception not thrown");
      }
      catch ( coral::Exception &e ) {
        std::string expMsg="please specify a physical connection for the alias";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }

      mgr.addConnection("testLogConStr" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_lcTable.logConExists( "testLogConStr" ) )
        CPPUNIT_FAIL("test logical connection not added");

      mgr.addConnection("testLogConStr" /* lcString*/, "testPhysConStr2" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr2" ) )
        CPPUNIT_FAIL("test connection 2 not added");

      if ( !mgr.m_lcTable.logConExists( "testLogConStr" ) )
        CPPUNIT_FAIL("test logical 2 connection not added");

      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 2, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
      }

      mgr.addConnection("testLogConStr" /* lcString*/, "roTestPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "ReadOnly","" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "roTestPhysConStr" ) )
        CPPUNIT_FAIL("test connection 3 not added");
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 2, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 3, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 3 ", std::string("roTestPhysConStr"),
                                      set->replica(2).connectionString() );
      }

      mgr.addConnection("testLogConStr" /* lcString*/, "roTestPhysConStr2" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "ReadOnly","" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "roTestPhysConStr2" ) )
        CPPUNIT_FAIL("test connection 4 not added");
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 2, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 4, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("roTestPhysConStr"),
                                      set->replica(2).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("roTestPhysConStr2"),
                                      set->replica(3).connectionString() );
      }


    }

    // ------------------------------------------------------

    void test_delConnection_alias()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      mgr.addConnection("testLogConStr" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_lcTable.logConExists( "testLogConStr" ) )
        CPPUNIT_FAIL("test logical connection not added");

      // delete the only connection ----------------------------------------------------
      mgr.delConnection("testLogConStr" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*role*/ );

      if ( mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not deleted");

      if ( mgr.m_lcTable.logConExists( "testLogConStr" ) )
        CPPUNIT_FAIL("test logical connection not deleted");

      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 0, set->numberOfReplicas() );
      }


      // more than one replica
      mgr.addConnection("testLogConStr" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_lcTable.logConExists( "testLogConStr" ) )
        CPPUNIT_FAIL("test logical connection not added");

      mgr.addConnection("testLogConStr" /* lcString*/, "testPhysConStr2" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr2" ) )
        CPPUNIT_FAIL("test connection 2 not added");

      if ( !mgr.m_lcTable.logConExists( "testLogConStr" ) )
        CPPUNIT_FAIL("test logical 2 connection not added");

      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 2, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
      }

      mgr.addConnection("testLogConStr" /* lcString*/, "roTestPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "ReadOnly","" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "roTestPhysConStr" ) )
        CPPUNIT_FAIL("test connection 3 not added");
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 2, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 3, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 3 ", std::string("roTestPhysConStr"),
                                      set->replica(2).connectionString() );
      }

      mgr.addConnection("testLogConStr" /* lcString*/, "roTestPhysConStr2" /*pcString*/,
                        "" /*user*/, "" /*passwd*/, "" /*role*/,
                        "ReadOnly","" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "roTestPhysConStr2" ) )
        CPPUNIT_FAIL("test connection 4 not added");

      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 2, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 4, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(1).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 3 ", std::string("roTestPhysConStr"),
                                      set->replica(2).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 4 ", std::string("roTestPhysConStr2"),
                                      set->replica(3).connectionString() );
      }

      // delete  one connection ----------------------------------------------------
      mgr.delConnection("testLogConStr" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*role*/ );
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 1, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(0).connectionString() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 3, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("roTestPhysConStr"),
                                      set->replica(1).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("roTestPhysConStr2"),
                                      set->replica(2).connectionString() );
      }

      // delete  a read only connection ----------------------------------------------------
      mgr.delConnection("testLogConStr" /* lcString*/, "roTestPhysConStr2" /*pcString*/,
                        "" /*user*/, "" /*role*/ );
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 1, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(0).connectionString() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 2, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(0).connectionString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("roTestPhysConStr"),
                                      set->replica(1).connectionString() );
      }
      // delete  a read only connection ----------------------------------------------------
      mgr.delConnection("testLogConStr" /* lcString*/, "roTestPhysConStr" /*pcString*/,
                        "" /*user*/, "" /*role*/ );
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 1, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(0).connectionString() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 1, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 2 ", std::string("testPhysConStr2"),
                                      set->replica(0).connectionString() );
      }
      // delete  the last connection ----------------------------------------------------
      mgr.delConnection("testLogConStr" /* lcString*/, "testPhysConStr2" /*pcString*/,
                        "" /*user*/, "" /*role*/ );
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 0, set->numberOfReplicas() );
      }
      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("testLogConStr", ReadOnly, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 0, set->numberOfReplicas() );
      }

    }

    // ------------------------------------------------------

    void test_addConnection_credentials()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      // no pcs
      try {
        mgr.addConnection("" /* lcString*/, "" /*pcString*/,
                          "test" /*user*/, "pw" /*passwd*/, "" /*role*/,
                          "" /*accessMode*/,"" /*authMechanism*/ );
        CPPUNIT_FAIL("exception not thrown when adding credentials without pcs");
      }
      catch ( coral::Exception &e ) {
        std::string expMsg="to add credentials, pcs, user and password are necessary";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
      // no user
      try {
        mgr.addConnection("" /* lcString*/, "testpcs" /*pcString*/,
                          "" /*user*/, "pw" /*passwd*/, "" /*role*/,
                          "" /*accessMode*/,"" /*authMechanism*/ );
        CPPUNIT_FAIL("exception not thrown when adding credentials without pcs");
      }
      catch ( coral::Exception &e ) {
        std::string expMsg="to add credentials, pcs, user and password are necessary";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
      // no password
      try {
        mgr.addConnection("" /* lcString*/, "testpcs" /*pcString*/,
                          "user" /*user*/, "" /*passwd*/, "" /*role*/,
                          "" /*accessMode*/,"" /*authMechanism*/ );
        CPPUNIT_FAIL("exception not thrown when adding credentials without pcs");
      }
      catch ( coral::Exception &e ) {
        std::string expMsg="to add credentials, pcs, user and password are necessary";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }


      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/, "TestPw" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                            IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not added");
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           IAuthenticationCredentials::defaultRole(), 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      // custom role
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "ownerTestUser" /*user*/, "ownerTestPw" /*passwd*/, "owner" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "ownerTestUser",
                                            "owner" ) )
        CPPUNIT_FAIL("test credentials not added");

      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "owner", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("ownerTestUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("ownerTestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      // add logical service name, physical connection string and credentials at once
      mgr.addConnection("alias1" /* lcString*/, "testPhysConStr25" /*pcString*/,
                        "TestUser25" /*user*/, "TestPw25" /*passwd*/, "owner" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr25", "TestUser25",
                                            "owner" ) )
        CPPUNIT_FAIL("test credentials not added");

      if ( !mgr.m_lcTable.logConExists( "alias1" ) )
        CPPUNIT_FAIL("test logical connection not added");

      {
        std::unique_ptr<DatabaseServiceSet> set( mgr.lookup("alias1", Update, "") );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "number of replicas", 1, set->numberOfReplicas() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "replica 1 ", std::string("testPhysConStr25"),
                                      set->replica(0).connectionString() );
      }
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr25",
                                                           "owner", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestUser25"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw25"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

    }

    // ------------------------------------------------------

    void test_delConnection_credentials()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/, "TestPw" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                            IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not added");
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      mgr.delConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/,  "" /*role*/);

      if ( mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not deleted");

      if ( mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                           IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not deleted");

      // two different roles
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/, "TestPw" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "mytestUser" /*user*/, "myTestPw" /*passwd*/, "myRole" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                            IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not added");
      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "mytestUser",
                                            "myRole" ) )
        CPPUNIT_FAIL("test credentials not added");

      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "myRole", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("mytestUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("myTestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      // delete the default role by user name----------------------------------------------
      mgr.delConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/,  "" /*role*/);

      if ( mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                           IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not deleted");
      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "mytestUser",
                                            "myRole" ) )
        CPPUNIT_FAIL("test credentials not added");

      {
        CPPUNIT_ASSERT_THROW( mgr.getCredentials( "testPhysConStr", IAuthenticationCredentials::defaultRole(), 0 ),
                              coral::AuthenticationServiceException);
      };
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "myRole", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("mytestUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("myTestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/, "TestPw" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "myRole", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("mytestUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("myTestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      // delete default role by role----------------------------------------------------
      mgr.delConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/,  IAuthenticationCredentials::defaultRole() /*role*/);
      {
        CPPUNIT_ASSERT_THROW(mgr.getCredentials( "testPhysConStr",
                                                 IAuthenticationCredentials::defaultRole(), 0 ),
                             coral::AuthenticationServiceException);
      };
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "myRole", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("mytestUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("myTestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/, "TestPw" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                            IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not added");
      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "mytestUser",
                                            "myRole" ) )
        CPPUNIT_FAIL("test credentials not added");

      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "myRole", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("mytestUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("myTestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      // delete the myRole role by user name----------------------------------------------
      mgr.delConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "mytestUser" /*user*/,  "" /*role*/);

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                            IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not added");
      if ( mgr.m_crTable.existCredentials( "testPhysConStr", "mytestUser",
                                           "myRole" ) )
        CPPUNIT_FAIL("test credentials not deleted");

      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        CPPUNIT_ASSERT_THROW(mgr.getCredentials( "testPhysConStr",
                                                 "myRole", 0 ),
                             coral::AuthenticationServiceException);
      };
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "mytestUser" /*user*/, "myTestPw" /*passwd*/, "myRole" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      if ( !mgr.m_pcTable.physConExists( "testPhysConStr" ) )
        CPPUNIT_FAIL("test connection not added");

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                            IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not added");
      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "mytestUser",
                                            "myRole" ) )
        CPPUNIT_FAIL("test credentials not added");

      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "myRole", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("mytestUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("myTestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      // delete the myRole role by role name----------------------------------------------
      mgr.delConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*user*/,  "myRole" /*role*/);

      if ( !mgr.m_crTable.existCredentials( "testPhysConStr", "testUser",
                                            IAuthenticationCredentials::defaultRole() ) )
        CPPUNIT_FAIL("test credentials not added");
      if ( mgr.m_crTable.existCredentials( "testPhysConStr", "mytestUser",
                                           "myRole" ) )
        CPPUNIT_FAIL("test credentials not deleted");

      {
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        CPPUNIT_ASSERT_THROW(mgr.getCredentials( "testPhysConStr",
                                                 "myRole", 0 ),
                             coral::AuthenticationServiceException);
      };
      if ( mgr.m_lcTable.logConExists( "testLogConStr" ) )
        CPPUNIT_FAIL("test logical connection not deleted");


    }

    // ------------------------------------------------------

    void test_addPermission_basic()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/, "TestPw" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUserWriter" /*user*/, "TestPwWriter" /*passwd*/, "Writer" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      {
        // it is possible to get the credentials in local mode (no certificate passed)
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // it is possible to get the credentials in local mode (no certificate passed)
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "Writer", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserWriter"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwWriter"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // not possible to retrive credentials without permissions
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // not possible to retrive credentials without permissions
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        cert.addFQAN("testFQAN");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // not possible to retrive credentials without permissions
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        cert.addFQAN("testFQAN");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "Writer", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // not possible to retrive credentials without permissions
        CertificateData cert;
        cert.setDistinguishedName("");
        cert.addFQAN("testFQAN");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( coral::Exception &e ) {
          std::string expMsg="Empty distinguished name is not allowed!";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // not possible to retrive credentials without permissions
        CertificateData cert;
        cert.setDistinguishedName("blah");
        cert.addFQAN("");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( coral::Exception &e ) {
          std::string expMsg="Empty FQAN is not allowed!";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // not possible to retrive credentials without permissions
        CertificateData cert;
        cert.setDistinguishedName("blah");
        cert.addFQAN("blah");
        cert.addFQAN("");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( coral::Exception &e ) {
          std::string expMsg="Empty FQAN is not allowed!";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

      try {
        mgr.addPermission("" /*lcString*/, "testPhysConStr" /*pcString*/,
                          "" /*role*/,
                          "" /*dn*/, "" /*vomsFQAN*/ );
        CPPUNIT_FAIL("Expected exception not thrown");
      }
      catch ( coral::Exception &e ) {
        std::string expMsg="Please specify either voms or dn.";
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                      expMsg, std::string( e.what(), expMsg.size() ) );
        // cut CORAL info
      }
    }

    // ------------------------------------------------------

    void test_addPermission_dn()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUser" /*user*/, "TestPw" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      mgr.addConnection("" /* lcString*/, "testPhysConStr" /*pcString*/,
                        "testUserWriter" /*user*/, "TestPwWriter" /*passwd*/, "Writer" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      // add permission based on dn ------------------------------------------------
      mgr.addPermission("" /*lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*role*/,
                        "testDN" /*dn*/, "" /*vomsFQAN*/ );
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // not possible to retrive credentials without permissions different role (Writer)
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testFQAN");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "Writer", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // wrong dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

      // add a second permission based on dn ------------------------------------------------
      mgr.addPermission("" /*lcString*/, "testPhysConStr" /*pcString*/,
                        "" /*role*/,
                        "testDN_second" /*dn*/, "" /*vomsFQAN*/ );
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDN_second");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUser"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPw"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // not possible to retrive credentials without permissions different role (Writer)
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testFQAN");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "Writer", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

      // add a second permission based on dn for writer ------------------------------------------------
      mgr.addPermission("" /*lcString*/, "testPhysConStr" /*pcString*/,
                        "Writer" /*role*/,
                        "testDN_writer" /*dn*/, "" /*vomsFQAN*/ );
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "Writer", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserWriter"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwWriter"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDN_writer");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStr",
                                                           "Writer", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserWriter"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwWriter"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        try {
          mgr.getCredentials( "testPhysConStr",
                              "Writer", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
    }

    // ------------------------------------------------------

    void test_addPermission_voms()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      mgr.addConnection("" /* lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "testUserVoms" /*user*/, "TestPwVoms" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      // add permission based on voms ------------------------------------------------
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms" /*vomsFQAN*/ );
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // wrong voms and dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        cert.addFQAN("testVOMS1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong and correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVOMS1");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // 2 wrong voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVOMS1");
        cert.addFQAN("testVOMS2");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

      // add second permission based on voms ------------------------------------------------
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms_second" /*vomsFQAN*/ );
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // wrong voms and dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        cert.addFQAN("testVOMS1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong and correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVOMS1");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // 2 wrong voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVOMS1");
        cert.addFQAN("testVOMS2");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // wrong and correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVOMS1");
        cert.addFQAN("testVoms_second");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // sql injection
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVOMS2\" ) or ( \"\"=\"");
        cert.addFQAN("testVOMS1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // lots of voms, one valid
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms_second");
        for (int i = 0; i<30; i++) {
          std::stringstream voms;
          voms << "testvoms" << i;
          cert.addFQAN( voms.str());
        };
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // lots of voms, none valid
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        for (int i = 0; i<30; i++) {
          std::stringstream voms;
          voms << "testvoms" << i;
          cert.addFQAN( voms.str());
        };
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
    }

    // ------------------------------------------------------

    void test_addPermission_dn_voms()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );
      mgr.addConnection("" /* lcString*/, "testPhysConStrDnVoms" /*pcString*/,
                        "testUserDnVoms" /*user*/, "TestPwDnVoms" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      // add permission based on dn and voms ------------------------------------------------
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "testDn" /*dn*/, "testVoms" /*vomsFQAN*/ );

      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrDnVoms",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserDnVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwDnVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        try {
          mgr.getCredentials( "testPhysConStrDnVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // wrong voms and dn
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        cert.addFQAN("testVOMS1");
        try {
          mgr.getCredentials( "testPhysConStrDnVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms but not correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        try {
          mgr.getCredentials( "testPhysConStrDnVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // wrong DNand correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN1");
        cert.addFQAN("testVoms");
        try {
          mgr.getCredentials( "testPhysConStrDnVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms and dn
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");

        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrDnVoms",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserDnVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwDnVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

    }
    // ------------------------------------------------------

    void test_delPermission()
    {
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      mgr.addConnection("" /* lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "testUserVoms" /*user*/, "TestPwVoms" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      mgr.addConnection("" /* lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "testUserVomsW" /*user*/, "TestPwVomsW" /*passwd*/, "Writer" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms" /*vomsFQAN*/ );
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // wrong and correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVOMS1");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      mgr.delPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms" /*vomsFQAN*/ );
      {
        // wrong DNand correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

      // --------------------------------------------------------

      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms" /*vomsFQAN*/ );
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms1" /*vomsFQAN*/ );
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "Writer" /*role*/,
                        "" /*dn*/, "testVoms_Writer" /*vomsFQAN*/ );
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "testDn" /*dn*/, "" /*vomsFQAN*/ );
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms 1
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms1");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDn");
        cert.addFQAN("testVoms_blah");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms_Writer");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "Writer", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      //---------------------------------------------------------------

      mgr.delPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms1" /*vomsFQAN*/ );
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms 1
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDn");
        cert.addFQAN("testVoms_blah");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms_Writer");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "Writer", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      //----------------------------------------

      // this fails, unfortunately silent... FIXME
      mgr.delPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms_writer" /*vomsFQAN*/ );
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms 1
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDn");
        cert.addFQAN("testVoms_blah");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms_Writer");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "Writer", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      //----------------------------------------

      mgr.delPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "Writer" /*role*/,
                        "" /*dn*/, "testVoms_writer" /*vomsFQAN*/ );
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms 1
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDn");
        cert.addFQAN("testVoms_blah");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms_Writer");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

      //----------------------------------------

      mgr.delPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "testDn" /*dn*/, "" /*vomsFQAN*/ );
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms 1
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDn");
        cert.addFQAN("testVoms_blah");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms_Writer");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

      //----------------------------------------

      mgr.delPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms" /*vomsFQAN*/ );
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms 1
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct dn
        CertificateData cert;
        cert.setDistinguishedName("testDn");
        cert.addFQAN("testVoms_blah");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN_blah");
        cert.addFQAN("testVoms_Writer");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

    }

    // ------------------------------------------------------

    void test_delConnection_credentials_permissions()
    {
      // check that all permissions are deleted when the corresponding credentials are deleted
      QueryMgr::deleteDatabase( *m_schema );
      QueryMgr::initDatabase( *m_schema );

      QueryMgr mgr( *m_schema );

      mgr.addConnection("" /* lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "testUserVoms" /*user*/, "TestPwVoms" /*passwd*/, "" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );
      mgr.addConnection("" /* lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "testUserVomsW" /*user*/, "TestPwVomsW" /*passwd*/, "Writer" /*role*/,
                        "" /*accessMode*/,"" /*authMechanism*/ );

      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms" /*vomsFQAN*/ );
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "" /*role*/,
                        "" /*dn*/, "testVoms1" /*vomsFQAN*/ );
      mgr.addPermission("" /*lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "Writer" /*role*/,
                        "" /*dn*/, "testVoms_writer" /*vomsFQAN*/ );
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "Writer", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms1
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms1");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVoms"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms_writer");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "Writer", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      //------------------------------------------------------------------

      // delete credential
      mgr.delConnection("" /* lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "testUserVoms" /*user*/, "" /*role*/ );
      {
        // no credentials data
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", 0 );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // no credentials data
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "Writer", 0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms1
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms_writer");
        AuthenticationCredentials cred=mgr.getCredentials( "testPhysConStrVoms",
                                                           "Writer", &cert );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials user",
                                      std::string("testUserVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::userItem() ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test credentials password",
                                      std::string("TestPwVomsW"),
                                      cred.valueForItem( IAuthenticationCredentials::passwordItem() ) );
      };

      //--------------------------------------

      // delete credential
      mgr.delConnection("" /* lcString*/, "testPhysConStrVoms" /*pcString*/,
                        "testUserVomsW" /*user*/, "Writer" /*role*/ );
      {
        // no credentials data
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", 0 );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // no credentials data
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "Writer", 0 );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms1
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms1");
        try {
          mgr.getCredentials( "testPhysConStrVoms",
                              "", &cert );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };
      {
        // correct voms writer
        CertificateData cert;
        cert.setDistinguishedName("testDN");
        cert.addFQAN("testVoms_writer");
        try {
          mgr.getCredentials( "testPhysConStrVoms", "Writer", 0 );
          CPPUNIT_FAIL("Excepted exception not thrown");
        } catch ( AuthenticationServiceException &e ) {
          std::string expMsg="no credentials found";
          CPPUNIT_ASSERT_EQUAL_MESSAGE( "exception message",
                                        expMsg, std::string( e.what(), expMsg.size() ) );
          // cut CORAL info
        }
      };

    }
    // ------------------------------------------------------

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( QueryMgrTest );

}

CORALCPPUNITTEST_MAIN( QueryMgrTest )
