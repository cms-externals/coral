#include "UserReporter.h"
#include "UsermonService.h"

#include "RelationalAccess/MonitoringException.h"

#ifdef CORAL_SEALED

#include "SealKernel/MessageStream.h"

DEFINE_SEAL_COMPONENT ( usermon::UserReporter, "CORAL/Services/UserMonitoringService/UserReporter" )

  namespace usermon
  {
    UserReporter::UserReporter( seal::Context* c )
      : seal::Component( c, classContextKey() )
    {
    }

    UserReporter::UserReporter( seal::Context* c , const std::string& key )
      : seal::Component( c, key )
    {
    }

    UserReporter::~UserReporter()
    {
    }

    // The coral::IMonitoringReporter interface implementation

    std::set< std::string > UserReporter::monitoredDataSources() const
    {
      seal::Handle<usermon::UsermonService> umsvc = this->context()->component<usermon::UsermonService>();
      return umsvc->monitoredDataSources();
    }

    void UserReporter::report( unsigned int /*level*/ ) const
    {
      seal::Handle<usermon::UsermonService> umsvc = this->context()->component<usermon::UsermonService>();
      const Repository& events = umsvc->repository();

      Repository::const_iterator rit;

      seal::MessageStream log( umsvc->localContext() , "MonitoringUserReporter", seal::Msg::Info );

      // Dummy reporting so far
      for( rit = events.begin(); rit != events.end(); ++rit )
        reportForSession( rit, log );
    }

    void UserReporter::report( const std::string& contextKey, unsigned int /*level*/ ) const
    {
      seal::Handle<usermon::UsermonService> umsvc = this->context()->component<usermon::UsermonService>();
      const Repository& events = umsvc->repository();

      Repository::const_iterator rit;

      if( ( rit = events.find( contextKey ) ) == events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringUserReporter::record", umsvc->name() );

      // Dummy reporting so far
      seal::MessageStream log(  umsvc->localContext(), "MonitoringUserReporter", seal::Msg::Info );

      reportForSession( rit, log );
    }

    void UserReporter::reportToOutputStream( const std::string& contextKey, std::ostream& os, unsigned int /*level*/ ) const
    {
      seal::Handle<usermon::UsermonService> umsvc = this->context()->component<usermon::UsermonService>();
      const Repository& events = umsvc->repository();
      Repository::const_iterator rit;

      if( ( rit = events.find( contextKey ) ) == events.end() )
        throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringUserReporter::record", umsvc->name() );

      // Dummy reporting so far
      seal::MessageStream log(  umsvc->localContext() , "MonitoringUserReporter" );

      reportForSession( rit, os );
    }

    // FIXME !!!!!!!!!!!!  Make the thing work via service handle

    void UserReporter::setOutputType( coral::monitor::ReportType /*rp*/, const std::string& /*fn*/ )
    {
      // FIXME!!! Implement reporting style switch, e.g. list, top 10 long run sql or top 10 most frequent sql
    }

    void UserReporter::reportOnEvent( EventStream::const_iterator& it, std::ostream& os ) const
    {
      os << (*it).m_id << " " << (*it).m_time.format (true,  "%H:%M:%S %d.%m. %Y")
         << " " << (*it).m_source << " " << (*it).m_type << " " << (*it).m_description
         << " " << ((*it).hasData() ? (*it).m_data->valueAsString() : std::string("no payload")) << std::endl;
    }

    void UserReporter::reportOnEvent( EventStream::const_iterator& it, seal::MessageStream& os ) const
    {
      os << (*it).m_id << " " << (*it).m_time.format (true,  "%H:%M:%S %d.%m. %Y")
         << " " << (*it).m_source << " " << (*it).m_type << " " << (*it).m_description
         << " " << ((*it).hasData() ? (*it).m_data->valueAsString() : std::string("no payload")) << seal::flush;
    }

    void UserReporter::reportForSession( Repository::const_iterator& it, std::ostream& os ) const
    {
      os << "Session: " << (*it).first        << std::flush;
      os << "Level:   " << (*it).second.level << std::flush;

      const EventStream& evsref = (*it).second.stream;

      for( EventStream::const_iterator evit = evsref.begin(); evit != evsref.end(); ++evit )
      {
        reportOnEvent( evit, os );
      }
    }

    void UserReporter::reportForSession( Repository::const_iterator& it, seal::MessageStream& os ) const
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

      os << lvl << " has recorded " << evsref.size() << " events" << seal::flush;


      for( EventStream::const_iterator evit = evsref.begin(); evit != evsref.end(); ++evit )
      {
        reportOnEvent( evit, os );
      }
    }

  } // namespace usermon

#else

#include "CoralBase/boost_datetime_headers.h"
#include "CoralBase/MessageStream.h"

namespace usermon
{
  UserReporter::UserReporter( usermon::UsermonService& msvc )
    : m_svc( msvc )
  {
  }

  UserReporter::~UserReporter()
  {
  }

  // The coral::IMonitoringReporter interface implementation

  std::set< std::string > UserReporter::monitoredDataSources() const
  {
    return m_svc.monitoredDataSources();
  }

  void UserReporter::report( unsigned int /*level*/ ) const
  {
    const Repository& events = m_svc.repository();

    Repository::const_iterator rit;

    coral::MessageStream log( "coral::Tests::MonitoringUserReporter" );

    // Dummy reporting so far
    for( rit = events.begin(); rit != events.end(); ++rit )
      reportForSession( rit, log );
  }

  void UserReporter::report( const std::string& contextKey, unsigned int /*level*/ ) const
  {
    const Repository& events = m_svc.repository();

    Repository::const_iterator rit;

    if( ( rit = events.find( contextKey ) ) == events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringUserReporter::record", m_svc.name() );

    // Dummy reporting so far
    coral::MessageStream log(  "coral::Tests::MonitoringUserReporter" );

    reportForSession( rit, log );
  }

  void UserReporter::reportToOutputStream( const std::string& contextKey, std::ostream& os, unsigned int /*level*/ ) const
  {
    const Repository& events = m_svc.repository();

    Repository::const_iterator rit;

    if( ( rit = events.find( contextKey ) ) == events.end() )
      throw coral::MonitoringException( "Monitoring for session " + contextKey + " not initialized...", "MonitoringUserReporter::record", m_svc.name() );

    // Dummy reporting so far
    coral::MessageStream log(  "coral::Tests::MonitoringUserReporter" );

    reportForSession( rit, os );
  }

  // FIXME !!!!!!!!!!!!  Make the thing work via service handle

  void UserReporter::setOutputType( coral::monitor::ReportType /*rp*/, const std::string& /*fn*/ )
  {
    // FIXME!!! Implement reporting style switch, e.g. list, top 10 long run sql or top 10 most frequent sql
  }

  void UserReporter::reportOnEvent( EventStream::const_iterator& it, std::ostream& os ) const
  {
    os << (*it).m_id << " " << (*it).m_time.toString()
       << " " << (*it).m_source << " " << (*it).m_type << " " << (*it).m_description
       << " " << ((*it).hasData() ? (*it).m_data->valueAsString() : std::string("no payload")) << std::endl;
  }

  void UserReporter::reportOnEvent( EventStream::const_iterator& it, coral::MessageStream& os ) const
  {
    os << (*it).m_id << " " << (*it).m_time.toString()
       << " " << (*it).m_source << " " << (*it).m_type << " " << (*it).m_description
       << " " << ((*it).hasData() ? (*it).m_data->valueAsString() : std::string("no payload")) << coral::MessageStream::endmsg;
  }

  void UserReporter::reportForSession( Repository::const_iterator& it, std::ostream& os ) const
  {
    os << "Session: " << (*it).first        << std::endl;
    os << "Level:   " << (*it).second.level << std::endl;

    const EventStream& evsref = (*it).second.stream;

    for( EventStream::const_iterator evit = evsref.begin(); evit != evsref.end(); ++evit )
    {
      reportOnEvent( evit, os );
    }
  }

  void UserReporter::reportForSession( Repository::const_iterator& it, coral::MessageStream& os ) const
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

} // namespace usermon

#endif
