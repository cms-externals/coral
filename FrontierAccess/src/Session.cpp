// $Id: Session.cpp,v 1.25 2011/03/22 10:36:50 avalassi Exp $
#include "Session.h"
#include "Connection.h"
#include "SessionPropertiesProxy.h"
#include "DomainProperties.h"
#include "MonitorController.h"
#include "ErrorHandler.h"
#include "Transaction.h"
#include "Schema.h"
#include "Statement.h"

#include "CoralCommon/URIParser.h"
#include "CoralCommon/MonitoringEventDescription.h"

#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IWebCacheControl.h"
#include "RelationalAccess/IWebCacheInfo.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/MessageStream.h"

#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"

#include "frontier_client/FrontierException.hpp"

#include <locale>

coral::FrontierAccess::Session::Session( coral::IDevConnection& connection,
                                         const DomainProperties& domainProperties,
                                         const std::string& connectionString,
                                         frontier::Connection& fconnection,
                                         boost::mutex& flock,
                                         const std::string& schemaName,
                                         const coral::ITypeConverter& converter )
  : coral::IDevSession( connection )
  , m_properties( new coral::FrontierAccess::SessionProperties( domainProperties, connectionString, fconnection, flock, schemaName, const_cast<coral::ITypeConverter&>(converter), *this ) )
  , m_connected( false )
  , m_monitorController( new coral::FrontierAccess::MonitorController( *m_properties ) )
  , m_serverVersion( "" )
  , m_schema( 0 )
  , m_transaction( 0 )
  , m_schemas()
{
}

coral::FrontierAccess::Session::~Session()
{
  for( std::map< std::string, std::pair< coral::FrontierAccess::SessionPropertiesProxy*, coral::FrontierAccess::Schema* > >::iterator
         iSchema = m_schemas.begin(); iSchema != m_schemas.end(); ++iSchema )
  {
    delete iSchema->second.second;
    delete iSchema->second.first;
  }

  delete m_transaction;
  delete m_schema; // fix bug #42189: delete transaction first, schema later!
  delete m_monitorController;
  delete m_properties;
}

coral::IMonitoring& coral::FrontierAccess::Session::monitoring()
{
  return *m_monitorController;
}

void coral::FrontierAccess::Session::startUserSession( const std::string& /*userName*/, const std::string& /*password*/ ) // Login & Password ignored for Frontier
{
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  // A message log
  // Setting the user name as schema name
  // This is obsoleted by the new connection string syntax --> m_properties->setSchemaName( userName );

  // If this the first schema we assume it is the nominal schema
  if( ! m_schema )
    m_schema = new coral::FrontierAccess::Schema( *m_properties );

  if( ! m_transaction )
    m_transaction = new coral::FrontierAccess::Transaction( *m_properties, *m_schema );

  // And the password - there is none for Frontier (yet)

  // Authenticating - there is none for Frontier (yet)

  // FIXME - (Do we ever do it for Frontier?) Enable sql trace

  log << coral::Debug << "Starting Frontier user session to: " << this->m_properties->connectionString() << " on schema: " << this->m_properties->schemaName() << coral::MessageStream::endmsg;

  // Record the beginning of the session
  if ( m_properties->monitoringService() )
  {
    m_properties->monitoringService()->record( m_properties->connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.sessionBegin() );
  }
}

bool coral::FrontierAccess::Session::isUserSessionActive() const
{
  return ( ! m_properties->log().empty() );
}

void coral::FrontierAccess::Session::endUserSession()
{
  if ( this->isUserSessionActive() )
  {

    // Abort any active transaction
    if ( this->transaction().isActive() )
      this->transaction().rollback();

    // FIXME - (Do we ever do it for Frontier?) Stop the tracing

    // We report the executed commands to Verbose channel & clear it
    coral::MessageStream log( m_properties->domainProperties().service()->name() );

    for( SessionLog::const_iterator sli = m_properties->log().begin(); sli != m_properties->log().end(); ++sli )
      log << coral::Verbose << "User: " << m_properties->schemaName() << " query: " << (*sli).sql << coral::MessageStream::endmsg;

    m_properties->log().clear();

    // Record the ending of the session
    if ( m_properties->monitoringService() )
    {
      m_properties->monitoringService()->record( m_properties->connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.sessionEnd() );
    }
  }
}

coral::ITransaction& coral::FrontierAccess::Session::transaction()
{
  return *m_transaction;
}

coral::ISchema& coral::FrontierAccess::Session::nominalSchema()
{
  return *m_schema;
}

coral::ISchema& coral::FrontierAccess::Session::schema( const std::string& schema )
{
  // Form the schema name correctly by setting everything to upper case
  std::string schemaName = schema;
  for ( std::string::size_type i = 0; i < schemaName.size(); ++i )
  {
    schemaName[i] = std::toupper( schemaName[i], std::locale::classic() );
  }

  // Check first is the schema with the corresponding name exists in the map
  std::map< std::string, std::pair< coral::FrontierAccess::SessionPropertiesProxy*, coral::FrontierAccess::Schema* > >::iterator iSchema = m_schemas.find( schemaName );

  if ( iSchema != m_schemas.end() )
    return *( iSchema->second.second );

  // Check in the database if a schema with such a name exists
  coral::AttributeList inBuffer; inBuffer.extend<std::string>( "user" ); inBuffer[0].data<std::string>() = schemaName;
  coral::AttributeList outBuffer; outBuffer.extend<std::string>( "USERNAME" );

  coral::FrontierAccess::Statement query( *m_properties, "SELECT USERNAME FROM ALL_USERS WHERE USERNAME=:\"user\"" );

  if( ! query.execute( inBuffer, false ) )
  {
    // FIXME - error reporting
    ;
  }

  query.defineOutput( outBuffer );
  query.fetchNext();

  std::string foundSchema = "";

  foundSchema = outBuffer[0].data<std::string>();

  if( foundSchema.empty() )
    throw coral::InvalidSchemaNameException( m_properties->domainProperties().service()->name(), "coral::FrontierAccess::Session::schema" );

  // The schema exists. Insert it into the map of the known ones
  coral::FrontierAccess::SessionPropertiesProxy* properties = new coral::FrontierAccess::SessionPropertiesProxy( *m_properties, schemaName );
  coral::FrontierAccess::Schema* newSchema = new coral::FrontierAccess::Schema( *properties );

  m_schemas.insert( std::make_pair( schemaName, std::make_pair( properties, newSchema ) ) );

  return *newSchema;
}
