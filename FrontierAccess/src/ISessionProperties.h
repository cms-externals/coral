// -*- C++ -*-
// $Id: ISessionProperties.h,v 1.14 2011/03/22 10:36:50 avalassi Exp $
#ifndef ISESSION_PROPERTIES_H
#define ISESSION_PROPERTIES_H 1

#include "CoralBase/TimeStamp.h"
#include "CoralBase/boost_thread_headers.h"

#include <vector>
#include <string>

namespace frontier
{
  class Connection;
}

namespace coral
{
  class TimeStamp;

  class ITypeConverter;
  class ITransaction;
  class ISession;
  class ISchema;
  class IWebCacheControl;

  namespace monitor
  {
    class IMonitoringService;
  }

  namespace FrontierAccess
  {
    class DomainProperties;
    class TypeConverter;

    struct LogEntry
    {
      LogEntry()
        : ts( coral::TimeStamp::now() ), sql( "" ), err( 0 ), errmsg( "" )
      {
      }

      explicit LogEntry( const std::string& query, int error_code=0, const std::string& error_message="", coral::TimeStamp t=coral::TimeStamp::now() )
        : ts( t ), sql( query ), err( error_code ), errmsg( error_message )
      {
      }

      coral::TimeStamp ts;
      std::string sql;
      int err;
      std::string errmsg;
    };

    typedef std::vector<LogEntry> SessionLog;

    /**
     * Class SessionProperties
     *
     * Interface for classes holding the domain properties, as well as the global
     * data and properties for the current session.
     */
    class ISessionProperties
    {
    public:
      virtual ~ISessionProperties() {};

      /// Returns session query log along with status and/or error message logged
      virtual coral::FrontierAccess::SessionLog& log() = 0;

      /// Returns the domain properties
      virtual const DomainProperties& domainProperties() const = 0;

      /// Returns the type converter
      virtual coral::ITypeConverter& typeConverter() = 0;
      virtual const coral::ITypeConverter& typeConverter() const = 0;

      /// Returns the monitoring service
      virtual coral::monitor::IMonitoringService* monitoringService() const = 0;

      /// Returns the transaction state
      virtual bool isTransactionActive() const = 0;

      /// Returns the server version
      virtual int serverVersion() const = 0;

      /// Returns the corresponding schema
      virtual coral::ISchema& schema() const = 0;

      /// Returns the schema name
      virtual std::string schemaName() const = 0;

      /// Returns the connection string
      virtual std::string connectionString() const = 0;

      /// Returns the Frontier connection URL
      virtual std::string connectionURL() const = 0;

      /// Returns the Frontier physical connection handle
      virtual frontier::Connection& connection() const = 0;

      /// Access to Web cache control
      virtual const coral::IWebCacheControl& cacheControl() const = 0;

      /// Access to the connection lock
      virtual boost::mutex& lock() const = 0;
    };
  }
}

#endif
