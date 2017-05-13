// $Id: Session.cpp,v 1.12.2.1 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include <iostream>
#include "RelationalAccess/SessionException.h"
#include "CoralServerBase/NotImplemented.h"

// Local include files
#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "Schema.h"
#include "Session.h"
#include "SessionProperties.h"
#include "Transaction.h"
#include "logger.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

Session::Session( coral::IDevConnection& connection,
                  const ConnectionProperties& connectionProperties,
                  const std::string& dbUrl,
                  const coral::AccessMode mode )
  : coral::IDevSession( connection )
  , m_properties( new SessionProperties( connectionProperties, dbUrl, mode, *this ) )
  , m_schemas()
  , m_transaction( 0 )
{
  logger << "Create Session for " << m_properties->databaseUrl()
         << " in mode " << m_properties->accessMode() << endlog;
}

//-----------------------------------------------------------------------------

Session::~Session()
{
  logger << "Delete Session for " << m_properties->databaseUrl()
         << " in mode " << m_properties->accessMode() << endlog;
  if ( m_transaction ) delete m_transaction;
  m_transaction = 0;
  for ( std::map<std::string,Schema*>::const_iterator
          iSchema = m_schemas.begin(); iSchema != m_schemas.end(); iSchema++ ) delete iSchema->second;
  m_schemas.clear();
  try { if ( this->isUserSessionActive() ) this->endUserSession(); }
  catch( ... ) { logger << "WARNING! Exception caught in ~Session" << endlog; }
  if ( m_properties ) delete m_properties;
  m_properties = 0;
}

//-----------------------------------------------------------------------------

coral::IMonitoring& Session::monitoring()
{
  throw NotImplemented("Session::monitoring");
}

//-----------------------------------------------------------------------------

void Session::startUserSession( const std::string& userName,
                                const std::string& password )
{
  logger << "Start user session for '" << userName
         << "' with password '" << password << "'" << endlog;
  try
  {
    bool fromProxy = false;
    /*
    static int caproxy = -1;
    if ( caproxy == -1 ) caproxy = ( getenv ( "CORALACCESS_SETFROMPROXY" ) ? 1 : 0 );
    if ( caproxy > 0 )
    {
      std::cout << "WARNING! Set fromProxy=true in Session::startUserSession" << std::endl;
      fromProxy = true;
    }
    */
    coral::Token sessionID = facade().connect( m_properties->databaseUrl(), m_properties->accessMode(), fromProxy );
    if ( fromProxy ) logger << "Started user session #" << sessionID << " (over a proxy)" << endlog;
    else logger << "Started user session #" << sessionID << " (NOT over a proxy)" << endlog;
    m_properties->setState( sessionID, fromProxy, true );
    logger << "Retrieve the properties of the remote session" << endlog;
    const std::vector<std::string> remoteSessionProperties = facade().fetchSessionProperties( sessionID );
    if ( remoteSessionProperties.size() != 3 )
      throw coral::Exception( "Wrong number of remote session properties retrieved",
                              "Session::startUserSession" ,
                              m_properties->connectionProperties().domainProperties().service()->name() );
    m_properties->setRemoteProperties( remoteSessionProperties[0],
                                       remoteSessionProperties[1],
                                       remoteSessionProperties[2] );
  }
  catch ( std::exception& e )
  {
    logger << Error << "Exception caught in Session::startUserSession: "
           << e.what() << endlog;
    throw;
  }
}

//-----------------------------------------------------------------------------

void Session::endUserSession()
{
  logger << "Delete user session #"
         << m_properties->sessionID() << endlog;
  facade().releaseSession( m_properties->sessionID() );
  m_properties->setRemoteProperties( "", "", "" );
  m_properties->setState( 0, false, false );
}

//-----------------------------------------------------------------------------

bool Session::isUserSessionActive() const
{
  return m_properties->isUserSessionActive();
}

//-----------------------------------------------------------------------------

coral::ITransaction& Session::transaction()
{
  if ( !m_transaction ) m_transaction = new Transaction( *m_properties );
  return *m_transaction;
}

//-----------------------------------------------------------------------------

coral::ISchema& Session::nominalSchema()
{
  if ( ! this->isUserSessionActive() )
    throw coral::ConnectionNotActiveException( m_properties->connectionProperties().domainProperties().service()->name(),
                                               "Session::nominalSchema" );
  std::string schemaName = m_properties->remoteNominalSchemaName();
  return schema( schemaName );
}

//-----------------------------------------------------------------------------

coral::ISchema& Session::schema( const std::string& schemaName )
{
  if ( schemaName == "" )
    throw coral::Exception( "Invalid schema name ''",
                            "Session::schemaName" ,
                            m_properties->connectionProperties().domainProperties().service()->name() );
  logger << "Get schema '" << schemaName << "'" << endlog;
  if ( m_schemas.find( schemaName ) == m_schemas.end() )
    m_schemas[ schemaName ] = new Schema( *m_properties, schemaName );
  logger << "Got schema '" << schemaName << "'" << endlog;
  return *( m_schemas[ schemaName ] );
}

//-----------------------------------------------------------------------------

std::string Session::remoteTechnologyName() const
{
  if ( ! this->isUserSessionActive() )
    throw coral::ConnectionNotActiveException( m_properties->connectionProperties().domainProperties().service()->name(),
                                               "Session::remoteTechnologyName" );
  return m_properties->remoteTechnologyName();
}

//-----------------------------------------------------------------------------

std::string Session::remoteServerVersion() const
{
  if ( ! this->isUserSessionActive() )
    throw coral::ConnectionNotActiveException( m_properties->connectionProperties().domainProperties().service()->name(),
                                               "Session::remoteTechnologyName" );
  return m_properties->remoteServerVersion();
}

//-----------------------------------------------------------------------------

const coral::ICoralFacade& Session::facade() const
{
  return m_properties->connectionProperties().facade();
}

//-----------------------------------------------------------------------------

bool Session::isTransactionActive() const
{
  if ( !m_transaction ) return false;
  else return m_transaction->isActive();
}

//-----------------------------------------------------------------------------
