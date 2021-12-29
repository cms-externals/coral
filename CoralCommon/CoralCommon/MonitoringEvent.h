// -*- C++ -*-
// $Id: MonitoringEvent.h,v 1.8 2011/03/22 11:22:33 avalassi Exp $
#ifndef CORAL_MONITOR_EVENT_H
#define CORAL_MONITOR_EVENT_H 1

#include "CoralBase/TimeStamp.h"

#include "RelationalAccess/IMonitoringService.h"

#include <string>
#include <sstream>
#include <memory>

namespace coral
{
  namespace monitor
  {
    namespace Event
    {
      typedef size_t ID;

      class Payload;

      struct Record
      {
        Record( coral::monitor::Event::ID id, coral::TimeStamp ts, coral::monitor::Source s, coral::monitor::Type t, const std::string& d, Payload* data );
        bool hasData() const;

        unsigned long m_id; // Automatic unique ID
        coral::TimeStamp m_time; // Automatic timestamp
        Source m_source; // Origin of the event
        Type m_type; // Type of the event
        std::string m_description; // Event description
        std::unique_ptr<Payload> m_data; // Actual data related to an event, 0 if data irrelevant

        static ID id();
      };

      /** Class Payload
       *  The event data holder. One needs only three types of payload:
       *  time    - timer events (begin/end)
       *  text    - information like exception message or specific description of a particular event
       *  integer - numerical value describing a quantity like buffer size or number of rows fetched etc.
       */
      class Payload
      {
      public:
        virtual ~Payload() {};
        virtual std::string valueAsString() const = 0;
      };

      template <typename T> class RealPayload : public Payload
      {
      public:
        typedef T data_type;

      public:
        RealPayload( const T& data) : m_data( data ) { }

        virtual ~RealPayload() { }

        virtual std::string valueAsString() const {
          std::stringstream os;
          if( ! m_data )
            os << "";
          else
            os << m_data;
          return os.str();
        }

        operator T() { return m_data; }

      protected:
        RealPayload();

      private:
        T m_data;
      };

      template <> inline
      RealPayload<std::string>::RealPayload( const std::string& data ) {
        m_data = data;
      }

      template <> inline std::string
      RealPayload<std::string>::valueAsString() const { return m_data; }

      typedef RealPayload<std::string>   TextData;
      typedef RealPayload<unsigned long> NumericData;
      typedef RealPayload<unsigned long> TimeData;

    }

    template <typename T>
    Event::Record createEvent( coral::monitor::Source source, coral::monitor::Type type, const std::string& info, const T& data )
    {
      Event::Record r( Event::Record::id(), coral::TimeStamp::now(), source, type, info, new Event::RealPayload<T>( data ) );
      return r;
    }

    /// Payload-less version
    Event::Record createEvent( coral::monitor::Source source, coral::monitor::Type type, const std::string& info );

    /// We need this overload in order to compile for const char* input
    Event::Record createEvent( coral::monitor::Source source, coral::monitor::Type type, const std::string& info, const char* data );

  } // namespace monitor
} // namespace coral

#endif // CORAL_MONITOR_EVENT_H
