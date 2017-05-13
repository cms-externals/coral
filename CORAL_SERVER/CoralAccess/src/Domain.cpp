// $Id: Domain.cpp,v 1.9.2.1 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include "CoralBase/Exception.h"

// Local include files
#include "Connection.h"
#include "Domain.h"
#include "DomainProperties.h"
#include "logger.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

Domain::Domain( const std::string& key )
  : Service( key )
  , m_flavorName( "Coral" )
  , m_implementationName( "CORAL_SERVER" )
  , m_implementationVersion( "1.0.0" )
  , m_properties( new DomainProperties( this ) )
{
  logger << "Create Domain for flavour '" << m_flavorName << "'" << endlog;
}

//-----------------------------------------------------------------------------

Domain::~Domain()
{
  logger << "Delete Domain" << endlog;
  delete m_properties;
}

//-----------------------------------------------------------------------------

std::string Domain::flavorName() const
{
  return m_flavorName;
}

//-----------------------------------------------------------------------------

std::string Domain::implementationName() const
{
  return m_implementationName;
}

//-----------------------------------------------------------------------------

std::string Domain::implementationVersion() const
{
  return m_implementationVersion;
}

//-----------------------------------------------------------------------------

coral::IConnection* Domain::newConnection( const std::string& uriString ) const
{
  logger << "Create newConnection for " << uriString << endlog;
  return new Connection( *m_properties, uriString );
}

//-----------------------------------------------------------------------------

std::pair<std::string, std::string >
Domain::decodeUserConnectionString( const std::string& uriString ) const
{
  // Split the coralServer and DB fields
  logger << "Parsing:    '" << uriString << "'" << endlog;
  std::string::size_type andPosition = uriString.find( "&" );
  if ( andPosition == std::string::npos )
    throw Exception( "Character '&' not found in URI '"+uriString+"'",
                     "Domain::decodeUserConnectionString",
                     "coral::CoralAccess" );
  std::string coralServerUri = uriString.substr( 0, andPosition );
  std::string dbUri = uriString.substr( andPosition + 1 );
  logger << "CoralServer URI:  " << coralServerUri << endlog;
  logger << "DB URI:     " << dbUri << endlog;
  return std::make_pair( coralServerUri, dbUri );
}

//-----------------------------------------------------------------------------
