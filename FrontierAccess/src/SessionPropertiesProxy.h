// -*- C++ -*-
// $Id: SessionPropertiesProxy.h,v 1.8 2011/03/22 10:36:50 avalassi Exp $
#ifndef SESSION_PROPERTIES_PROXY_H
#define SESSION_PROPERTIES_PROXY_H

#include "SessionProperties.h"

namespace coral
{
  namespace FrontierAccess
  {
    /**
     * Class SessionProperties
     *
     * An implementation of the ISessionProperties interface a proxy to the SessionProperties
     */

    class SessionPropertiesProxy : public ISessionProperties
    {
    public:
      /// Constructor
      SessionPropertiesProxy( SessionProperties& properties, const std::string& schemaName );

      /// Destructor
      virtual ~SessionPropertiesProxy();

      // Return session log
      coral::FrontierAccess::SessionLog& log();

      /// Returns the domain properties
      const DomainProperties& domainProperties() const;

      /// Returns the type converter
      coral::ITypeConverter& typeConverter();
      const coral::ITypeConverter& typeConverter() const;

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

      /// Returns the connection string
      std::string connectionString() const;

      /// Returns the connection string
      std::string connectionURL() const;

      /// The Frontier physical connection handle
      frontier::Connection& connection() const;

      /// Access to Web cache control
      const coral::IWebCacheControl& cacheControl() const;

      /// Access to the connection lock
      boost::mutex& lock() const;

    private:
      /// The underlying properties object
      SessionProperties& m_properties;

      /// The schema name
      const std::string m_schemaName;
    };
  }
}

// Inline methods
inline coral::FrontierAccess::SessionLog& coral::FrontierAccess::SessionPropertiesProxy::log()
{
  return m_properties.log();
}

inline coral::FrontierAccess::SessionPropertiesProxy::SessionPropertiesProxy( coral::FrontierAccess::SessionProperties& properties, const std::string& schemaName )
  : m_properties( properties ), m_schemaName( schemaName )
{
}

inline coral::FrontierAccess::SessionPropertiesProxy::~SessionPropertiesProxy()
{
}

inline const coral::FrontierAccess::DomainProperties& coral::FrontierAccess::SessionPropertiesProxy::domainProperties() const
{
  return m_properties.domainProperties();
}

inline coral::ITypeConverter& coral::FrontierAccess::SessionPropertiesProxy::typeConverter()
{
  return const_cast< coral::FrontierAccess::SessionProperties& >( m_properties ).typeConverter();
}

inline const coral::ITypeConverter& coral::FrontierAccess::SessionPropertiesProxy::typeConverter() const
{
  return m_properties.typeConverter();
}

inline coral::monitor::IMonitoringService* coral::FrontierAccess::SessionPropertiesProxy::monitoringService() const
{
  return m_properties.monitoringService();
}

inline bool coral::FrontierAccess::SessionPropertiesProxy::isTransactionActive() const
{
  return m_properties.isTransactionActive();
}

inline int coral::FrontierAccess::SessionPropertiesProxy::serverVersion() const
{
  return m_properties.serverVersion();
}

inline std::string coral::FrontierAccess::SessionPropertiesProxy::schemaName() const
{
  return m_schemaName;
}

inline std::string coral::FrontierAccess::SessionPropertiesProxy::connectionString() const
{
  return m_properties.connectionString();
}

inline std::string coral::FrontierAccess::SessionPropertiesProxy::connectionURL() const
{
  return m_properties.connectionURL();
}

inline frontier::Connection& coral::FrontierAccess::SessionPropertiesProxy::connection() const
{
  return m_properties.connection();
}

inline const coral::IWebCacheControl& coral::FrontierAccess::SessionPropertiesProxy::cacheControl() const
{
  return m_properties.cacheControl();
}

inline boost::mutex& coral::FrontierAccess::SessionPropertiesProxy::lock() const
{
  return m_properties.lock();
}

#endif
