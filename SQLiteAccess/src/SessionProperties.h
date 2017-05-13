#ifndef SQLITEACCESS_SESSIONPROPERTIES_H
#define SQLITEACCESS_SESSIONPROPERTIES_H 1

#include <string>
#include <boost/shared_ptr.hpp>
#include "CoralBase/boost_thread_headers.h"

#include "ConnectionProperties.h"

//class sqlite3;
struct sqlite3;

namespace coral
{
  class DomainProperties;
  class ISchema;
  class ISession;
  class ITypeConverter;

  namespace monitor
  {
    class IMonitoringService;
  }

  namespace SQLiteAccess
  {
    class DomainProperties;
    class Session;
    class TypeConverter;

    /**
     * Class SessionProperties
     *
     * A simple class holding the connection properties, as well as the global
     * data and properties for the current session.
     */

    class SessionProperties
    {
    public:

      /// Constructor
      SessionProperties( boost::shared_ptr<ConnectionProperties> connectionProperties,
                         Session& session,
                         bool readOnly,
                         boost::mutex* mutex );

      /// Destructor
      virtual ~SessionProperties();

      /// Sets the sqlite db handle
      void setDbHandle( sqlite3* dbHandle );

      /// Invalidates the session (FIXME: choose another method name?)
      void invalidateSession();

      /// Sets the monitoring service
      void setMonitoringService( coral::monitor::IMonitoringService* monitoringService );

      /// Returns the domain properties
      const DomainProperties& domainProperties() const;

      /// Returns the connection properties
      boost::shared_ptr<ConnectionProperties> connectionProperties();

      /// Returns the connection string
      std::string connectionString() const;

      /// Returns the type converter
      coral::ITypeConverter& typeConverter();

      /// Returns the type converter
      const coral::ITypeConverter& typeConverter() const;

      /// Returns the monitoring service
      coral::monitor::IMonitoringService* monitoringService() const;

      /// Returns the transaction state
      bool isTransactionActive() const;

      /// Returns the transaction mode
      bool isTransactionReadOnly() const;

      /// Returns the server version
      //int serverVersion() const;

      /// Returns the corresponding schema
      coral::ISchema& schema() const;

      /// Returns the schema name
      std::string schemaName() const;

      /// Returns the readOnly flag
      bool isReadOnly() const;

      /// Returns the db handle
      sqlite3* dbHandle() const;

      /// Returns the mutex
      boost::mutex* mutex() const { return m_mutex; };

    private:

      /// The connection properties
      boost::shared_ptr<ConnectionProperties> m_connectionProperties;

      /// The domain properties
      ///DomainProperties& m_domainProperties;

      /// The connection string
      std::string m_connectionString;

      /// The type converter
      //coral::ITypeConverter& m_typeConverter;

      /// The session reference
      Session* m_session;

      /// The sqlite db handle
      mutable sqlite3* m_dbHandle;

      /// The monitoring service
      coral::monitor::IMonitoringService* m_monitoringService;

      /// The schema name
      //std::string m_schemaName;

      /// The read-only flag
      bool m_isReadOnly;

      //reference of the original mutex
      //settled in connection properties
      boost::mutex* m_mutex;

    };
  }
}

inline void
coral::SQLiteAccess::SessionProperties::invalidateSession()
{
  m_session = NULL;
}

#endif
