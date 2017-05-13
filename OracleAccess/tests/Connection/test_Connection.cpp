// Include files
#include <cstdlib>
#include <memory>
#include "CoralBase/TimeStamp.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralCommon/Utilities.h"
#include "CoralKernel/Context.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/RelationalServiceException.h"

namespace
{

//FIXME, the following methods should be moved to a common space

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  /* get a new session from service description and credentials */
  coral::ISession*
  getSession( const coral::IDatabaseServiceDescription& svdesc, const coral::IAuthenticationCredentials& creds )
  {
    // Get the global context
    coral::Context& context = coral::Context::instance();
    // Load the oracle component
    context.loadComponent( "CORAL/RelationalPlugins/oracle" );
    coral::IHandle<coral::IRelationalDomain> iHandle = context.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/oracle" );
    if ( ! iHandle.isValid() )
    {
      throw coral::NonExistingDomainException( "oracle" );
    }

    std::pair<std::string, std::string> connectionAndSchema = iHandle->decodeUserConnectionString( svdesc.connectionString() );

    coral::IConnection* connection = iHandle->newConnection( connectionAndSchema.first );

    if ( ! connection->isConnected() )
      connection->connect();

    coral::ISession* session = connection->newSession( connectionAndSchema.second );
    if ( session )
    {
      session->startUserSession( creds.valueForItem( creds.userItem() ), creds.valueForItem( creds.passwordItem() ) );
    }
    return session;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  const coral::IDatabaseServiceDescription&
  getServiceDescription( const std::string& connectionString )
  {
    // Get the global context
    coral::Context& context = coral::Context::instance();

    context.loadComponent( "CORAL/Services/XMLLookupService" );
    coral::IHandle<coral::ILookupService> lookupSvc = context.query<coral::ILookupService>( "CORAL/Services/XMLLookupService" );

    if ( ! lookupSvc.isValid() )
    {
      throw std::runtime_error( "Can't load Lookup Service" );
    }

    coral::IDatabaseServiceSet* dbSet = lookupSvc->lookup( connectionString );

    if( dbSet->numberOfReplicas() == 0 )
    {
      throw std::runtime_error( "No replicas found" );
    }

    return dbSet->replica( 0 );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  const coral::IAuthenticationCredentials&
  getCredentials( const coral::IDatabaseServiceDescription& svdesc )
  {
    // Get the global context
    coral::Context& context = coral::Context::instance();

    context.loadComponent( "CORAL/Services/XMLAuthenticationService" );
    coral::IHandle<coral::IAuthenticationService> authSvc = context.query<coral::IAuthenticationService>(  "CORAL/Services/XMLAuthenticationService" );

    if ( ! authSvc.isValid() )
    {
      throw std::runtime_error( "Can't load Authentication Service" );
    }

    return authSvc->credentials( svdesc.connectionString() );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

} // empty namespace

namespace coral
{

  class ConnectionTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( ConnectionTest );
    CPPUNIT_TEST( test_SimpleConnection );
    CPPUNIT_TEST( test_expiredPassword_bug77440 );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_SimpleConnection()
    {
      const coral::IDatabaseServiceDescription& svdesc = getServiceDescription("CORAL-Oracle-lcgnight/admin");

      const coral::IAuthenticationCredentials& creds = getCredentials( svdesc );

      coral::ISession* session = getSession( svdesc, creds );

      session->transaction().start();

      const std::set<std::string>& tables = session->nominalSchema().listTables();

      std::cout << "Retrieved " << tables.size() << " tables:" << std::endl;
      unsigned int i = 0;
      for ( std::set<std::string>::const_iterator it=tables.begin(); it != tables.end(); ++it, ++i )
      {
        if ( i<10 || i>tables.size()-10 ) 
          std::cout << "  " << *it << std::endl;
        else if ( i==10 )
          std::cout << "  ..." << std::endl;
      }

      session->transaction().commit();

      session->transaction().start( true );

      session->endUserSession();
      if ( session->isUserSessionActive() )
      {
      throw std::runtime_error( "Session appears to be up and running !!!" );
      }

      session->startUserSession( creds.valueForItem( creds.userItem() ), creds.valueForItem( creds.passwordItem() ) );

      if ( ! session->isUserSessionActive() )
      {
        throw std::runtime_error( "Connection lost..." );
      }

      session->transaction().start();

      delete session;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_expiredPassword_bug77440()
    {
      coral::ConnectionService connSvc;
      coral::IConnectionServiceConfiguration& config = connSvc.configuration();
      int timeout = 20; // Reduce timeout from 300 to 20 seconds (bug #78276)
      config.setConnectionRetrialPeriod( 4 );
      config.setConnectionRetrialTimeOut( timeout );
      long long start = coral::TimeStamp::now(true).total_nanoseconds();
      try
      {
        std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/expired" ) );
      }
      catch( coral::ConnectionNotAvailableException& e )
      {
        std::cout << "ConnectionNotAvailableException caught: " << e.what() << std::endl;
        long long end = coral::TimeStamp::now(true).total_nanoseconds();
        if ( (end-start)/1000000000LL < timeout )
        {
          std::cout << "Exception caught within timeout: OK" << std::endl;
        }
        else
        {
          std::cout << "ERROR! Exception caught after timeout exceeded" << std::endl;
          throw;
        }        
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void setUp()
    {
      coral::ConnectionService connSvc;
      coral::IConnectionServiceConfiguration& config = connSvc.configuration();
      static int defPeriod = config.connectionRetrialPeriod();
      static int defTimeOut = config.connectionRetrialTimeOut();
      config.setConnectionRetrialPeriod( defPeriod );
      config.setConnectionRetrialTimeOut( defTimeOut );
    }
    
    void tearDown()
    {
    }
    
  };

  CPPUNIT_TEST_SUITE_REGISTRATION( ConnectionTest );

}

CORALCPPUNITTEST_MAIN( ConnectionTest )
