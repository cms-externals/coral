// -*- C++ -*-
// $Id: Service.h,v 1.8 2011/03/22 11:24:54 avalassi Exp $
#ifndef CORAL_MONITOR_SERVICE_H
#define CORAL_MONITOR_SERVICE_H 1

#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"

#include "CoralCommon/MonitoringReportTypes.h"
#include "CoralCommon/MonitoringEvent.h"

#include "CoralKernel/Service.h"

#include <string>
#include <map>
#include <vector>

namespace coral
{
  class MessageStream;

  namespace monitor
  {
    /// The raw stream of recorder monitoring events
    typedef std::vector<coral::monitor::Event::Record> EventStream;

    /// The session related book-keeping of monitored events
    struct SessionMonitor
    {
      SessionMonitor();
      SessionMonitor( bool, coral::monitor::Level );

      bool active;
      coral::monitor::Level level;
      EventStream stream;
    };

    /// The repository of monitored events for all sessions where event streams
    /// are identified by session key
    typedef std::map< std::string, SessionMonitor > Repository;

    class Service : public coral::Service, virtual public coral::monitor::IMonitoringService, virtual public coral::IMonitoringReporter
    {
    public:
      explicit Service( const std::string& componentName );
      virtual ~Service();

    public:
      /** The implementation coral::IMonitoringService interface */
      /**
       * Sets the level
       * @param contextKey The session ID for which to make the report
       * @param level      The monitoring level ( Default, Debug, Trace )
       */
      virtual void setLevel( const std::string& contextKey, coral::monitor::Level level );

      /**
       * Return current monitoring level
       */
      virtual coral::monitor::Level level( const std::string& contextKey ) const;

      /**
       * Return monitoring activity status
       */
      virtual bool active( const std::string& contextKey ) const;

      virtual void enable( const std::string& contextKey );
      virtual void disable( const std::string& contextKey );

      /**
       * Records an event without a payload ( time event for example )
       */
      virtual void record( const std::string& contextKey,
                           Source source,
                           coral::monitor::Type type,
                           const std::string& description );

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           Source source,
                           coral::monitor::Type type,
                           const std::string& description,
                           int data );

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           Source source,
                           coral::monitor::Type type,
                           const std::string& description,
                           long long int data );

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           Source source,
                           coral::monitor::Type type,
                           const std::string& description,
                           double data );

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           Source source,
                           coral::monitor::Type type,
                           const std::string& description,
                           const std::string& data );

      /**
       * Return the current reporter
       */
      virtual const coral::IMonitoringReporter& reporter() const;

      //
      // The coral::IMonitoringReporter interface implementation
      //

      /**
       * Return the set of currently monitored data sources
       */
      virtual std::set< std::string > monitoredDataSources() const;

      /**
       * Reports all the events
       * @param contextKey The session ID for which to make the report
       */
      virtual void report( unsigned int level ) const;

      /**
       * Reports the events to the default reporter
       * @param contextKey The session ID for which to make the report
       */
      virtual void report( const std::string& contextKey, unsigned int level ) const;

      /**
       * Reports the events to the specified output stream
       * @param contextKey The session ID for which to make the report
       */
      virtual void reportToOutputStream( const std::string& contextKey, std::ostream& os, unsigned int level ) const;

      /**
       * Configure the desired type of report medium, which can be:
       *
       * @param rp Report medium type: CSV, XML, SQLite file
       * @param fn File name to store report data into
       */
      void setOutputType( ReportType rp, const std::string& fn );

      void reportOnEvent( EventStream::const_iterator& it, std::ostream&        os ) const;
      void reportOnEvent( EventStream::const_iterator& it, coral::MessageStream& os ) const;

      void reportForSession( Repository::const_iterator& it, std::ostream&        os ) const;
      void reportForSession( Repository::const_iterator& it, coral::MessageStream& os ) const;

    private:

      /// Ignore case for oracle context keys (workaround for bug #58841)
      const std::string distinctContextKey( const std::string& contextKey ) const;

      /// The all events repository classified by the sessions' keys
      Repository m_events;
      std::set<std::string> m_monitoredDS;
    };
  } // namespace monitor
} // namespace coral

#endif // CORAL_MONITOR_SERVICE_H
