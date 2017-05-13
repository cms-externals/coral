// -*- C++ -*-
// $Id: UserReporter.h,v 1.5 2010/09/21 09:17:24 avalassi Exp $
#ifndef CORAL_USERREPORTER_H
#define CORAL_USERREPORTER_H 1

#include "SessionMonitor.h"

#include "RelationalAccess/IMonitoringReporter.h"

#include <string>
#include <map>
#include <vector>

#ifdef CORAL_SEALED

#include "SealKernel/Component.h"

namespace seal
{
  class MessageStream;
}

namespace usermon
{
  class UserReporter : public seal::Component, virtual public coral::IMonitoringReporter
  {
    DECLARE_SEAL_COMPONENT;

  public:
    explicit UserReporter( seal::Context* );
    explicit UserReporter( seal::Context*, const std::string& );
    virtual ~UserReporter();

  public:

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
    void setOutputType( coral::monitor::ReportType rp, const std::string& fn );

    void reportOnEvent( EventStream::const_iterator& it, std::ostream&        os ) const;
    void reportOnEvent( EventStream::const_iterator& it, seal::MessageStream& os ) const;

    void reportForSession( Repository::const_iterator& it, std::ostream&        os ) const;
    void reportForSession( Repository::const_iterator& it, seal::MessageStream& os ) const;
  };
} // namespace usermon

#else

namespace coral
{
  class MessageStream;
}

namespace usermon
{
  class UsermonService;

  class UserReporter : virtual public coral::IMonitoringReporter
  {
  public:
    explicit UserReporter( usermon::UsermonService& );
    virtual ~UserReporter();

  public:

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
    void setOutputType( coral::monitor::ReportType rp, const std::string& fn );

    void reportOnEvent( EventStream::const_iterator& it, std::ostream&        os ) const;
    void reportOnEvent( EventStream::const_iterator& it, coral::MessageStream& os ) const;

    void reportForSession( Repository::const_iterator& it, std::ostream&        os ) const;
    void reportForSession( Repository::const_iterator& it, coral::MessageStream& os ) const;

  private:
    usermon::UsermonService& m_svc;
  };
} // namespace usermon

#endif

#endif // CORAL_MONITOR_SERVICE_H
