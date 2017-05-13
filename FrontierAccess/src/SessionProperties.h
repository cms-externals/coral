// -*- C++ -*-
// $Id: SessionProperties.h,v 1.15 2011/03/22 10:36:50 avalassi Exp $
#ifndef FRONTIER_SESSION_PROPERTIES_H
#define FRONTIER_SESSION_PROPERTIES_H 1

#include "ISessionProperties.h"
#include "DomainProperties.h"

#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IMonitoringService.h"

#include "CoralBase/boost_thread_headers.h"

namespace frontier
{
  class Connection;
}

namespace coral
{
  namespace FrontierAccess
  {
    /**
     * Class SessionProperties
     *
     * An implementation of the ISessionProperties interface.
     */
    class SessionProperties : public ISessionProperties
    {
    public:
      /// Constructor
      SessionProperties( const DomainProperties& domainProperties,
                         const std::string& connectionString,
                         frontier::Connection& fconnection,
                         boost::mutex& flock,
                         const std::string& schemaName,
                         coral::ITypeConverter& converter,
                         coral::ISession& session );
      /// Destructor
      virtual ~SessionProperties();
      virtual coral::FrontierAccess::SessionLog& log();
      /// Sets the server version and resets the type converter
      void setServerVersion( int majorServerVersion );
      /// Sets the monitoring service
      void setMonitoringService( coral::monitor::IMonitoringService* monitoringService );
      /// Returns the domain properties
      const DomainProperties& domainProperties() const;
      /// Returns the type converter
      virtual coral::ITypeConverter& typeConverter();
      virtual const coral::ITypeConverter& typeConverter() const;
      /// Returns the connection string
      std::string connectionString() const;
      void setConnectionString( const std::string& );
      std::string connectionURL() const;
      void setConnectionURL( const std::string& );
      /// The Frontier physical connection handle
      frontier::Connection& connection() const;
      /// Returns the session object
      coral::ISession& session();
      /// Returns the monitoring service
      coral::monitor::IMonitoringService* monitoringService() const;
      /// Returns the transaction state
      bool isTransactionActive() const;
      /// Returns the server version
      int serverVersion() const;
      /// Returns the corresponding schema
      coral::ISchema& schema() const;
      /// Returns the schema name
      std::string schemaName() const;
      void setSchemaName( const std::string& );
      /// Access to Web cache control
      const coral::IWebCacheControl& cacheControl() const;
      /// Access to the connection lock
      boost::mutex& lock() const;
      /// Get the handle to the connection service
      // AV 16.04.2008 Fix for bug #35528
      //coral::IHandle<coral::IConnectionService> coral::FrontierAccess::SessionProperties::connectionService() const;
      coral::IHandle<coral::IConnectionService> connectionService() const;

    private:
      /// The domain properties
      const DomainProperties& m_domainProperties;
      /// The connection string
      std::string m_connectionString;
      /// The connection URL
      std::string m_connectionURL;
      /// The physical connection
      frontier::Connection& m_connection;
      /// The session reference
      coral::ISession& m_session;
      /// Type converter
      coral::ITypeConverter& m_typeConverter;
      /// Monitoring service
      coral::monitor::IMonitoringService* m_monitoringService;
      /// The server major version
      int m_serverVersion;
      /// The schema name
      std::string m_schemaName;
      /// Session query log
      coral::FrontierAccess::SessionLog m_log;
      /// Access to the connection lock
      boost::mutex& m_lock;
    };
  }
}

// Inline methods
inline coral::FrontierAccess::SessionLog& coral::FrontierAccess::SessionProperties::log()
{
  return m_log;
}

inline const coral::FrontierAccess::DomainProperties& coral::FrontierAccess::SessionProperties::domainProperties() const
{
  return m_domainProperties;
}

inline std::string coral::FrontierAccess::SessionProperties::connectionString() const
{
  return m_connectionString;
}

inline void coral::FrontierAccess::SessionProperties::setConnectionString( const std::string& newCs )
{
  m_connectionString = newCs;
}

inline std::string coral::FrontierAccess::SessionProperties::connectionURL() const
{
  return m_connectionURL;
}

inline void coral::FrontierAccess::SessionProperties::setConnectionURL( const std::string& newCs )
{
  m_connectionURL = newCs;
}

inline frontier::Connection& coral::FrontierAccess::SessionProperties::connection() const
{
  return m_connection;
}

inline coral::monitor::IMonitoringService* coral::FrontierAccess::SessionProperties::monitoringService() const
{
  return m_monitoringService;
}

inline int coral::FrontierAccess::SessionProperties::serverVersion() const
{
  return m_serverVersion;
}

inline coral::ISession& coral::FrontierAccess::SessionProperties::session()
{
  return m_session;
}

inline const coral::IWebCacheControl& coral::FrontierAccess::SessionProperties::cacheControl() const
{
  return m_domainProperties.cacheControl();
}

inline boost::mutex& coral::FrontierAccess::SessionProperties::lock() const
{
  return m_lock;
}

#endif
