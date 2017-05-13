#include <exception>
#include <iomanip>
#include <iostream>
#include <vector>

#include "CoralBase/Exception.h"
#include "CoralCommon/MonitoringEvent.h"
#include "CoralCommon/Sleep.h"

using namespace coral::monitor;

double boostElapsed()
{
  boost::posix_time::ptime t0, t1;

  t0 = boost::posix_time::microsec_clock::local_time();

  // Waste some time
  coral::sys::sleep( 1 );

  t1 = boost::posix_time::microsec_clock::local_time();

  return (t1-t0).total_nanoseconds() * 1e-9;
}

int main( int, char** )
{
  try {
    std::cout.flags( std::ios::right | std::ios::hex | std::ios::showbase );
    std::cout.fill('0');
    std::cout.width(10);

    Event::TextData td( "text" );
    Event::NumericData nd( 2 );
    Event::TimeData ttd( 0xFFFFFFFF );

    std::cout << "Coral event text data:    " << td.valueAsString()  << std::endl;
    std::cout << "Coral event numeric data: " << nd.valueAsString()  << std::endl;
    std::cout << "Coral event time data:    " << ttd.valueAsString() << std::endl;

    Event::TextData::data_type text    = td;
    Event::NumericData::data_type numeric = nd;
    Event::TimeData::data_type timer   = ttd;

    std::cout << "Coral event text data:    " << text    << std::endl;
    std::cout << "Coral event numeric data: " << numeric << std::endl;
    std::cout << "Coral event time data:    " << timer   << std::endl;

    Level ml = Default;

    if( Info & ml )
      std::cout << "Info OK:  " << Info  << " " << ml << " " << (Info & ml )  << std::endl;
    if( Error & ml )
      std::cout << "Error OK: " << Error << " " << ml << " " << (Error & ml ) << std::endl;
    if( ! (Time & ml) )
      std::cout << "Time OK:  " << Time  << " " << ml << " " << (Time & ml )  << std::endl;

    typedef std::vector<Event::Record> VER;

    VER erv;

    erv.push_back( createEvent( Application, Info,    "No payload event" ) );
    erv.push_back( createEvent( Application, Info,    "App event",     "Info" ) );
    erv.push_back( createEvent( Session,     Time,    "Session event", 2      ) );
    erv.push_back( createEvent( Transaction, Warning, "Tx event",      2.2    ) );
    erv.push_back( createEvent( Session,     Time,    "Session boost event", boostElapsed()      ) );

    for( VER::iterator it = erv.begin(); it != erv.end(); ++it )
      std::cout << "Created event: "
                << (*it).m_id          << " "
                << (*it).m_source      << " " << (*it).m_type   << " "
                << (*it).m_description << " " << ((*it).hasData() ? (*it).m_data->valueAsString() : std::string("")) << std::endl;
  }
  catch ( coral::Exception& e ) {
    std::cerr << "Coral Exception " << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << "C++ standard exception " << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Unknown exception..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
