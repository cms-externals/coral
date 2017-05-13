// $Id: ConnectionProperties.cpp,v 1.3.2.1 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include "CoralBase/Exception.h"

// Local include files
#include "ConnectionProperties.h"
#include "DomainProperties.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

ConnectionProperties::ConnectionProperties( const DomainProperties& domainProperties,
                                            const std::string& coralServerUrl )
  : m_domainProperties( domainProperties )
  , m_coralServerUrl( coralServerUrl )
  , m_facade( 0 )
{
}

//-----------------------------------------------------------------------------

ConnectionProperties::~ConnectionProperties()
{
}

//-----------------------------------------------------------------------------

void
ConnectionProperties::setFacade( coral::ICoralFacade* facade )
{
  m_facade = facade;
}

//-----------------------------------------------------------------------------

const DomainProperties& ConnectionProperties::domainProperties() const
{
  return m_domainProperties;
}

//-----------------------------------------------------------------------------

std::string ConnectionProperties::coralServerUrl() const
{
  return m_coralServerUrl;
}

//-----------------------------------------------------------------------------

bool ConnectionProperties::isConnected() const
{
  return ( m_facade != 0 );
}

//-----------------------------------------------------------------------------

const coral::ICoralFacade&
ConnectionProperties::facade() const
{
  if ( !m_facade )
    throw coral::Exception( "No facade is available",
                            "ConnectionProperties::facade" ,
                            domainProperties().service()->name() );
  return *m_facade;
}

//-----------------------------------------------------------------------------
