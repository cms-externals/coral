// $Id: Domain.cpp,v 1.17 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralBase/MessageStream.h"
#include "CoralCommon/URIParser.h"

#include "Connection.h"
#include "Domain.h"
#include "DomainProperties.h"

coral::MySQLAccess::Domain::Domain( const std::string& componentName )
  : coral::Service( componentName )
  , m_flavorName( "MySQL" )
  , m_implementationName( "MySQL C API" )
  , m_implementationVersion( "" )
  , m_initialized( false )
{
  m_properties = new coral::MySQLAccess::DomainProperties( this );
  // Get the client library version
  m_implementationVersion = mysql_get_client_info();
  // Perform domain specific setup
  this->initializeDomain();
}

coral::MySQLAccess::Domain::~Domain()
{
  delete m_properties;
}

std::string coral::MySQLAccess::Domain::flavorName() const
{
  return m_flavorName;
}

std::string coral::MySQLAccess::Domain::implementationName() const
{
  return m_implementationName;
}

std::string coral::MySQLAccess::Domain::implementationVersion() const
{
  return m_implementationVersion;
}

coral::IConnection* coral::MySQLAccess::Domain::newConnection( const std::string& uriString ) const
{
  return new coral::MySQLAccess::Connection( *m_properties, uriString );
}

std::pair<std::string, std::string> coral::MySQLAccess::Domain::decodeUserConnectionString(const std::string& cs ) const
{
  // Processing the connection string
  coral::URIParser parser;
  parser.setURI( cs );
  std::ostringstream osDatabase;
  osDatabase << parser.hostName();
  int portNumber = parser.portNumber();
  if ( portNumber != 0 ) osDatabase << ":" << portNumber;

  std::string tempSchema = parser.databaseOrSchemaName();
  std::string::size_type slashPosition = tempSchema.find( "/" );
  if ( slashPosition != std::string::npos ) {
    osDatabase << "/" << tempSchema.substr( 0, slashPosition );
    tempSchema = tempSchema.substr( slashPosition + 1 );
  }

  // In MySQL we're case sensitive except Windows based servers
  std::string schemaName = tempSchema;
  //std::string schemaName = "";
  //for ( std::string::size_type i = 0; i < tempSchema.size(); ++i ) {
  //  schemaName += std::toupper( tempSchema[i], std::locale::classic() );
  //}

  return std::make_pair( std::string( osDatabase.str() ), schemaName );
}

void coral::MySQLAccess::Domain::initializeDomain()
{
  if( ! this->m_initialized )
  {
    //     coral::MessageStream log( this->name() );
    //
    //     coral::IHandle< seal::ComponentLoader > loader = localContext()->component< seal::ComponentLoader >();
    //
    //     if( loader )
    //     {
    //       log << seal::Msg::Debug << "MySQLAccess domain is loading its components" << seal::flush;
    //
    //       // Load MySQLAccess specific components
    //       loader->loadAll( localContext(), "CORAL/mysql/" );
    //     }
  }

  this->m_initialized = true;
}
