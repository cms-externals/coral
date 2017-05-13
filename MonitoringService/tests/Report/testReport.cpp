#include "RelationalAccess/MonitoringException.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"

#include "CoralBase/Exception.h"

#include "CoralKernel/Context.h"

#include <iostream>
#include <stdexcept>
#include <memory>

int main( int, char** )
{
  try
  {
    coral::Context& ctx = coral::Context::instance();

    ctx.loadComponent( "CORAL/Services/MonitoringService" );

    coral::IHandle<coral::monitor::IMonitoringService> monsvc = ctx.query<coral::monitor::IMonitoringService>();

    if( ! monsvc.isValid() )
      throw coral::Exception( "Could not load CORAL Monitoring Service!", "Test Monitoring Service", "Main Program" );

    const coral::IMonitoringReporter& monreport = monsvc->reporter();

    monsvc->setLevel( "first" , coral::monitor::Default );
    monsvc->enable( "first" );

    // In Default level only the events of types: Info and Error are recorded.
    monsvc->record( "first",
                    coral::monitor::Application,
                    coral::monitor::Info,
                    "This the first event at the Default monitoring level without payload, e.g. only time is recorded" );

    try
    {
      // This should generate an error exception event due to uninitialized session monitoring
      monreport.report( "invalid" );
    }
    catch ( const coral::MonitoringException& e )
    {
      monsvc->record( "first",
                      coral::monitor::Application,
                      coral::monitor::Error,
                      "Application exception",
                      e.what() );
    }

    monsvc->record( "first",
                    coral::monitor::Application,
                    coral::monitor::Warning,
                    "This event will not be recorded neither the event ID will not grow" );

    monsvc->disable( "first" );

    monsvc->record( "first",
                    coral::monitor::Application,
                    coral::monitor::Info,
                    "This event will not be recorded neither the event ID will grow" );

    monsvc->enable( "first" );
    monsvc->record( "first",
                    coral::monitor::Application,
                    coral::monitor::Info,
                    "This the last event at the Default monitoring level without payload and should have ID 3" );

    // This should report 3 events for the first session
    monreport.report("first");

    monsvc->disable( "first" );

    /// THE SECOND SESSION AT DEBUG LEVEL
    monsvc->setLevel( "second",
                      coral::monitor::Debug   );

    monsvc->enable( "second" );

    monsvc->record( "second",
                    coral::monitor::Application,
                    coral::monitor::Config,
                    "This the first event at the Debug monitoring level with payload",
                    1000000 );

    // Retrieve the list of monitored data sources
    std::set< std::string > mds = monreport.monitoredDataSources();

    std::cout << "The monitored data sources are: " << std::endl;

    for( std::set< std::string >::iterator mdsit = mds.begin(); mdsit != mds.end(); ++mdsit )
    {
      std::cout << "Data source: " << (*mdsit) << std::endl;
    }

    std::cout << std::endl;

    try
    {
      // This should generate an error exception event due to uninitialized session monitoring
      monreport.report( "impossible to report anything" );
    }
    catch ( const coral::MonitoringException& e )
    {
      monsvc->record( "second",
                      coral::monitor::Application,
                      coral::monitor::Warning,
                      "Application exception",
                      e.what() );
    }

    monsvc->record( "second",
                    coral::monitor::Application,
                    coral::monitor::Time,
                    "This event will not be recorded neither the event ID will not grow" );

    monsvc->disable( "second" );

    monsvc->record( "second",
                    coral::monitor::Application,
                    coral::monitor::Time,
                    "This event will not be recorded neither the event ID will grow" );

    monsvc->enable( "second" );
    monsvc->record( "second",
                    coral::monitor::Application,
                    coral::monitor::Info,
                    "This the last event at the Debug monitoring level without payload and should have ID 6" );

    // This should report 3 events for the second session
    monreport.report("second");

    // This should simply report everyhting
    monreport.report( coral::monitor::Trace );

    mds = monreport.monitoredDataSources();

    std::cout << "The monitored data sources are: " << std::endl;

    for( std::set< std::string >::iterator mdsit = mds.begin(); mdsit != mds.end(); ++mdsit )
    {
      std::cout << "Data source: " << (*mdsit) << std::endl;
    }

    std::cout << std::endl;
  }
  catch ( coral::Exception& e ) {
    std::cerr << "CORAL Exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << "C++ Exception : " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Unknown exception ..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
