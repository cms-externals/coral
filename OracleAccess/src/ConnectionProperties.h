#ifndef ORACLE_CONNECTION_PROPERTIES_H
#define ORACLE_CONNECTION_PROPERTIES_H

#include <string>
#include "CoralBase/boost_thread_headers.h"

struct OCIEnv;
struct OCIError;
struct OCIServer;

namespace coral
{

  class ITypeConverter;

  namespace OracleAccess
  {

    class Connection;
    class DomainProperties;
    class TypeConverter;

    /**
     * Class ConnectionProperties
     *
     * A class holding the running parameters of a database connection
     */

    class ConnectionProperties
    {
    public:
      /// Constructor
      ConnectionProperties( const DomainProperties& domainProperties,
                            const std::string& connectionString,
                            Connection& connection );

      /// Destructor
      virtual ~ConnectionProperties();

      /// Invalidates the connection (FIXME: choose another method name?)
      void invalidateConnection();

      /// Sets the OCI handles and resets the type converter
      void setHandles( OCIEnv* ociEnvHandle,
                       OCIError* ociErrorHandle,
                       OCIServer* ociServerHandle,
                       int majorServerVersion );

      /// Returns the domain properties
      const DomainProperties& domainProperties() const;

      /// Returns the domain service name
      const std::string& domainServiceName() const;

      /// Returns the connection string
      std::string connectionString() const;

      /// Returns the connection object
      Connection& connection() const;

      /// Returns the type converter
      coral::ITypeConverter& typeConverter();
      const coral::ITypeConverter& typeConverter() const;

      /// Returns the OCI environment handle
      OCIEnv* ociEnvHandle() const;

      /// Returns the OCI error handle
      OCIError* ociErrorHandle() const;

      /// Returns the OCI server handle
      OCIServer* ociServerHandle() const;

      /// Returns the server version
      int serverVersion() const;

      /// Returns the mutex for this connection
      //boost::mutex& connectionMutex() const;

      /// NetworkGlitch: Check the validity of the connection
      bool wasConnectionLost();

      /// NetworkGlitch: Reconnect
      bool restartConnection();

    private:

      /// The domain properties
      const DomainProperties& m_domainProperties;

      /// The domain service name (this is retrieved from DomainProperties only
      /// in the ctor and stays valid at destruction time - fix bug #71210)
      const std::string m_domainServiceName;

      /// The connection string
      std::string m_connectionString;

      /// The type converter
      TypeConverter* m_typeConverter;

      /// The OCI environment handle
      OCIEnv* m_ociEnvHandle;

      /// The OCI error handle
      OCIError* m_ociErrorHandle;

      /// The OCI server handle
      OCIServer* m_ociServerHandle;

      /// The server major version
      int m_serverVersion;

      /// The connection pointer (NULL if invalid)
      Connection* m_connection;

      /// The external mutex lock (over OCI calls on shared connections).
      //mutable boost::mutex m_connectionMutex;

      /// The internal mutex lock (over the connection properties).
      mutable boost::mutex m_mutex;

    };

  }

}


// Inline methods
inline void
coral::OracleAccess::ConnectionProperties::invalidateConnection()
{
  m_connection = NULL;
}


inline const coral::OracleAccess::DomainProperties&
coral::OracleAccess::ConnectionProperties::domainProperties() const
{
  return m_domainProperties;
}


inline const std::string&
coral::OracleAccess::ConnectionProperties::domainServiceName() const
{
  return m_domainServiceName;
}


inline std::string
coral::OracleAccess::ConnectionProperties::connectionString() const
{
  return m_connectionString;
}


inline OCIEnv*
coral::OracleAccess::ConnectionProperties::ociEnvHandle() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return m_ociEnvHandle;
}


inline OCIError*
coral::OracleAccess::ConnectionProperties::ociErrorHandle() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return m_ociErrorHandle;
}


inline OCIServer*
coral::OracleAccess::ConnectionProperties::ociServerHandle() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return m_ociServerHandle;
}


inline int
coral::OracleAccess::ConnectionProperties::serverVersion() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return m_serverVersion;
}


/*
inline boost::mutex&
coral::OracleAccess::ConnectionProperties::connectionMutex() const
{
  return m_connectionMutex;
}
*/

#endif
