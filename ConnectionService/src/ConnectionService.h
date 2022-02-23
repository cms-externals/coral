#ifndef CONNECTIONSERVICE_CONNECTIONSERVICE_H
#define CONNECTIONSERVICE_CONNECTIONSERVICE_H

#include "RelationalAccess/ICertificateConnectionService.h"
#include "RelationalAccess/AccessMode.h"

#include "CoralKernel/Service.h"

#include <set>
#include "CoralBase/boost_thread_headers.h"

namespace coral
{

  class ISessionProxy;
  class ICertificateData;

  namespace ConnectionService
  {

    class ConnectionServiceConfiguration;
    class ConnectionPool;
    class SessionProxy;
    class WebCacheControl;

    /// coral built-in implementation of the interface IConnectionService
    class ConnectionService : public coral::Service,
                              virtual public coral::ICertificateConnectionService
    {

    public:

      /// Standard Constructor
      explicit ConnectionService( const std::string& componentName );

      /// Standard Destructor
      virtual ~ConnectionService();

      /** get proxy to ISession */
      coral::ISessionProxy* connect( const std::string& connectionString,
                                     AccessMode accessMode )
      {
        return connect( connectionString, accessMode, 0 );
      };

      /** get proxy to ISession */
      coral::ISessionProxy* connect( const std::string& connectionString,
                                     const std::string& asRole,
                                     AccessMode accessMode = Update )
      {
        return connect( connectionString, asRole, accessMode, 0 );
      };

      /** get proxy to ISession */
      ISessionProxy* connect( const std::string& connectionString,
                              AccessMode accessMode,
                              const ICertificateData* cert );

      /** get proxy to ISession */
      ISessionProxy* connect( const std::string& connectionString,
                              const std::string& asRole,
                              AccessMode accessMode,
                              const ICertificateData* cert );

      /**
       * Cleans up the connection pool from the unused connection, according to
       * the policy defined in the configuration.
       */
      void purgeConnectionPool();

      /**
       * Returns the configuration object for the service.
       */
      coral::IConnectionServiceConfiguration& configuration() ;

      /**
       * Returns the monitoring reporter
       */
      const coral::IMonitoringReporter& monitoringReporter() const;

      /**
       * Returns the object controlling the web cache
       */
      coral::IWebCacheControl& webCacheControl() ;

      /// returns the reference to the owned Connection Pool
      ConnectionPool& connectionPool();

      /// removes a session from the registry
      void unRegisterSession( SessionProxy* sessionProxy );

      /// returns the number of idle session in the pool
      size_t numberOfIdleConnectionsInPool();

      /// returns the number of active session in the pool
      size_t numberOfActiveConnectionsInPool();

    private:

      ConnectionPool* m_connectionPool;
      ConnectionServiceConfiguration* m_configuration;
      WebCacheControl* m_webCacheControl;
      std::set<SessionProxy*> m_sessions;
      boost::mutex m_mutexLock;
    };

  }

}
#endif
