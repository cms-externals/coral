// $Id: SessionProperties.cpp,v 1.5.2.1 2010/12/20 09:10:09 avalassi Exp $

// Local include files
#include "Session.h"
#include "SessionProperties.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

SessionProperties::SessionProperties( const ConnectionProperties& connectionProperties,
                                      const std::string& databaseUrl,
                                      const coral::AccessMode accessMode,
                                      const Session& session )
  : m_connectionProperties( connectionProperties )
  , m_databaseUrl( databaseUrl )
  , m_accessMode( accessMode )
  , m_session( session )
  , m_sessionID( 0 )
  , m_fromProxy( false )
  , m_isUserSessionActive( false )
  , m_remoteTechnologyName( "" )
  , m_remoteServerVersion( "" )
{
}

//-----------------------------------------------------------------------------

SessionProperties::~SessionProperties()
{
}

//-----------------------------------------------------------------------------

void
SessionProperties::setState( coral::Token sessionID,
                             bool fromProxy,
                             bool isUserSessionActive )
{
  m_sessionID = sessionID;
  m_fromProxy = fromProxy;
  m_isUserSessionActive = isUserSessionActive;
}

//-----------------------------------------------------------------------------

void
SessionProperties::setRemoteProperties( const std::string& remoteTechnologyName,
                                        const std::string& remoteServerVersion,
                                        const std::string& remoteNominalSchemaName )
{
  m_remoteTechnologyName = remoteTechnologyName;
  m_remoteServerVersion = remoteServerVersion;
  m_remoteNominalSchemaName = remoteNominalSchemaName;
}

//-----------------------------------------------------------------------------

const ConnectionProperties& SessionProperties::connectionProperties() const
{
  return m_connectionProperties;
}

//-----------------------------------------------------------------------------

const std::string& SessionProperties::databaseUrl() const
{
  return m_databaseUrl;
}

//-----------------------------------------------------------------------------

coral::AccessMode SessionProperties::accessMode() const
{
  return m_accessMode;
}

//-----------------------------------------------------------------------------

coral::Token SessionProperties::sessionID() const
{
  return m_sessionID;
}

//-----------------------------------------------------------------------------

bool SessionProperties::fromProxy() const
{
  return m_fromProxy;
}

//-----------------------------------------------------------------------------

bool SessionProperties::isUserSessionActive() const
{
  return m_isUserSessionActive;
}

//-----------------------------------------------------------------------------

const std::string& SessionProperties::remoteTechnologyName() const
{
  return m_remoteTechnologyName;
}

//-----------------------------------------------------------------------------

const std::string& SessionProperties::remoteServerVersion() const
{
  return m_remoteServerVersion;
}

//-----------------------------------------------------------------------------

const std::string& SessionProperties::remoteNominalSchemaName() const
{
  return m_remoteNominalSchemaName;
}

//-----------------------------------------------------------------------------

bool SessionProperties::isTransactionActive() const
{
  return m_session.isTransactionActive();
}

//-----------------------------------------------------------------------------
