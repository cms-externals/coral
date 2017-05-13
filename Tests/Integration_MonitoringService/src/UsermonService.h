// -*- C++ -*-
// $Id: UsermonService.h,v 1.6 2010/09/21 09:17:24 avalassi Exp $
#ifndef CORAL_USERMONITOR_SERVICE_H
#define CORAL_USERMONITOR_SERVICE_H 1

#include "SessionMonitor.h"

#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"

#include <string>
#include <map>
#include <vector>

#ifdef CORAL_SEALED

#include "SealKernel/Service.h"

namespace seal
{
  class MessageStream;
}

namespace usermon
{
  class UsermonService : public seal::Service, public coral::monitor::IMonitoringService
  {
    DECLARE_SEAL_COMPONENT;

  public:
    explicit UsermonService( seal::Context * );
    explicit UsermonService( seal::Context *, const std::string& );
    virtual ~UsermonService();

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
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description );

    /**
     * Records an event with a payload
     */
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, int data );

    /**
     * Records an event with a payload
     */
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, double data );

    /**
     * Records an event with a payload
     */
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, const std::string& data );

    /**
     * Return the current reporter
     */
    virtual const coral::IMonitoringReporter& reporter() const;

    // The access to repository to be used by the user reporters
    /**
     * Expose access to the monitoring events repository
     */
    const Repository& repository() const
    {
      return m_events;
    }

    Repository&       repository()
    {
      return m_events;
    }

    /**
     * Returns the set of the monitored data sources
     */
    std::set< std::string > monitoredDataSources() const
    {
      return m_monitoredDS;
    }

  private:

    /// The all events repository classified by the sessions' keys
    Repository m_events;
    std::set<std::string> m_monitoredDS;
  };
} // namespace usermon

#else

namespace coral
{
  class MessageStream;
}

namespace usermon
{
  class UsermonService : public coral::monitor::IMonitoringService
  {
  public:
    explicit UsermonService();
    virtual ~UsermonService();

  public:

    void setReporter( coral::IMonitoringReporter* reporter );

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
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description );

    /**
     * Records an event with a payload
     */
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, int data );

    /**
     * Records an event with a payload
     */
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, long long data );

    /**
     * Records an event with a payload
     */
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, double data );

    /**
     * Records an event with a payload
     */
    virtual void record( const std::string& contextKey, coral::monitor::Source source, coral::monitor::Type type, const std::string& description, const std::string& data );

    /**
     * Return the current reporter
     */
    virtual const coral::IMonitoringReporter& reporter() const;

    // The access to repository to be used by the user reporters
    /**
     * Expose access to the monitoring events repository
     */
    const Repository& repository() const
    {
      return m_events;
    }

    Repository&       repository()
    {
      return m_events;
    }

    /**
     * Returns the set of the monitored data sources
     */
    std::set< std::string > monitoredDataSources() const
    {
      return m_monitoredDS;
    }

    std::string name() const
    {
      return "usermon::UsermonService";
    }

  private:

    /// The all events repository classified by the sessions' keys
    Repository m_events;
    std::set<std::string>       m_monitoredDS;
    coral::IMonitoringReporter* m_reporter;
  };
} // namespace usermon

#endif

#endif // CORAL_MONITOR_SERVICE_H
