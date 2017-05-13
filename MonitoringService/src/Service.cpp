#include "Service.h"

#include "RelationalAccess/MonitoringException.h"

#include "CoralBase/MessageStream.h"

namespace coral
{
  namespace monitor
  {
    SessionMonitor::SessionMonitor()
      : active( false ), level( coral::monitor::Default ), stream()
    {
    }

    SessionMonitor::SessionMonitor( bool act, coral::monitor::Level lvl )
      : active( act ), level( lvl ), stream()
    {
    }

    Service::Service( const std::string& componentName )
      : coral::Service( componentName ),
        m_events()
    {
    }

    Service::~Service()
    {
    }

    void Service::setLevel( const std::string& contextKey, coral::monitor::Level level )
    {
      Repository::iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );
      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
      {
        m_events[distinctKey] = SessionMonitor();
        m_monitoredDS.insert( distinctKey );
      }

      m_events[distinctKey].level = level;

      if( level == coral::monitor::Off )
      {
        m_events[distinctKey].active = false;
      }
      else
      {
        m_events[distinctKey].active = true;
      }
    }

    coral::monitor::Level Service::level( const std::string& contextKey ) const
    {
      Repository::const_iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::level", this->name() );

      return (*rit).second.level;
    }

    bool Service::active( const std::string& contextKey ) const
    {
      Repository::const_iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::active", this->name() );

      return (*rit).second.active;
    }

    void Service::enable( const std::string& contextKey )
    {
      Repository::iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::enable", this->name() );

      (*rit).second.active = true;
    }

    void
    Service::disable( const std::string& contextKey )
    {
      Repository::iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::disable", this->name() );

      (*rit).second.active = false;
    }

    void
    Service::record( const std::string& contextKey, Source source, coral::monitor::Type type, const std::string& description )
    {
      Repository::iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description ) );
      }
    }

    void
    Service::record( const std::string& contextKey, Source source, coral::monitor::Type type, const std::string& description, int data )
    {
      long long int payloaddata = data;
      record( contextKey, source, type, description, payloaddata );
    }

    void
    Service::record( const std::string& contextKey, Source source, coral::monitor::Type type, const std::string& description, long long int data )
    {
      Repository::iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
      }
    }

    void
    Service::record( const std::string& contextKey, Source source, coral::monitor::Type type, const std::string& description, double data )
    {
      Repository::iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
      }
    }

    void
    Service::record( const std::string& contextKey, Source source, coral::monitor::Type type, const std::string& description, const std::string& data )
    {
      Repository::iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
      }
    }

    const coral::IMonitoringReporter& Service::reporter() const
    {
      //std::vector< seal::IHandle<coral::IMonitoringReporter> > rv;
      //this->localContext()->query( rv );

      //if( rv.empty() )
      return( static_cast<const coral::IMonitoringReporter&>(*this) );

      //return *(rv[0]);
    }

    // The coral::IMonitoringReporter interface implementation

    std::set< std::string > Service::monitoredDataSources() const
    {
      return m_monitoredDS;
    }

    void Service::report( unsigned int /*level*/ ) const
    {
      Repository::const_iterator rit;
      coral::MessageStream log( "coral::MonitoringService" );

      // Dummy reporting so far
      for( rit = m_events.begin(); rit != m_events.end(); ++rit )
        reportForSession( rit, log );
    }

    void Service::report( const std::string& contextKey, unsigned int /* level */ ) const
    {
      Repository::const_iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::record", this->name() );

      // Dummy reporting so far
      coral::MessageStream log( "coral::MonitoringService" );

      for(rit = m_events.begin(); rit != m_events.end(); ++rit)
	reportForSession( rit, log );
    }

    void Service::reportToOutputStream( const std::string& contextKey, std::ostream& os, unsigned int /* level */ ) const
    {
      Repository::const_iterator rit;
      std::string distinctKey = distinctContextKey( contextKey );

      if( ( rit = m_events.find( distinctKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + distinctKey + " not initialized...", "MonitoringService::record", this->name() );

      // Dummy reporting so far
      coral::MessageStream log(  "coral::MonitoringService" );

      for(rit = m_events.begin(); rit != m_events.end(); ++rit)
	reportForSession( rit, os );
    }

    void Service::setOutputType( ReportType /*rp*/, const std::string& /*fn*/ )
    {
      // FIXME!!!
    }

    void Service::reportOnEvent( EventStream::const_iterator& it, std::ostream& os ) const
    {
      //      os << (*it).m_id << " " << (*it).m_time.time().format (true,  "%H:%M:%S %d.%m. %Y")
      os << (*it).m_id << " "
         << (*it).m_time.hour()   << ":"
         << (*it).m_time.minute() << ":"
         << (*it).m_time.second() << " "
         << (*it).m_time.day()    << "."
         << (*it).m_time.month()  << "."
         << (*it).m_time.year()   << " "
         << " " << (*it).m_source << " " << (*it).m_type << " " << (*it).m_description
         << " " << ((*it).hasData() ? (*it).m_data->valueAsString() : std::string("no payload")) << std::endl;
    }

    void Service::reportOnEvent( EventStream::const_iterator& it, coral::MessageStream& os ) const
    {
      //      os << (*it).m_id << " " << (*it).m_time.time().format (true,  "%H:%M:%S %d.%m. %Y")
      os << (*it).m_id << " "
         << (*it).m_time.hour()   << ":"
         << (*it).m_time.minute() << ":"
         << (*it).m_time.second() << " "
         << (*it).m_time.day()    << "."
         << (*it).m_time.month()  << "."
         << (*it).m_time.year()   << " "
         << " " << (*it).m_source << " " << (*it).m_type << " " << (*it).m_description
         << " " << ((*it).hasData() ? (*it).m_data->valueAsString() : std::string("no payload")) << coral::MessageStream::endmsg;
    }

    void Service::reportForSession( Repository::const_iterator& it, std::ostream& os ) const
    {
      os << "Session: " << (*it).first        << std::endl;
      os << "Level:   " << (*it).second.level << std::endl;

      const EventStream& evsref = (*it).second.stream;

      for( EventStream::const_iterator evit = evsref.begin(); evit != evsref.end(); ++evit )
      {
        reportOnEvent( evit, os );
      }
    }

    void Service::reportForSession( Repository::const_iterator& it, coral::MessageStream& os ) const
    {
      const EventStream& evsref = (*it).second.stream;

      os << "Session: " << (*it).first        << " monitored at level: ";

      std::string lvl;
      switch( (*it).second.level )
      {
      case (coral::monitor::Off)     : lvl = "Off"; break;
      case (coral::monitor::Minimal) : lvl = "Minimal"; break;
      case (coral::monitor::Default) : lvl = "Default"; break;
      case (coral::monitor::Debug)   : lvl = "Debug"; break;
      case (coral::monitor::Trace)   : lvl = "Trace"; break;
      };

      os << lvl << " has recorded " << evsref.size() << " events" << coral::MessageStream::endmsg;


      for( EventStream::const_iterator evit = evsref.begin(); evit != evsref.end(); ++evit )
      {
        reportOnEvent( evit, os );
      }
    }

    const std::string Service::distinctContextKey( const std::string& contextKey ) const
    {
      if ( contextKey.find( "oracle" ) != 0 )
      {
        return contextKey;
      }
      else
      {
        std::string distinctKey = "";
        for ( std::string::size_type i = 0; i < contextKey.size(); ++i )
          distinctKey += std::tolower( contextKey[i], std::locale::classic() );
        return distinctKey;
      }
    }

  } // namespace monitor
} // namespace coral
