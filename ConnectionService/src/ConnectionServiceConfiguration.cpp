#include <cstdlib> // for atol
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "CoralBase/MessageStream.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/RefCounted.h"
#include "ConnectionServiceConfiguration.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IRelationalService.h"

/// constructor
coral::ConnectionService::ConnectionServiceConfiguration::ConnectionServiceConfiguration( const std::string& serviceName ) :
  IDevConnectionServiceConfiguration(),
  m_serviceContext( coral::Context::instance() ),
  m_serviceName( serviceName ),
  m_replicaFailover( true ),
  m_connectionSharing( true ),
  m_readOnlyOnUpdate( true ),
  m_automaticCleanUp( true ),
  m_periodOfConnectionRetrials( 10 ),
  m_connectionRetrialTimeOut( 60 ),
  m_connectionTimeOut( 300 ),
  m_exclusionTime( 3600 ),
  m_monitoringLevel( coral::monitor::Off ),
  m_authenticationService( "" ),
  m_lookupService( "" ),
  m_monitoringService( "CORAL/Services/MonitoringService" ),
  m_relationalService( "CORAL/Services/RelationalService" ),
  m_customAuthenticationService(0),
  m_customLookupService(0),
  m_customRelationalService(0),
  m_customMonitoringService(0),
  m_sortingAlgorithm(0),
  m_poolCleanUpPeriod( 10 ),
  m_monitoringServiceHandle()
{
  coral::MessageStream log( m_serviceName );

  char* userAuthSvc = ::getenv ( "CORAL_AUTH_SERVICE_LIBRARY" );
  if ( userAuthSvc ) {
    log << coral::Debug << "CORAL_AUTH_SERVICE_LIBRARY sets user-defined authentication service '" << userAuthSvc << "'" << coral::MessageStream::endmsg;
    m_authenticationService = "CORAL/Services/UserDefinedAuthenticationService";
  }
  else if ( ::getenv ( "CORAL_AUTH_PATH" ) ) {
    m_authenticationService = "CORAL/Services/XMLAuthenticationService";
  }
  else {
    if ( ::getenv ( "CORAL_AUTH_USER" ) && ::getenv ( "CORAL_AUTH_PASSWORD" ) )
      m_authenticationService = "CORAL/Services/EnvironmentAuthenticationService";
    else
      m_authenticationService = "CORAL/Services/XMLAuthenticationService";
  }

  char* userLookupSvc = ::getenv ( "CORAL_LOOKUP_SERVICE_LIBRARY" );
  if ( userLookupSvc ) {
    log << coral::Debug << "CORAL_LOOKUP_SERVICE_LIBRARY sets user-defined lookup service '" << userLookupSvc << "'" << coral::MessageStream::endmsg;
    m_lookupService = "CORAL/Services/UserDefinedLookupService";
  }
  else {
    m_lookupService = "CORAL/Services/XMLLookupService";
  }

  if ( m_connectionSharing )
    log << coral::Info << "Connection sharing is enabled" << coral::MessageStream::endmsg;
  else
    log << coral::Info << "Connection sharing is disabled" << coral::MessageStream::endmsg;

  char* cleanupPeriod = ::getenv ( "CORAL_CONNECTIONPOOL_CLEANUPPERIOD" );
  if ( cleanupPeriod ) {
    log << coral::Info << "CORAL_CONNECTIONPOOL_CLEANUPPERIOD '"
        << cleanupPeriod << "' sets new connection pool cleanup period to "
        << atol(cleanupPeriod) << " seconds" << coral::MessageStream::endmsg;
    if ( atol(cleanupPeriod) == 0 )
      log << coral::Warning << "Invalid CORAL_CONNECTIONPOOL_CLEANUPPERIOD: "
          << "keep default value of " << m_poolCleanUpPeriod
          << " seconds" << coral::MessageStream::endmsg;
    else
      m_poolCleanUpPeriod = atol(cleanupPeriod);
  } else {
    log << coral::Info << "Connection pool cleanup period (default value) is "
        << m_poolCleanUpPeriod << " seconds" << coral::MessageStream::endmsg;
  }
}

/// destructor
coral::ConnectionService::ConnectionServiceConfiguration::~ConnectionServiceConfiguration()
{
  // The plug-ins constructed by the users are NOT owned by CORAL.
}

/// enables the sharing of the same physical connection among more clients.
void
coral::ConnectionService::ConnectionServiceConfiguration::enableConnectionSharing(){
  m_connectionSharing = true;
  coral::MessageStream log( m_serviceName );
  log << coral::Info << "Enable connection sharing" << coral::MessageStream::endmsg;
}

/// disables the sharing of the same physical connection among more clients.
void
coral::ConnectionService::ConnectionServiceConfiguration::disableConnectionSharing(){
  m_connectionSharing = false;
  coral::MessageStream log( m_serviceName );
  log << coral::Info << "Disable connection sharing" << coral::MessageStream::endmsg;
}

/// loads the required plugins according to defaults
coral::IRelationalService&
coral::ConnectionService::ConnectionServiceConfiguration::relationalService() const
{
  if(m_customRelationalService) {
    return *m_customRelationalService;
  }

  coral::IHandle<coral::IRelationalService> relSvc = coral::Context::instance().query<coral::IRelationalService>( m_relationalService );

  if ( ! relSvc.isValid() )
  {
    coral::MessageStream log( m_serviceName );
    log << coral::Info << "Loading default plugin for coral::IRelationalService: " <<m_relationalService << coral::MessageStream::endmsg;
    loadService( m_relationalService );
    relSvc = m_serviceContext.query<coral::IRelationalService>( m_relationalService );
    if( ! relSvc.isValid() )
    {
      std::ostringstream mess;
      mess << "Selected plugin "<< m_relationalService <<" has not been found.";
      log << coral::Error << mess.str() << coral::MessageStream::endmsg;

      throw ConnectionServiceException(mess.str(), "ConnectionService::ConnectionServiceConfiguration::relationalService");
    }
  }

  return *relSvc;
}

/// loads the required plugins according to defaults
coral::IAuthenticationService&
coral::ConnectionService::ConnectionServiceConfiguration::authenticationService() const
{
  if(m_customAuthenticationService) {
    return *m_customAuthenticationService;
  }

  coral::IHandle<coral::IAuthenticationService> authSvc = m_serviceContext.query<coral::IAuthenticationService>( m_authenticationService );

  if ( ! authSvc.isValid() )
  {
    coral::MessageStream log( m_serviceName );
    log << coral::Info << "Loading default plugin for coral::IAuthenticationService: " << m_authenticationService << coral::MessageStream::endmsg;
    loadService( m_authenticationService );
    authSvc = m_serviceContext.query<coral::IAuthenticationService>( m_authenticationService );

    if ( ! authSvc.isValid() )
    {
      std::ostringstream mess;
      mess << "Selected plugin "<< m_authenticationService <<" has been not found.";
      log << coral::Error << mess.str() << coral::MessageStream::endmsg;

      throw ConnectionServiceException(mess.str(), "ConnectionService::ConnectionServiceConfiguration::authenticationService");
    }
  }

  return *authSvc;
}

/// loads the required plugins according to defaults
coral::ILookupService&
coral::ConnectionService::ConnectionServiceConfiguration::lookupService() const
{
  if(m_customLookupService) {
    return *m_customLookupService;
  }

  coral::IHandle<coral::ILookupService> lookupSvc = m_serviceContext.query<coral::ILookupService>( m_lookupService );

  if ( ! lookupSvc.isValid() )
  {
    coral::MessageStream log( m_serviceName );
    log << coral::Info << "Loading default plugin for coral::ILookupService: " << m_lookupService << coral::MessageStream::endmsg;
    loadService( m_lookupService );
    lookupSvc = m_serviceContext.query<coral::ILookupService>( m_lookupService );
    if( ! lookupSvc.isValid() )
    {
      std::ostringstream mess;
      mess << "Selected plugin "<< m_lookupService <<" has been not found.";
      log << coral::Error << mess.str() << coral::MessageStream::endmsg;

      throw ConnectionServiceException(mess.str(), "ConnectionService::ConnectionServiceConfiguration::lookupService");
    }
  }

  return *lookupSvc;
}

/// loads the required plugin according to defaults
coral::monitor::IMonitoringService&
coral::ConnectionService::ConnectionServiceConfiguration::monitoringService() const
{
  if(m_customMonitoringService) {
    return *m_customMonitoringService;
  }

  coral::IHandle<coral::monitor::IMonitoringService> monSvc = m_serviceContext.query<coral::monitor::IMonitoringService>( m_monitoringService );
  if ( ! monSvc.isValid() )
  {
    coral::MessageStream log( m_serviceName );
    log << coral::Info << "Loading default plugin for coral::IMonitoringService: " << m_monitoringService << coral::MessageStream::endmsg;
    loadService( m_monitoringService );
    monSvc = m_serviceContext.query<coral::monitor::IMonitoringService>( m_monitoringService );
    if( ! monSvc.isValid() )
    {
      std::ostringstream mess;
      mess << "Selected plugin "<< m_monitoringService <<" has been not found.";
      log << coral::Error << mess.str() << coral::MessageStream::endmsg;

      throw ConnectionServiceException(mess.str(), "ConnectionService::ConnectionServiceConfiguration::monitoringService");
    }
  }
  m_monitoringServiceHandle = monSvc; // set monitoring svc handle (bug #73529)
  return *monSvc;
}

/**
 * Loads and sets the authentication service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setAuthenticationService( const std::string serviceName) {
  m_authenticationService = serviceName;
  m_customAuthenticationService = 0;
  coral::MessageStream log( m_serviceName );
  log << coral::Info << "Loading default plugin for coral::IAuthenticationService: " << m_authenticationService << coral::MessageStream::endmsg;
  loadService( m_authenticationService );
}

/**
 * Loads and sets the lookup service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setLookupService( const std::string& serviceName ){
  m_lookupService = serviceName;
  m_customLookupService = 0;
  coral::MessageStream log( m_serviceName );
  log << coral::Info << "Loading default plugin for coral::ILookupService: " << m_lookupService << coral::MessageStream::endmsg;
  loadService( m_lookupService );
}

/**
 * Loads and sets the relational service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setRelationalService( const std::string& serviceName ){
  m_relationalService = serviceName;
  m_customRelationalService = 0;
  coral::MessageStream log( m_serviceName );
  log << coral::Info << "Loading default plugin for coral::IRelationalService: " << m_relationalService << coral::MessageStream::endmsg;
  loadService( m_relationalService );
}

/**
 * Loads and sets the monitoring service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setMonitoringService( const std::string& serviceName )
{
  m_monitoringService = serviceName;
  m_customMonitoringService = 0;
  coral::MessageStream log( m_serviceName );
  log << coral::Info << "Loading default plugin for coral::IMonitoringService: " << m_monitoringService << coral::MessageStream::endmsg;
  loadService( m_monitoringService );
  monitoringService(); // set monitoring svc handle (bug #73529)
}

/// access to the service Context
coral::Context*
coral::ConnectionService::ConnectionServiceConfiguration::serviceContext() const
{
  return &m_serviceContext;
}

/// access to the upper levevl service name
const std::string&
coral::ConnectionService::ConnectionServiceConfiguration::serviceName() const
{
  return m_serviceName;
}

/// loads the the specified Service
void
coral::ConnectionService::ConnectionServiceConfiguration::loadService( const std::string& serviceName ) const
{
  // Load a CORAL service by name
  coral::Context::instance().loadComponent( serviceName );
}

/**
 * Enable the failing over to the next available
 * replica in case the current one is not available.
 * Otherwise the ConnectionService gives up.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::enableReplicaFailOver(){
  m_replicaFailover = true;
}

/**
 * Disable the failing over to the next available
 * replica in case the current one is not available.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::disableReplicaFailOver(){
  m_replicaFailover = false;
}

/**
 * Returns the failover mode
 */
bool
coral::ConnectionService::ConnectionServiceConfiguration::isReplicaFailoverEnabled() const {
  return m_replicaFailover;
}

/**
 * Returns true if the connction sharing is enabled
 */
bool
coral::ConnectionService::ConnectionServiceConfiguration::isConnectionSharingEnabled() const {
  return m_connectionSharing;
}

/**
 * Enables the re-use of Update connections for Read-Only sessions
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::enableReadOnlySessionOnUpdateConnections(){
  m_readOnlyOnUpdate = true;
}

/**
 * Disables the re-use of Update connections for Read-Only sessions
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::disableReadOnlySessionOnUpdateConnections(){
  m_readOnlyOnUpdate = false;
}

/**
 * Returns true if the  re-use of Update connections for Read-Only sessions is enabled
 */
bool
coral::ConnectionService::ConnectionServiceConfiguration::isReadOnlySessionOnUpdateConnectionsEnabled() const {
  return m_readOnlyOnUpdate;
}

/**
 * Sets the period of connection retrials (time interval between two retrials).
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setConnectionRetrialPeriod( int timeInSeconds ){
  m_periodOfConnectionRetrials = timeInSeconds;
}

/**
 * Returns the rate of connection retrials (time interval between two retrials).
 */
int
coral::ConnectionService::ConnectionServiceConfiguration::connectionRetrialPeriod() const {
  return m_periodOfConnectionRetrials;
}

/**
 * Sets the time out for the connection retrials before the connection
 * service fails over to the next available replica or quits.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setConnectionRetrialTimeOut( int timeOutInSeconds ){
  m_connectionRetrialTimeOut = timeOutInSeconds;
}

/**
 * Returns the time out for the connection retrials before the connection
 * service fails over to the next available replica or quits.
 */
int
coral::ConnectionService::ConnectionServiceConfiguration::connectionRetrialTimeOut() const {
  return m_connectionRetrialTimeOut;
}

/**
 * Sets the connection time out in seconds.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setConnectionTimeOut( int timeOutInSeconds ){
  m_connectionTimeOut = timeOutInSeconds;
}

/**
 * Retrieves the connection time out in seconds.
 */
int
coral::ConnectionService::ConnectionServiceConfiguration::connectionTimeOut() const {
  return m_connectionTimeOut;
}

/**
 * Activate the parallel thread for idle pool cleaning up
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::enablePoolAutomaticCleanUp(){
  m_automaticCleanUp = true;
}

/**
 * Disable the parallel thread for idle pool cleaning up
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::disablePoolAutomaticCleanUp(){
  m_automaticCleanUp = false;
}

/**
 * Returns true if the parallel thread for idle pool cleaning up is enabled
 */
bool
coral::ConnectionService::ConnectionServiceConfiguration::isPoolAutomaticCleanUpEnabled() const {
  return m_automaticCleanUp;
}

/**
 * Sets the time duration of exclusion from failover list for a
 * connection not available.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setMissingConnectionExclusionTime( int timeInSeconds )
{
  m_exclusionTime = timeInSeconds;
}

/**
 * Retrieves the time duration of exclusion from failover list for a
 * connection not available.
 */
int
coral::ConnectionService::ConnectionServiceConfiguration::missingConnectionExclusionTime() const {
  return m_exclusionTime;
}

/**
 * Sets the monitoring level for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setMonitoringLevel( coral::monitor::Level level ){
  m_monitoringLevel = level;
}

/**
 * Loads and sets the authentication service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setAuthenticationService( IAuthenticationService& customAuthenticationService ){
  m_customAuthenticationService = &customAuthenticationService;
}

/**
 * Loads and sets the default lookup service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setLookupService( ILookupService& customLookupService ){
  m_customLookupService = &customLookupService;
}

/**
 * Loads and sets the default relational service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setRelationalService( IRelationalService& customRelationalService ){
  m_customRelationalService = &customRelationalService;
}

/**
 * Loads and sets the default monitoring service to be used for the new sessions.
 */
void
coral::ConnectionService::ConnectionServiceConfiguration::setMonitoringService( monitor::IMonitoringService& customMonitoringService )
{
  m_customMonitoringService = &customMonitoringService;
  m_monitoringServiceHandle = 0; // reset monitoring svc handle (bug #73529)
}

void
coral::ConnectionService::ConnectionServiceConfiguration::setReplicaSortingAlgorithm(coral::IReplicaSortingAlgorithm& algorithm)
{
  m_sortingAlgorithm = &algorithm;
}

/**
 * Retrieves the current monitoring level.
 */
coral::monitor::Level
coral::ConnectionService::ConnectionServiceConfiguration::monitoringLevel() const {
  return m_monitoringLevel;
}

coral::IReplicaSortingAlgorithm*
coral::ConnectionService::ConnectionServiceConfiguration::sortingAlgorithm() const {
  return m_sortingAlgorithm;
}

/// returns true if the monitoring level has been set
bool
coral::ConnectionService::ConnectionServiceConfiguration::isMonitoringEnabled() const {
  return m_monitoringLevel != coral::monitor::Off;
}

/// access to the pool clean up period
int
coral::ConnectionService::ConnectionServiceConfiguration::poolCleanUpPeriod() const
{
  return m_poolCleanUpPeriod;
}
