#include <iostream>
#include "MonitorController.h"
#include "SessionProperties.h"
#include "DomainProperties.h"

#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"
#include "RelationalAccess/SessionException.h"

#include "CoralCommon/IDevConnectionServiceConfiguration.h"

#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"

coral::SQLiteAccess::MonitorController::MonitorController( boost::shared_ptr<const SessionProperties> properties) :
  m_properties( properties )
{

}


coral::SQLiteAccess::MonitorController::~MonitorController()
{

}


void
coral::SQLiteAccess::MonitorController::start( coral::monitor::Level level )
{

  coral::IHandle<coral::IConnectionService> connService = coral::Context::instance().query<coral::IConnectionService>();
  if(connService.isValid()) {

    coral::monitor::IMonitoringService* monitoringService = m_properties->monitoringService();

    if ( monitoringService == NULL )
    {
      SessionProperties* properties = const_cast<SessionProperties*>( m_properties.get() );

      coral::IHandle<coral::monitor::IMonitoringService> monService = coral::Context::instance().query<coral::monitor::IMonitoringService>();

      if( ! monService.isValid() )
      {
        throw coral::MonitoringServiceNotFoundException( m_properties->domainProperties().service()->name() );
      }

      monitoringService = monService.get();

      properties->setMonitoringService( monitoringService );
    }

    monitoringService->setLevel( m_properties->connectionString(),level );
    monitoringService->enable( m_properties->connectionString() );
  } else {

    throw coral::MonitoringServiceNotFoundException( m_properties->domainProperties().service()->name() );

  }
}


void
coral::SQLiteAccess::MonitorController::stop()
{

  coral::monitor::IMonitoringService* monitoringService = m_properties->monitoringService();
  if ( monitoringService ) {
    monitoringService->disable( m_properties->connectionString() );
  }
  SessionProperties* properties = const_cast<SessionProperties*>( m_properties.get() );
  properties->setMonitoringService( 0 );
}


void
coral::SQLiteAccess::MonitorController::reportToOutputStream( std::ostream& os ) const
{

  coral::monitor::IMonitoringService* monitoringService = m_properties->monitoringService();
  if ( ! monitoringService ) {
    throw coral::MonitoringServiceNotFoundException( m_properties->domainProperties().service()->name(),
                                                     "IMonitoring::reportToOutputStream" );
  }
  monitoringService->reporter().reportToOutputStream( m_properties->domainProperties().service()->name(),os );
}


void
coral::SQLiteAccess::MonitorController::report() const
{

  coral::monitor::IMonitoringService* monitoringService = m_properties->monitoringService();
  if ( ! monitoringService ) {
    throw coral::MonitoringServiceNotFoundException( m_properties->domainProperties().service()->name(),
                                                     "IMonitoring::report" );
  }
  monitoringService->reporter().report( m_properties->domainProperties().service()->name() );
}


bool
coral::SQLiteAccess::MonitorController::isActive() const
{

  return ( m_properties->monitoringService() != 0 ||
           m_properties->monitoringService()->active( m_properties->connectionString() ) );
}
