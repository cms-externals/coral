#include "DomainProperties.h"

#include "Domain.h"

#include "RelationalAccess/IConnectionService.h"

#include "CoralBase/Exception.h"

#include "CoralKernel/Context.h"

boost::mutex coral::FrontierAccess::DomainProperties::s_lock{};

coral::FrontierAccess::DomainProperties::DomainProperties( coral::FrontierAccess::Domain* service ) :
  m_service( service ),
  m_tableSpaceForTables(""),
  m_tableSpaceForIndices(""),
  m_tableSpaceForLobs(""),
  m_lobChunkSize(0)
{
}


coral::FrontierAccess::DomainProperties::~DomainProperties()
{
}


void
coral::FrontierAccess::DomainProperties::setTableSpaceForTables( const std::string& tableSpace )
{
  m_tableSpaceForTables = tableSpace;
}


void
coral::FrontierAccess::DomainProperties::setTableSpaceForIndices( const std::string& tableSpace )
{
  m_tableSpaceForIndices = tableSpace;
}


void
coral::FrontierAccess::DomainProperties::setTableSpaceForLobs( const std::string& tableSpace )
{
  m_tableSpaceForLobs = tableSpace;
}


void
coral::FrontierAccess::DomainProperties::setLobChunkSize( int lobChunkSize )
{
  m_lobChunkSize = lobChunkSize;
}


coral::Service* coral::FrontierAccess::DomainProperties::service() const
{
  return dynamic_cast<coral::Service*>( this->m_service );
}


std::string
coral::FrontierAccess::DomainProperties::tableSpaceForTables() const
{
  return m_tableSpaceForTables;
}


std::string
coral::FrontierAccess::DomainProperties::tableSpaceForIndices() const
{
  return m_tableSpaceForIndices;
}


std::string
coral::FrontierAccess::DomainProperties::tableSpaceForLobs() const
{
  return m_tableSpaceForIndices;
}


int
coral::FrontierAccess::DomainProperties::lobChunkSize() const
{
  return m_lobChunkSize;
}

const coral::IWebCacheControl& coral::FrontierAccess::DomainProperties::cacheControl() const
{
  coral::IHandle< coral::IConnectionService > consvc = this->connectionService();

  if( ! consvc.isValid() )
    throw std::runtime_error( "Did not find coral::ConnectionService " );

  return consvc->webCacheControl();
}

coral::IHandle< coral::IConnectionService > coral::FrontierAccess::DomainProperties::connectionService() const
{
  coral::IHandle<coral::IConnectionService> cv = coral::Context::instance().query<coral::IConnectionService>();

  if( ! cv.isValid() )
    throw coral::Exception( "coral::FrontierAccess", "Could not access CORAL ConnectionService", "DomainProperties" );

  return cv;
}
