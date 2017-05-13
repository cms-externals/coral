#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ISchema.h"
#include <string>
#include "Connection.h"
#include "SchemaTest.h"
#include "MysqlSchema.h"
#include <iostream>
#include <stdexcept>
#include "CoralBase/Exception.h"
#include "RelationalAccess/ITypeConverter.h"
#include <exception>
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/SessionException.h"

#include "../../ConnectionService/src/ConnectionService.h"
#include "../../ConnectionService/src/SessionProxy.h"
#include "../../ConnectionService/src/ConnectionPool.h"

#include "TestEnv/Testing.h"

#define LOG( msg ){ coral::MessageStream myMsg("CSERVICE"); myMsg << coral::Info << msg << coral::MessageStream::endmsg; }

// use empty namespace to avoid name conflicts
namespace {

  //-----------------------------------------------------------------------------

  class TestConnectionServicePool : public Testing
  {
  public:

    TestConnectionServicePool( const TestEnv& env )
      : Testing(env)
    {
      // open an update session
      ///createSession(0);
      // create a default table with 10 filled rows
      ///writeTableT1(getSession(), 10);
      // close update session
      ///deleteSession();
    }

    virtual ~TestConnectionServicePool()
    {
    }

    // Test for connection pool
    void run()
    {
      coral::MessageStream msgstream("CSERVICE");
      // Do a small hack to get the connection service object
      coral::ConnectionService::ConnectionService& csvc = dynamic_cast<coral::ConnectionService::ConnectionService&>(getConnectionService());
      // All without connection sharing

      // Get the configuration
      coral::ConnectionService::ConnectionServiceConfiguration& config = dynamic_cast<coral::ConnectionService::ConnectionServiceConfiguration&>(csvc.configuration());
      // Change some details for timeouts
      if( !config.isPoolAutomaticCleanUpEnabled() )
        throw std::runtime_error( "AutomaticCleanUp was not enabled" );
      // After 2 seconds the connection was defined as experired
      config.setConnectionTimeOut( 2 );

      // Clean the connection pool
      csvc.connectionPool().clearConnectionPool();
      // There should be no idle connections
      if( csvc.numberOfIdleConnectionsInPool() != 0 )
        throw std::runtime_error( "Connections in the pool are still idle" );
      // There should be no active connections
      if( csvc.numberOfActiveConnectionsInPool() != 0 )
        throw std::runtime_error( "Connections in the pool are still active" );

      std::string mi("**** Number of idle   connections");
      std::string ma("**** Number of active connections");

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)0, ma, msgstream);

      coral::ISessionProxy* session01 = newSession(0, coral::ReadOnly, false);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)1, ma, msgstream);

      coral::ISessionProxy* session02 = newSession(0, coral::ReadOnly, false);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)2, ma, msgstream);

      coral::ISessionProxy* session03 = newSession(0, coral::ReadOnly, false);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)3, ma, msgstream);

      coral::ISessionProxy* session04 = newSession(0, coral::ReadOnly, false);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)4, ma, msgstream);

      delete session04;

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)1, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)3, ma, msgstream);

      coral::ISessionProxy* session05 = newSession(0, coral::ReadOnly, false);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)4, ma, msgstream);

      delete session03;
      delete session02;

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)2, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)2, ma, msgstream);

      coral::ISessionProxy* session06 = newSession(0, coral::ReadOnly, false);
      coral::ISessionProxy* session07 = newSession(0, coral::ReadOnly, false);
      coral::ISessionProxy* session08 = newSession(0, coral::ReadOnly, false);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)5, ma, msgstream);

      delete session01;
      delete session05;
      delete session06;
      delete session07;
      delete session08;

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)5, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)0, ma, msgstream);

      // wait to the next pool clean up
      // all idle connections should be removed now
      coral::sleepSeconds(config.poolCleanUpPeriod() + 1);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)0, ma, msgstream);

      coral::ISessionProxy* session09 = newSession(0, coral::ReadOnly, false);
      delete session09;

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)1, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)0, ma, msgstream);
      // Deactivate automatic pool cleanup
      config.disablePoolAutomaticCleanUp();

      coral::sleepSeconds(config.poolCleanUpPeriod() + 1);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)1, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)0, ma, msgstream);

      config.enablePoolAutomaticCleanUp();

      coral::sleepSeconds(config.poolCleanUpPeriod() + 1);

      compareValuesAndLog(csvc.numberOfIdleConnectionsInPool(),   (size_t)0, mi, msgstream);
      compareValuesAndLog(csvc.numberOfActiveConnectionsInPool(), (size_t)0, ma, msgstream);
    }

  };

  //-----------------------------------------------------------------------------

  class TestConnectionServiceBasic : public Testing
  {
  public:

    TestConnectionServiceBasic( const TestEnv& env )
      : Testing(env)
    {
    }

    virtual ~TestConnectionServiceBasic()
    {
    }

    /*   Try to open all kind of connection types
     */
    void run()
    {
      // Without connection sharing

      // create a new update session
      coral::ISessionProxy* session01 = newSession(0, coral::Update, false);
      // create a new update session
      coral::ISessionProxy* session02 = newSession(0, coral::ReadOnly, false);

      // start update transaction
      session01->transaction().start(false);
      // start update transaction on a read only session
      try
      {
        session02->transaction().start(false);
        throw std::runtime_error( "Was able to start update transaction on read only session" );
      }
      catch(coral::InvalidOperationInReadOnlyModeException)
      {
        session02->transaction().start(true);
      }

      delete session01;
      delete session02;

      // With connection sharing

      // create a new update session
      coral::ISessionProxy* session03 = newSession(0, coral::Update, true);
      // create a new update session
      coral::ISessionProxy* session04 = newSession(0, coral::ReadOnly, true);

      // start update transaction
      session03->transaction().start(false);
      // start update transaction on a read only session
      try
      {
        session04->transaction().start(false);
        throw std::runtime_error( "Was able to start update transaction on read only session" );
      }
      catch(coral::InvalidOperationInReadOnlyModeException)
      {
        session04->transaction().start(true);
      }

      delete session03;
      delete session04;
    }

  };

}

//-----------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
  TestEnv testenv("CSERVICE");

  if(testenv.check(argc, argv))
  {
    try
    {
      //define service name 0 from dbtype 0
      testenv.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_ADMIN);
      // create basic test and run
      TestConnectionServiceBasic test_basic(testenv);
      test_basic.run();
      // create connection pool test and run
      TestConnectionServicePool test_pool(testenv);
      test_pool.run();
    }
    catch ( coral::Exception& e )
    {
      std::cerr << "CORAL Exception : " << e.what() << std::endl;
      return 1;
    }
    catch ( std::exception& e )
    {
      std::cerr << "C++ Exception : " << e.what() << std::endl;
      return 1;
    }
    catch ( ... )
    {
      std::cerr << "Unhandled exception " << std::endl;
      return 1;
    }
    std::cout << "[OVAL] Success" << std::endl;
    return 0;
  }
  return 1;
}
