#include "CoralCommon/MonitoringEvent.h"

namespace coral
{
  namespace monitor
  {
    namespace Event
    {
      ID Record::id()
      {
        static ID s_id = 0;
        return ++s_id;
      }

      Record::Record( coral::monitor::Event::ID id, coral::TimeStamp ts, coral::monitor::Source s, coral::monitor::Type t, const std::string& d, Payload* data )
        : m_id(id),
          m_time( ts ),
          m_source(s),
          m_type(t),
          m_description(d),
          m_data(data)
      {
      }

      bool Record::hasData() const
      {
        return( m_data.get() != 0 );
      }
    }

    // Payload-less version
    Event::Record createEvent( coral::monitor::Source source, coral::monitor::Type type, const std::string& info )
    {
      Event::Record r( Event::Record::id(), coral::TimeStamp::now(), source, type, info, 0 );
      return r;
    }

    // We need this overload in order to compile for const char* input
    Event::Record createEvent( coral::monitor::Source source, coral::monitor::Type type, const std::string& info, const char* data )
    {
      Event::Record r( Event::Record::id(), coral::TimeStamp::now(), source, type, info, new Event::TextData( std::string(data) ) );
      return r;
    }
  }
}
