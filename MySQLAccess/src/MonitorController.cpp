// $Id: MonitorController.cpp,v 1.7 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <vector>
#include "CoralKernel/Context.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "MonitorController.h"
#include "SessionProperties.h"

coral::MySQLAccess::MonitorController::MonitorController( coral::MySQLAccess::SessionProperties& properties ) :
  m_properties( properties )
{
}


coral::MySQLAccess::MonitorController::~MonitorController()
{
}


void
coral::MySQLAccess::MonitorController::start( coral::monitor::Level level )
{
  coral::monitor::IMonitoringService* monitoringService = m_properties.monitoringService();
  if ( ! monitoringService )
  {
    coral::IHandle<coral::monitor::IMonitoringService> monService = coral::Context::instance().query<coral::monitor::IMonitoringService>();

    if( ! monService.isValid() )
      throw coral::MonitoringServiceNotFoundException( m_properties.domainProperties().service()->name(),
                                                       "IMonitoring::reportToOutputStream" );

    monitoringService = monService.get();
    m_properties.setMonitoringService( monitoringService );
  }
  monitoringService->setLevel( m_properties.connectionString(), level );
  monitoringService->enable( m_properties.connectionString() );
}


void
coral::MySQLAccess::MonitorController::stop()
{
  coral::monitor::IMonitoringService* monitoringService = m_properties.monitoringService();
  if ( monitoringService ) {
    monitoringService->disable( m_properties.connectionString() );
  }
}


void
coral::MySQLAccess::MonitorController::reportToOutputStream( std::ostream& os ) const
{
  coral::monitor::IMonitoringService* monitoringService = m_properties.monitoringService();

  if ( ! monitoringService )
  {
    throw coral::MonitoringServiceNotFoundException( m_properties.domainProperties().service()->name(),
                                                     "IMonitoring::reportToOutputStream" );
  }

  monitoringService->reporter().reportToOutputStream(  m_properties.connectionString(), os );
}


void
coral::MySQLAccess::MonitorController::report() const
{
  coral::monitor::IMonitoringService* monitoringService = m_properties.monitoringService();

  if ( ! monitoringService )
  {
    throw coral::MonitoringServiceNotFoundException( m_properties.domainProperties().service()->name(),
                                                     "IMonitoring::report" );
  }

  monitoringService->reporter().report( m_properties.connectionString() );
}


bool
coral::MySQLAccess::MonitorController::isActive() const
{
  return ( m_properties.monitoringService() != 0 ||
           m_properties.monitoringService()->active( m_properties.connectionString() ) );
}
