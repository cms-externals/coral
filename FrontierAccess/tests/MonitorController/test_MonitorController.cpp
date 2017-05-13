//-----------------------------------------------------------------------------
// This test is aimed at checking if monitoring of the FrontierAccess plugin
// is able to produce a standard output report containing the operation
// summary of the session.
//-----------------------------------------------------------------------------

// Include files
#include <cstdlib>
#include <memory>
#include <string>
#include "CoralBase/TimeStamp.h"
#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"
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
#include "RelationalAccess/IMonitoringReporter.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/RelationalServiceException.h"

namespace coral
{

  class MonitorControllerTest : public CoralCppUnitDBTest
  {

    CPPUNIT_TEST_SUITE( MonitorControllerTest );
    CPPUNIT_TEST( test_MonitorController_frontier );
    CPPUNIT_TEST_SUITE_END();

  public:

    const IDatabaseServiceDescription&
    getServiceDescription( const std::string& connectionString,
                           IConnectionServiceConfiguration& /*config*/ )
    {
      // Get the global context
      Context& context = Context::instance();
      context.loadComponent( "CORAL/Services/XMLLookupService" );
      IHandle<ILookupService> lookupSvc = context.query<ILookupService>( "CORAL/Services/XMLLookupService" );
      if ( ! lookupSvc.isValid() )
      {
        throw std::runtime_error( "Can't load Lookup Service" );
      }
      IDatabaseServiceSet* dbSet = lookupSvc->lookup( connectionString, ReadOnly );
      if( dbSet->numberOfReplicas() == 0 )
      {
        throw std::runtime_error( "No replicas found" );
      }
      return dbSet->replica( 0 );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_MonitorController_frontier()
    {

      //----- Create a Context -----
      Context& context = Context::instance();
      //const std::set<std::string>& comps = context.knownComponents(); // remove build warning
      context.knownComponents();

      //----- Configure the Monitoring Service
      IConnectionServiceConfiguration& config = connSvc().configuration();
      config.setMonitoringService( "CORAL/Services/MonitoringService" );
      config.setMonitoringLevel(monitor::Trace);
      monitor::Level monLev = config.monitoringLevel();
      std::string lvl;
      switch( monLev ) {
      case (monitor::Off)     : lvl = "Off"; break;
      case (monitor::Minimal) : lvl = "Minimal"; break;
      case (monitor::Default) : lvl = "Default"; break;
      case (monitor::Debug)   : lvl = "Debug"; break;
      case (monitor::Trace)   : lvl = "Trace"; break;
      default: lvl = "";
      };

      // ----- Create the Session -----
      std::auto_ptr<ISessionProxy> session( connSvc().connect( UrlRO(), ReadOnly ) );

      // ----- Start the transaction -----
      session->transaction().start(true);
      //ISchema& schema = session->nominalSchema(); // remove build warning
      session->nominalSchema();

      // ----- Commit the transaction -----
      session->transaction().commit();

      // ----- Create a report -----
      std::string physConnectionString = getServiceDescription(UrlRO(), config).connectionString();
      IMonitoringReporter const & reporter = connSvc().monitoringReporter();
      //reporter.report( physConnectionString, monitor::Trace );
      reporter.reportToOutputStream( physConnectionString, std::cout , monitor::Trace );
      std::cout << std::endl;

    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void setUp()
    {
      ConnectionService connSvc;
      IConnectionServiceConfiguration& config = connSvc.configuration();
      static int defPeriod = config.connectionRetrialPeriod();
      static int defTimeOut = config.connectionRetrialTimeOut();
      config.setConnectionRetrialPeriod( defPeriod );
      config.setConnectionRetrialTimeOut( defTimeOut );
    }

    void tearDown()
    {
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( MonitorControllerTest );

}

CORALCPPUNITTEST_MAIN( MonitorControllerTest )
