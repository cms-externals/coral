#ifndef CONNECTIONSERVICE_CONNECTIONPOOL_H
#define CONNECTIONSERVICE_CONNECTIONPOOL_H

#include "ConnectionMap.h"
#include "SessionHandle.h"
#include "CoralBase/boost_thread_headers.h"

namespace coral {

  class ICertificateData;

  namespace ConnectionService {

    class ReplicaCatalogue;
    class ConnectionServiceConfiguration;

    /// class describing the storage of the Connection pool
    class ConnectionPool {

    public:

      /// constructor
      explicit ConnectionPool( const ConnectionServiceConfiguration& configuration );

      /// destructor
      virtual ~ConnectionPool();

      /// combines the two above methods
      SessionHandle getValidSession( const std::string& connectionString,
                                     const std::string& role, const AccessMode accessMode,
                                     const coral::ICertificateData *cert=0 );

      /// releases the use of the specified connection
      bool releaseConnection( ConnectionHandle& connection );

      /// releases the use of the specified connection
      bool removeConnection( ConnectionHandle& connection );

      /// removes the connections from the pool
      void clearConnectionPool();

      /// remove from the pool the connection timed out.
      void cleanUpTimedOutConnections();

      /// returns the number of idle session in the pool
      size_t numberOfIdleConnections();

      /// returns the number of active session in the pool
      size_t numberOfActiveConnections();

      /// returns false if the wait was interupted by stopping the thread
      bool waitForTimeout();

    private:

      /// initialize the pool when required
      void startPool();

      // returns the handle to a new constructed connection, registered in the in-use connection pool
      SessionHandle getSessionFromNewConnection( const ConnectionParams& connectionParameters,
                                                 AccessMode accessMode );

      /// returns the handle to a re-used connection from the pool, applying the subscribed policy.
      SessionHandle getSessionFromConnectionPool( const ConnectionParams& connectionParameters,
                                                  AccessMode accessMode );

    private:
      /// the free connection pool
      ConnectionMap m_idleConnections;

      /// the used connection pool
      ConnectionMap m_activeConnections;

      /// the reference to the service configuration
      const ConnectionServiceConfiguration& m_connectionServiceConfiguration;

      /// the connection replica service;
      ReplicaCatalogue* m_replicaCatalogue;

      /// thread for the pool clean up
      boost::thread* m_timerThread;

      /// flag for the status of the timer
      boost::timed_mutex m_finalized;

      /// semaphores for the parallel timer execution
      boost::mutex m_mutexForPool;
      boost::mutex m_mutexForThread;

      bool m_poolStarted;

    };

  }

}

#endif
