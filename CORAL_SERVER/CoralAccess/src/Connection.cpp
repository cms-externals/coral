// $Id: Connection.cpp,v 1.18.2.2 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include <iostream>
#include "CoralCommon/URIParser.h"
#include "CoralKernel/Context.h"
#include "CoralServerBase/ICoralFacade.h"
#include "CoralServerBase/NotImplemented.h"
#include "CoralSockets/GenericSocketException.h"
#include "CoralStubs/ClientStub.h"
#include "CoralStubs/ServerStub.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/SessionException.h"

// Local include files
#include "Connection.h"
#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "Session.h"
#include "SocketClientWrapper.h"
#include "logger.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

Connection::Connection( const DomainProperties& domainProperties,
                        const std::string& coralServerUrl )
  : m_properties( new ConnectionProperties( domainProperties, coralServerUrl ) )
  , m_handler( 0 )
  , m_facade( 0 )
{
  logger << "Create Connection for " << m_properties->coralServerUrl() << endlog;
}

//-----------------------------------------------------------------------------

Connection::~Connection()
{
  logger << "Delete Connection for " << m_properties->coralServerUrl() << endlog;
  //logger << "Delete Connection for " << m_properties->coralServerUrl() << "... " << endlog;
  disconnect();
  if ( m_facade ) delete m_facade;
  m_facade = 0;
  //logger << "Delete Connection for " << m_properties->coralServerUrl() << "... 1" << endlog;
  if ( m_handler ) delete m_handler;
  m_handler = 0;
  //logger << "Delete Connection for " << m_properties->coralServerUrl() << "... 2" << endlog;
  if ( m_properties ) delete m_properties;
  m_properties = 0;
  //logger << "Delete Connection for " << m_properties->coralServerUrl() << "... DONE" << endlog;
}

//-----------------------------------------------------------------------------

void Connection::connect()
{
  logger << Always << "Connect to " << m_properties->coralServerUrl() << endlog;
  URIParser parser;
  parser.setURI( m_properties->coralServerUrl() );
  logger << Always << "Coral server technology: " << parser.technology() << endlog;
  logger << Always << "Coral server protocol:   " << parser.protocol() << endlog;
  logger << Always << "Coral server host:       " << parser.hostName() << endlog;
  int portNumber = parser.portNumber();
  logger << Always << "Coral server port:       " << portNumber << endlog;
  // Hack for bug #64446: use another port for the CORAL_2_3-patches nightlies
  // Keep this commented out in HEAD (only applies to CORAL_2_3-patches)
  std::string lcgNgtSltEnv( "LCG_NGT_SLT_NAME" );
  if ( ::getenv( lcgNgtSltEnv.c_str() ) )
  {
    std::string lcgNgtSlt( ::getenv( lcgNgtSltEnv.c_str() ) );
    logger << Always << "*** Env variable " << lcgNgtSltEnv << " is set to '"
           << lcgNgtSlt << "'" << endlog;
    // No need to check which slot we are in... we are using CORAL_2_3-patches
    logger << Always << "*** Coral server port will be overridden" << endlog;
    portNumber = 40009; // hardcoded
    logger << Always << "Coral server port:       " << portNumber << endlog;
  }
  //------------------------------------------
  // Production connections ("coral://...")
  //------------------------------------------
  if ( parser.protocol() == "" )
  {
    try
    {
      m_handler = new SocketClientWrapper( parser.hostName(), portNumber );
    }
    catch ( CoralSockets::GenericSocketException& e )
    {
      logger << Warning << "GenericSocketException caught: '" << e.what()
             << "' - rethrow as ConnectionException" << endlog;
      throw ConnectionException( "CoralAccess", "Connection::connect", e.what() );
    }
    m_facade = new CoralStubs::ClientStub( *m_handler );
    m_properties->setFacade( m_facade );
  }
  //------------------------------------------
  // Test connections ("coral_TEST://...")
  //------------------------------------------
  else if ( parser.protocol() == "TEST" )
  {
    logger << Warning << "*** WARNING! Create connection in TEST mode..." << endlog;
    if ( parser.portNumber() != 0 ) // Use the original portNumber from the URL
      throw Exception( "Port number must be 0 for connections in TEST mode",
                       "Connection::connect",
                       "coral::CoralAccess" );
    std::string host = parser.hostName();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Supported values:
    // --> coral_TEST://SFac&...
    // --> coral_TEST://CStb+SStb+SFac&...
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if ( host != "SFac" && host != "CStb+SStb+SFac" )
      throw Exception( "Invalid host '"+host+"'",
                       "Connection::connect",
                       "coral::CoralAccess" );
    // Load ConnectionService2
    std::string connSvc2Name = "CORAL/Services/ConnectionService2";
    IHandle<IConnectionService> connSvc2 = Context::instance().query<IConnectionService>( connSvc2Name );
    if ( !connSvc2.isValid() )
    {
      Context::instance().loadComponent( connSvc2Name );
      connSvc2 = Context::instance().query<IConnectionService>( connSvc2Name );
      if ( !connSvc2.isValid() )
        throw Exception( "Could not retrieve service "+connSvc2Name,
                         "Connection::connect",
                         "coral::CoralAccess" );
    }
    // Configure ConnectionService2
    connSvc2->configuration().disablePoolAutomaticCleanUp();
    connSvc2->configuration().setConnectionTimeOut(0);
    // Load ServerFacade
    std::string sFacadeSvcName = "CORAL/Services/CoralServerFacade";
    IHandle<ICoralFacade> hSFacade = Context::instance().query<ICoralFacade>( sFacadeSvcName );
    if ( !hSFacade.isValid() )
    {
      Context::instance().loadComponent( sFacadeSvcName );
      hSFacade = Context::instance().query<ICoralFacade>( sFacadeSvcName );
      if ( !hSFacade.isValid() )
        throw Exception( "Could not retrieve service "+sFacadeSvcName,
                         "Connection::connect",
                         "coral::CoralAccess" );
    }
    // +++ coral_TEST://SFac&...
    if ( host == "SFac" )
    {
      m_handler = 0;
      m_facade = 0;
      m_properties->setFacade( hSFacade.get() );
    }
    // +++ coral_TEST://CStb+SStb+SFac&...
    else
    {
      m_handler = new CoralStubs::ServerStub( *hSFacade );
      m_facade = new CoralStubs::ClientStub( *m_handler );
      m_properties->setFacade( m_facade );
    }
    logger << Warning << "*** WARNING! Create connection in TEST mode... DONE" << endlog;
  }
  //-----------------------------------------
  // Unknown connections ("coral_XXX://...")
  //-----------------------------------------
  else
  {
    throw Exception( "Protocol is not supported: '"+parser.protocol()+"'",
                     "Connection::connect",
                     "coral::CoralAccess" );
  }
}

//-----------------------------------------------------------------------------

coral::ISession*
Connection::newSession( const std::string& schemaName,
                        coral::AccessMode mode ) const
{
  if ( ! const_cast<Connection*>( this )->isConnected() )
    throw ConnectionNotActiveException
      ( m_properties->domainProperties().service()->name(), "IConnection::newSession" );

  // Initially, CoralAccess will only support R/O sessions:
  // R/W sessions are enabled only if an environment variable is set
  static int enableRW = -1;
  if ( enableRW == -1 ) enableRW = ( getenv( "CORALACCESS_ENABLERW" ) ? 1 : 0 );
  if ( mode != coral::ReadOnly && enableRW == 0 )
    throw NotImplemented( "Connection::newSession (R/W mode)" );

  // For the CoralAccess client, schemaName is the full database URI
  // (it is the second field returned by Domain::decodeUserConnectionString)
  const std::string& dbUrl = schemaName;
  IDevConnection* thisDevConnection = static_cast<IDevConnection*>( const_cast<Connection*>(this) );
  return new Session( *thisDevConnection, *m_properties, dbUrl, mode );
}

//-----------------------------------------------------------------------------

bool Connection::isConnected( bool /*probePhysicalConnection*/ )
{
  return m_properties->isConnected();
}

//-----------------------------------------------------------------------------

void Connection::disconnect()
{
  m_properties->setFacade( 0 ); // Deletes current facade if it exists
}

//-----------------------------------------------------------------------------

std::string Connection::serverVersion() const
{
  return "";
}

//-----------------------------------------------------------------------------

coral::ITypeConverter& Connection::typeConverter()
{
  throw NotImplemented("Connection::typeConverter");
}

//-----------------------------------------------------------------------------
