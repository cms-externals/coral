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

      Record::Record( const Record& right )
      {
        m_id          = right.m_id;
        m_time        = right.m_time;
        m_source      = right.m_source;
        m_type        = right.m_type;
        m_description = right.m_description;
        m_data        = const_cast<std::auto_ptr<Payload>&>(right.m_data); // Needed to avoid double delete
      }

      Record& Record::operator=( const Record& right )
      {
        if( this != &right )
        {
          m_id          = right.m_id;
          m_time        = right.m_time;
          m_source      = right.m_source;
          m_type        = right.m_type;
          m_description = right.m_description;
          m_data        = const_cast<std::auto_ptr<Payload>&>(right.m_data); // Needed to avoid double delete
        }

        return *this;
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
