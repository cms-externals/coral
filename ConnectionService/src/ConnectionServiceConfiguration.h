#ifndef CONNECTIONSERVICE_CONNECTIONSERVICECONFIGURATION_H
#define CONNECTIONSERVICE_CONNECTIONSERVICECONFIGURATION_H

#include "CoralCommon/IDevConnectionServiceConfiguration.h"
#include "CoralKernel/IHandle.h"
#include "RelationalAccess/IMonitoringService.h"

namespace coral
{

  class Context;
  class IReplicaSortingAlgorithm;

  namespace ConnectionService
  {

    /// implementation of the interface IConnectionServiceConfiguration
    class ConnectionServiceConfiguration : public IDevConnectionServiceConfiguration
    {
    public:

      /// constructor
      ConnectionServiceConfiguration( const std::string& serviceName );

      /// destructor
      virtual ~ConnectionServiceConfiguration();

      /**
       * Enable the failing over to the next available
       * replica in case the current one is not available.
       * Otherwise the ConnectionService gives up.
       */
      void enableReplicaFailOver();

      /**
       * Disable the failing over to the next available
       * replica in case the current one is not available.
       */
      void disableReplicaFailOver();

      /**
       * Returns the failover mode
       */
      bool isReplicaFailoverEnabled() const;

      /**
       * Enables the sharing of the same physical connection
       * among more clients.
       */
      void enableConnectionSharing();

      /**
       * Disables the sharing of the same physical connection
       * among more clients.
       */
      void disableConnectionSharing();

      /**
       * Returns true if the connction sharing is enabled
       */
      bool isConnectionSharingEnabled() const;

      /**
       * Enables the re-use of Update connections for Read-Only sessions
       */
      void enableReadOnlySessionOnUpdateConnections();

      /**
       * Disables the re-use of Update connections for Read-Only sessions
       */
      void disableReadOnlySessionOnUpdateConnections();

      /**
       * Returns true if the  re-use of Update connections for Read-Only sessions is enabled
       */
      bool isReadOnlySessionOnUpdateConnectionsEnabled() const;

      /**
       * Sets the period of connection retrials (time interval between two retrials).
       */
      void setConnectionRetrialPeriod( int timeInSeconds );

      /**
       * Returns the rate of connection retrials (time interval between two retrials).
       */
      int connectionRetrialPeriod() const;

      /**
       * Sets the time out for the connection retrials before the connection
       * service fails over to the next available replica or quits.
       */
      void setConnectionRetrialTimeOut( int timeOutInSeconds );

      /**
       * Returns the time out for the connection retrials before the connection
       * service fails over to the next available replica or quits.
       */
      int connectionRetrialTimeOut() const;

      /**
       * Sets the connection time out in seconds.
       */
      void setConnectionTimeOut( int timeOutInSeconds );

      /**
       * Retrieves the connection time out in seconds.
       */
      int connectionTimeOut() const;

      /**
       * Activate the parallel thread for idle pool cleaning up
       */
      void enablePoolAutomaticCleanUp();

      /**
       * Disable the parallel thread for idle pool cleaning up
       */
      void disablePoolAutomaticCleanUp();

      /**
       * Returns true if the parallel thread for idle pool cleaning up is enabled
       */
      bool isPoolAutomaticCleanUpEnabled() const;

      /**
       * Sets the time duration of exclusion from failover list for a
       * connection not available.
       */
      void setMissingConnectionExclusionTime( int timeInSeconds );

      /**
       * Retrieves the time duration of exclusion from failover list for a
       * connection not available.
       */
      int missingConnectionExclusionTime() const;

      /**
       * Sets the monitoring level for the new sessions.
       */
      void setMonitoringLevel( coral::monitor::Level level );

      /**
       * Retrieves the current monitoring level.
       */
      coral::monitor::Level monitoringLevel() const ;

      /**
       * Loads and sets the authentication service to be used for the new sessions.
       */
      void setAuthenticationService( const std::string serviceName);

      /**
       * Loads and sets the lookup service to be used for the new sessions.
       */
      void setLookupService( const std::string& serviceName );

      /**
       * Loads and sets the relational service to be used for the new sessions.
       */
      void setRelationalService( const std::string& serviceName );

      /**
       * Loads and sets the monitoring service to be used for the new sessions.
       */
      void setMonitoringService( const std::string& serviceName );

      /**
       * Loads and sets the authentication service to be used for the new sessions.
       */
      void setAuthenticationService( IAuthenticationService& customAuthenticationService );

      /**
       * Loads and sets the default lookup service to be used for the new sessions.
       */
      void setLookupService( ILookupService& customLookupService );

      /**
       * Loads and sets the default relational service to be used for the new sessions.
       */
      void setRelationalService( IRelationalService& customRelationalService );

      /**
       * Loads and sets the default monitoring service to be used for the new sessions.
       */
      void setMonitoringService( monitor::IMonitoringService& customMonitoringService );

      /**
       * Sets the algorithm to be used for the database replica ordering.
       */
      void setReplicaSortingAlgorithm(coral::IReplicaSortingAlgorithm& algorithm);

      /// returns true if the monitoring level has been set
      bool isMonitoringEnabled() const;

      /// returns loads the reference to the required plugin. Loads default when required.
      coral::IRelationalService& relationalService() const;

      /// returns loads the reference to the required plugin. Loads default when required.
      coral::IAuthenticationService& authenticationService() const;

      /// returns loads the reference to the required plugin. Loads default when required.
      coral::ILookupService& lookupService() const;

      /// returns loads the reference to the required plugin. Loads default when required.
      coral::monitor::IMonitoringService& monitoringService() const;

      coral::IReplicaSortingAlgorithm* sortingAlgorithm() const;

      /// access to the service Context
      coral::Context* serviceContext() const;

      /// upper level service name
      const std::string& serviceName() const;

      /// access to the pool clean up period
      int poolCleanUpPeriod() const;

    private:

      /// loads the the specified Service
      void loadService( const std::string& serviceName ) const;

    private:

      /// the service context
      coral::Context& m_serviceContext;

      /// the upper level service name
      std::string m_serviceName;

      /// the flag enabling replica failover
      bool m_replicaFailover;

      /// the flag enabling connection sharing
      bool m_connectionSharing;

      /// the flag enabling Read-Only re-use of Update connection
      bool m_readOnlyOnUpdate;

      /// the flag enabling the automatic clean up
      bool m_automaticCleanUp;

      /// the number of connection retrials per minute
      int m_periodOfConnectionRetrials;

      /// the time out for the connection retrial
      int m_connectionRetrialTimeOut;

      /// the life time of the unused connections
      int m_connectionTimeOut;

      /// the time of the exclusion of unavailable connection
      int m_exclusionTime;

      /// the stored monitoring level
      coral::monitor::Level m_monitoringLevel;

      /// the name of the default authentication service
      std::string m_authenticationService;

      /// the name of the default lookup service
      std::string m_lookupService;

      /// the name of the default monitoring service
      std::string m_monitoringService;

      /// the name of the default relational service
      std::string m_relationalService;

      // the pointer of the custom service (owned by the external user)
      coral::IAuthenticationService* m_customAuthenticationService;

      // the pointer of the custom service (owned by the external user)
      coral::ILookupService* m_customLookupService;

      // the pointer of the custom service (owned by the external user)
      coral::IRelationalService* m_customRelationalService;

      // the pointer of the custom service (owned by the external user)
      coral::monitor::IMonitoringService* m_customMonitoringService;

      // the pointer of the custom service to be used;
      coral::IReplicaSortingAlgorithm* m_sortingAlgorithm;

      // the period of the pool clean up
      int m_poolCleanUpPeriod;

      // reference the monitoring service so that it outlives us (bug #73529)
      mutable IHandle<monitor::IMonitoringService> m_monitoringServiceHandle;

    };
  }

}

#endif
