// $Id: SessionProperties.h,v 1.23 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQL_SESSION_PROPERTIES_H
#define MYSQL_SESSION_PROPERTIES_H 1

#include "CoralKernel/IHandle.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/IMonitoringService.h"

#include "ISessionProperties.h"
#include "TypeConverter.h"

namespace coral
{
  class ISession;

  namespace MySQLAccess
  {

    /**
     * Class SessionProperties
     *
     */

    class SessionProperties : virtual public ISessionProperties
    {
    public:
      /// Constructor
      SessionProperties( const DomainProperties& domainProperties,
                         const std::string& connectionString,
                         const std::string& schemaName,
                         MYSQL*& m_mysql,
                         bool& connected,
                         boost::mutex& conlock,
                         coral::ISession& session,
                         coral::AccessMode mode );

      /// Destructor
      virtual ~SessionProperties();

      /// Returns the domain properties
      const DomainProperties& domainProperties() const;

      /// Returns the connection string
      std::string connectionString() const;

      /// Sets the monitoring service
      void setMonitoringService( coral::monitor::IMonitoringService* monitoringService );

      /// Returns the monitoring service
      coral::monitor::IMonitoringService* monitoringService();
      const coral::monitor::IMonitoringService* monitoringService() const;

      /// Returns the type converter
      coral::ITypeConverter& typeConverter();
      const coral::ITypeConverter& typeConverter() const;

      /// Returns the connection handle
      MYSQL*& connectionHandle() const;

      /// Sets the connect flag
      void setConnected( bool isConnected = true );

      /// Returns the connection status
      bool isConnected() const;

      /// Sets the user-session flag
      void setUserSession( bool started = true );

      /// Returns the status is the user session
      bool hasUserSessionStarted() const;

      /// Returns the transaction state
      bool isTransactionActive() const;

      /// Returns the transaction mode
      bool isTransactionReadOnly() const;

      /// Returns the corresponding schema
      coral::ISchema& schema() const;

      /// Returns the schema name
      std::string schemaName() const;

      /// The database server version
      std::string serverVersion() const;

      /// Set the database server version
      void setServerVersion( const std::string& );

      /// The database server version
      ServerRevision serverRevision() const;

      /// Set the database server revision
      void setServerRevision( unsigned long, unsigned long, unsigned long );

      /// Access to the connection mutex lock
      boost::mutex& lock() const;

      /// Returns the readOnly flag
      bool isReadOnly() const;

    private:
      /// The domain properties
      const DomainProperties& m_domainProperties;

      /// The connection string
      std::string m_connectionString;

      /// The session reference
      coral::ISession& m_session;

      /// Access mode: ReadOnly or Update
      coral::AccessMode m_accessMode;

      /// The connectionHandle
      MYSQL*& m_connectionHandle;

      /// The connect flag
      bool& m_isConnected;

      /// The user-session flag
      bool m_userSessionStarted;

      /// The monitoring service
      coral::monitor::IMonitoringService* m_monitoringService;

      /// The type converter
      coral::ITypeConverter& m_typeConverter;

      /// The schema name
      std::string m_schemaName;

      /// Server version
      std::string m_serverVersion;

      /// Server revision
      ServerRevision m_serverRevision;

      /// Connection lock
      boost::mutex& m_lock;

    };
  }
}

// Inline methods
inline const coral::MySQLAccess::DomainProperties& coral::MySQLAccess::SessionProperties::domainProperties() const
{
  return m_domainProperties;
}

inline std::string coral::MySQLAccess::SessionProperties::connectionString() const
{
  return m_connectionString;
}

inline MYSQL*& coral::MySQLAccess::SessionProperties::connectionHandle() const
{
  return const_cast<MYSQL*&>( this->m_connectionHandle );
}

inline void coral::MySQLAccess::SessionProperties::setConnected( bool isConnected )
{
  m_isConnected = isConnected;
}

inline bool coral::MySQLAccess::SessionProperties::isConnected() const
{
  return m_isConnected;
}

inline void coral::MySQLAccess::SessionProperties::setUserSession( bool started )
{
  m_userSessionStarted = started;
}

inline bool coral::MySQLAccess::SessionProperties::hasUserSessionStarted() const
{
  return m_userSessionStarted;
}

inline void coral::MySQLAccess::SessionProperties::setMonitoringService( coral::monitor::IMonitoringService* monitoringService )
{
  m_monitoringService = monitoringService;
}

inline coral::monitor::IMonitoringService* coral::MySQLAccess::SessionProperties::monitoringService()
{
  return m_monitoringService;
}

inline const coral::monitor::IMonitoringService* coral::MySQLAccess::SessionProperties::monitoringService() const
{
  return m_monitoringService;
}

inline boost::mutex& coral::MySQLAccess::SessionProperties::lock() const
{
  return m_lock;
}

#endif // MYSQL_SESSION_PROPERTIES_H
