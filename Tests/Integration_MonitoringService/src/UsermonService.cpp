#include "UsermonService.h"

#include "RelationalAccess/MonitoringException.h"

#ifdef CORAL_SEALED

#include "SealKernel/MessageStream.h"

DEFINE_SEAL_COMPONENT ( usermon::UsermonService, "CORAL/Services/UserMonitoringService" )

  namespace usermon
  {
    SessionMonitor::SessionMonitor()
      : active( false ), level( coral::monitor::Default ), stream()
    {
    }

    SessionMonitor::SessionMonitor( bool act, coral::monitor::Level lvl )
      : active( act ), level( lvl ), stream()
    {
    }

    UsermonService::UsermonService( seal::Context * c )
      : seal::Service( c, classContextKey() ),
        m_events()
    {
    }

    UsermonService::UsermonService( seal::Context* c , const std::string& key )
      : seal::Service( c, key ),
        m_events()
    {
    }

    UsermonService::~UsermonService()
    {
    }

    void UsermonService::setLevel( const std::string& contextKey, coral::monitor::Level level )
    {
      Repository::iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      {
        m_events[contextKey] = SessionMonitor();
        m_monitoredDS.insert( contextKey );
      }

      m_events[contextKey].level = level;

      if( level == coral::monitor::Off )
      {
        m_events[contextKey].active = false;
      }
      else
      {
        m_events[contextKey].active = true;
      }

      seal::MessageStream log( this->localContext(), "UsermonService", seal::Msg::Info );

      log << seal::Msg::Info << "Setting monitoring level for connection " << contextKey << " to " << level << seal::flush;
    }

    coral::monitor::Level UsermonService::level( const std::string& contextKey ) const
    {
      Repository::const_iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::level", this->name() );

      return (*rit).second.level;
    }

    bool UsermonService::active( const std::string& contextKey ) const
    {
      Repository::const_iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::active", this->name() );

      return (*rit).second.active;
    }

    void UsermonService::enable( const std::string& contextKey )
    {
      Repository::iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::enable", this->name() );

      (*rit).second.active = true;
    }

    void
    UsermonService::disable( const std::string& contextKey )
    {
      Repository::iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::disable", this->name() );

      (*rit).second.active = false;
    }

    void
    UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description )
    {
      Repository::iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description ) );
      }
    }

    void
    UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, int data )
    {
      Repository::iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
      }
    }

    void
    UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, double data )
    {
      Repository::iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
      }
    }

    void
    UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, const std::string& data )
    {
      Repository::iterator rit;

      if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", this->name() );

      bool active = (*rit).second.active;
      coral::monitor::Level level  = (*rit).second.level;

      if( active && (type & level) )
      {
        (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
      }
    }

    const coral::IMonitoringReporter& UsermonService::reporter() const
    {
      std::vector< seal::IHandle<coral::IMonitoringReporter> > rv;
      this->localContext()->query( rv );

      if( rv.empty() )
        throw std::logic_error( "usermon::UsermonService : No user defined reporting facility installed" );

      return *(rv[0]);
    }

  } // namespace usermon

#else

#include "CoralBase/MessageStream.h"

namespace usermon
{
  SessionMonitor::SessionMonitor()
    : active( false ), level( coral::monitor::Default ), stream()
  {
  }

  SessionMonitor::SessionMonitor( bool act, coral::monitor::Level lvl )
    : active( act ), level( lvl ), stream()
  {
  }

  UsermonService::UsermonService()
    : m_events()
  {
  }

  UsermonService::~UsermonService()
  {
  }

  void UsermonService::setReporter( coral::IMonitoringReporter* reporter )
  {
    m_reporter = reporter;
  }

  void UsermonService::setLevel( const std::string& contextKey, coral::monitor::Level level )
  {
    Repository::iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
    {
      m_events[contextKey] = SessionMonitor();
      m_monitoredDS.insert( contextKey );
    }

    m_events[contextKey].level = level;

    if( level == coral::monitor::Off )
    {
      m_events[contextKey].active = false;
    }
    else
    {
      m_events[contextKey].active = true;
    }

    coral::MessageStream log( "coral::Tests::UsermonService" );

    log << coral::Info << "Setting monitoring level for connection " << contextKey << " to " << level << coral::MessageStream::endmsg;
  }

  coral::monitor::Level UsermonService::level( const std::string& contextKey ) const
  {
    Repository::const_iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::level", name() );

    return (*rit).second.level;
  }

  bool UsermonService::active( const std::string& contextKey ) const
  {
    Repository::const_iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::active", name() );

    return (*rit).second.active;
  }

  void UsermonService::enable( const std::string& contextKey )
  {
    Repository::iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::enable", name() );

    (*rit).second.active = true;
  }

  void
  UsermonService::disable( const std::string& contextKey )
  {
    Repository::iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::disable", name() );

    (*rit).second.active = false;
  }

  void
  UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description )
  {
    Repository::iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", name() );

    bool active = (*rit).second.active;
    coral::monitor::Level level  = (*rit).second.level;

    if( active && (type & level) )
    {
      (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description ) );
    }
  }

  void
  UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, int data )
  {
    long long payloaddata = data;
    record( contextKey, source, type, description, payloaddata );
  }

  void
  UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, long long data )
  {
    Repository::iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", name() );

    bool active = (*rit).second.active;
    coral::monitor::Level level  = (*rit).second.level;

    if( active && (type & level) )
    {
      (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
    }
  }

  void
  UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, double data )
  {
    Repository::iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", name() );

    bool active = (*rit).second.active;
    coral::monitor::Level level  = (*rit).second.level;

    if( active && (type & level) )
    {
      (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
    }
  }

  void
  UsermonService::record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, const std::string& data )
  {
    Repository::iterator rit;

    if( ( rit = m_events.find( contextKey ) ) == m_events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringService::record", name() );

    bool active = (*rit).second.active;
    coral::monitor::Level level  = (*rit).second.level;

    if( active && (type & level) )
    {
      (*rit).second.stream.push_back( coral::monitor::createEvent( source, type, description, data ) );
    }
  }

  const coral::IMonitoringReporter& UsermonService::reporter() const
  {
    return *m_reporter;
  }

} // namespace usermon

#endif
