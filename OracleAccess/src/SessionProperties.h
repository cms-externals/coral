#ifndef ORACLE_SESSIONPROPERTIES_H
#define ORACLE_SESSIONPROPERTIES_H 1

#include <vector>
#include "CoralBase/boost_thread_headers.h"
#include "ConnectionProperties.h"

struct OCIEnv;
struct OCIError;
struct OCIServer;
struct OCISvcCtx;
struct OCIStmt;

namespace coral
{

  class ISchema;
  class ISession;
  //class ITypeConverter;
  class IView;
  class Session;

  namespace monitor
  {
    class IMonitoringService;
  }

  namespace OracleAccess
  {

    /**
     * Class SessionProperties
     *
     * A proxy to OracleAccess::Session,
     * for internal use by OracleAccess classes.
     */

    class SessionProperties
    {

    public:

      /// Constructor
      SessionProperties( boost::shared_ptr<ConnectionProperties> connectionProperties,
                         const std::string& schemaName,
                         Session& session,
                         bool readOnly );

      /// Destructor
      virtual ~SessionProperties();

      /// Invalidates the session (FIXME: choose another method name?)
      void invalidateSession();

      /// Sets the service context handle
      void setOciSvcCtxHandle( OCISvcCtx* ociSvcCtxHandle );

      /// Sets the monitoring service
      void setMonitoringService( coral::monitor::IMonitoringService* monitoringService );

      /// Returns the connection properties
      //const ConnectionProperties& connectionProperties() const;

      /// Returns the domain properties
      const DomainProperties& domainProperties() const;

      /// Returns the domain service name
      const std::string& domainServiceName() const;

      /// Returns the connection string
      std::string connectionString() const;

      /// Returns the type converter
      //coral::ITypeConverter& typeConverter();
      //const coral::ITypeConverter& typeConverter() const;

      /// Returns the OCI environment handle
      OCIEnv* ociEnvHandle() const;

      /// Returns the OCI error handle
      OCIError* ociErrorHandle() const;

      /// Returns the OCI server handle
      OCIServer* ociServerHandle() const;

      /// Returns the OCI service context handle
      OCISvcCtx* ociSvcCtxHandle() const;

      /// Returns the monitoring service
      coral::monitor::IMonitoringService* monitoringService() const;

      /// Returns the transaction state
      bool isTransactionActive() const;

      /// Returns the transaction mode
      bool isTransactionReadOnly() const;

      /// Returns the server version
      int serverVersion() const;

      /// Returns the schema name
      std::string schemaName() const;

      /// Returns the readOnly flag
      bool isReadOnly() const;

      /// Can this session 'select any table'?
      /// Initially it is assumed that it can.
      bool selectAnyTable() const;

      /// Set the (mutable) 'select any table' flag to false.
      void cannotSelectAnyTable() const;

      // Checks if a table exists in a schema (fix MT bug #80065)
      // Schema name "" indicates the nominal schema
      bool existsTable( const std::string& schemaName,
                        const std::string& tableName ) const;

      // Checks if a view exists in a schema (fix MT bug #80065)
      // Schema name "" indicates the nominal schema
      bool existsView( const std::string& schemaName,
                       const std::string& viewName ) const;

      // Get column names of a table in a schema (fix MT bug #80065)
      // Schema name "" indicates the nominal schema
      const std::vector<std::string>
      describeTable( const std::string& schemaName,
                     const std::string& tableName ) const;

      // Get column names of a view in a schema (fix MT bug #80065)
      // Schema name "" indicates the nominal schema
      const std::vector<std::string>
      describeView( const std::string& schemaName,
                    const std::string& viewName ) const;

      // Get a view handle in a schema (fix MT bug #80065)
      // Schema name "" indicates the nominal schema
      IView& viewHandle( const std::string& schemaName,
                         const std::string& viewName ) const;

      // Returns the current C++ type for an SQL type (fix MT bug #80065)
      // Throws UnSupportedSqlTypeException is thrown if SQL type is invalid
      std::string cppTypeForSqlType( const std::string& sqlType ) const;

      // Returns the current SQL type for a C++ type (fix MT bug #80065)
      // Throws UnSupportedCppTypeException is thrown if C++ type is invalid
      std::string sqlTypeForCppType( const std::string& cppType ) const;

      // Release an OCI statement (fix possible MT version of bug #83601)
      void releaseOCIStmt( OCIStmt* ociStmtHandle ) const;

    private:

      /// The connection properties
      boost::shared_ptr<ConnectionProperties> m_connectionProperties;

      /// The domain service name (this is retrieved from DomainProperties only
      /// in the ctor and stays valid at destruction time - fix bug #71210)
      const std::string m_domainServiceName;

      /// The session pointer (NULL if invalid)
      Session* m_session;

      /// The OCI service context handle
      OCISvcCtx* m_ociSvcCtxHandle;

      /// The monitoring service
      coral::monitor::IMonitoringService* m_monitoringService;

      /// The schema name
      std::string m_schemaName;

      /// The read-only flag
      bool m_isReadOnly;

      /// The mutex lock
      mutable boost::mutex m_mutex;

      /// The mutex lock for the schema (fix bug #80065)
      mutable boost::mutex m_schemaMutex;

      /// The mutex lock for the transaction (fix bug #80174?)
      //mutable boost::mutex m_transactionMutex;

      /// The 'select any table' flag
      mutable bool m_selectAnyTable;

    };

  }

}


// Inline methods

//inline const coral::OracleAccess::ConnectionProperties&
//coral::OracleAccess::SessionProperties::connectionProperties() const
//{
//  return *m_connectionProperties;
//}


inline const coral::OracleAccess::DomainProperties&
coral::OracleAccess::SessionProperties::domainProperties() const
{
  return m_connectionProperties->domainProperties();
}


inline const std::string&
coral::OracleAccess::SessionProperties::domainServiceName() const
{
  return m_domainServiceName;
}


inline std::string
coral::OracleAccess::SessionProperties::connectionString() const
{
  return m_connectionProperties->connectionString();
}


//inline coral::ITypeConverter&
//coral::OracleAccess::SessionProperties::typeConverter()
//{
//  return m_connectionProperties->typeConverter();
//}


//inline const coral::ITypeConverter&
//coral::OracleAccess::SessionProperties::typeConverter() const
//{
//  return m_connectionProperties->typeConverter();
//}


inline OCIEnv*
coral::OracleAccess::SessionProperties::ociEnvHandle() const
{
  return m_connectionProperties->ociEnvHandle();
}


inline OCIError*
coral::OracleAccess::SessionProperties::ociErrorHandle() const
{
  return m_connectionProperties->ociErrorHandle();
}


inline OCIServer*
coral::OracleAccess::SessionProperties::ociServerHandle() const
{
  return m_connectionProperties->ociServerHandle();
}


inline int
coral::OracleAccess::SessionProperties::serverVersion() const
{
  return m_connectionProperties->serverVersion();
}


inline void
coral::OracleAccess::SessionProperties::setOciSvcCtxHandle( OCISvcCtx* ociSvcCtxHandle )
{
  boost::mutex::scoped_lock lock(m_mutex);
  m_ociSvcCtxHandle = ociSvcCtxHandle;
}


inline OCISvcCtx*
coral::OracleAccess::SessionProperties::ociSvcCtxHandle() const
{
  boost::mutex::scoped_lock lock(m_mutex);
  return m_ociSvcCtxHandle;
}


inline void
coral::OracleAccess::SessionProperties::setMonitoringService( coral::monitor::IMonitoringService* monitoringService )
{
  boost::mutex::scoped_lock lock(m_mutex);
  m_monitoringService = monitoringService;
}


inline coral::monitor::IMonitoringService*
coral::OracleAccess::SessionProperties::monitoringService() const
{
  boost::mutex::scoped_lock lock(m_mutex);
  return m_monitoringService;
}


inline bool
coral::OracleAccess::SessionProperties::isReadOnly() const
{
  return m_isReadOnly;
}


inline std::string
coral::OracleAccess::SessionProperties::schemaName() const
{
  return m_schemaName;
}

#endif
