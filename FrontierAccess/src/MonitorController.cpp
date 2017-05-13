#include "MonitorController.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"
#include "RelationalAccess/SessionException.h"
#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"


coral::FrontierAccess::MonitorController::MonitorController( coral::FrontierAccess::SessionProperties& properties ) :
  m_properties( properties )
{
}


coral::FrontierAccess::MonitorController::~MonitorController()
{
}


void
coral::FrontierAccess::MonitorController::start( coral::monitor::Level level )
{
  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = 
    coral::Context::instance().query<coral::monitor::IMonitoringService>("CORAL/Services/MonitoringService");

  if ( ! monitoringService.isValid() )
  {
    throw coral::MonitoringServiceNotFoundException( m_properties.domainProperties().service()->name() );
  }

  m_properties.setMonitoringService( monitoringService.get() );
  monitoringService->setLevel( m_properties.connectionString(), level );
  monitoringService->enable( m_properties.connectionString() );
}

void
coral::FrontierAccess::MonitorController::stop()
{
  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = m_properties.monitoringService();

  if ( monitoringService.isValid() )
  {
    monitoringService->disable( m_properties.connectionString() );
  }

  m_properties.setMonitoringService( 0 );
}

void
coral::FrontierAccess::MonitorController::reportToOutputStream( std::ostream& os ) const
{
  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = m_properties.monitoringService();

  if ( ! monitoringService.isValid() )
  {
    throw coral::MonitoringServiceNotFoundException( m_properties.domainProperties().service()->name(), "IMonitoring::reportToOutputStream" );
  }

  monitoringService->reporter().reportToOutputStream(  m_properties.connectionString(), os );
}

void
coral::FrontierAccess::MonitorController::report() const
{
  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = m_properties.monitoringService();

  if ( ! monitoringService.isValid() )
  {
    throw coral::MonitoringServiceNotFoundException( m_properties.domainProperties().service()->name(), "IMonitoring::report" );
  }

  monitoringService->reporter().report( m_properties.connectionString() );
}

bool
coral::FrontierAccess::MonitorController::isActive() const
{
  return ( m_properties.monitoringService() != 0 ||
           m_properties.monitoringService()->active( m_properties.connectionString() ) );
}
