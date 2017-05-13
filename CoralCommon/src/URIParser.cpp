#include "CoralCommon/URIParser.h"
#include "CoralCommon/URIException.h"
#include <sstream>

coral::URIParser::URIParser() :
  m_URI( "" ),
  m_technology( "" ),
  m_protocol( "" ),
  m_hostName( "" ),
  m_databaseOrSchemaName( "" ),
  m_portNumber( 0 ),
  m_needsParsing( true )
{
}

coral::URIParser::~URIParser()
{
}

void
coral::URIParser::setURI( const std::string& URIString )
{
  if ( URIString != m_URI ) {
    m_URI = URIString;
    m_technology = m_protocol = m_hostName = m_databaseOrSchemaName = "";
    m_portNumber = 0;
    m_needsParsing = true;
  }
}

const std::string&
coral::URIParser::getURI() const
{
  return m_URI;
}

const std::string&
coral::URIParser::technology() const
{
  if ( m_needsParsing ) const_cast< coral::URIParser* >( this )->parse();
  return m_technology;
}

const std::string&
coral::URIParser::protocol() const
{
  if ( m_needsParsing ) const_cast< coral::URIParser* >( this )->parse();
  return m_protocol;
}

const std::string&
coral::URIParser::hostName() const
{
  if ( m_needsParsing ) const_cast< coral::URIParser* >( this )->parse();
  return m_hostName;
}

const std::string
coral::URIParser::databaseOrSchemaName() const
{
  if ( m_needsParsing ) const_cast< coral::URIParser* >( this )->parse();
  return m_databaseOrSchemaName;
}

int
coral::URIParser::portNumber() const
{
  if ( m_needsParsing ) const_cast< coral::URIParser* >( this )->parse();
  return m_portNumber;
}

void
coral::URIParser::parse()
{
  m_needsParsing = false;

  // Retrieve the technology and the protocol first
  std::string::size_type doubleColonPosition = m_URI.find( ":" );
  if ( doubleColonPosition == std::string::npos )
    throw coral::URIException( m_URI );

  std::string technologyAndProtocol = m_URI.substr( 0, doubleColonPosition );

  // Separate the technology from the protocol
  std::string::size_type underscorePosition = technologyAndProtocol.find( "_" );
  if ( underscorePosition == std::string::npos ) {
    m_technology = technologyAndProtocol;
  }
  else {
    m_technology = technologyAndProtocol.substr( 0, underscorePosition );
    if ( underscorePosition + 1 < technologyAndProtocol.size() )
      m_protocol = technologyAndProtocol.substr( underscorePosition + 1 );
  }

  // Separate the host and the port number from the database
  if ( doubleColonPosition + 1 < m_URI.size() ) {
    std::string path = m_URI.substr( doubleColonPosition + 1 );

    // Check if it is a relative path: technology_protocol:localpath.ext
    if ( path[0] != '/' ) {
      m_databaseOrSchemaName = path;
      return;
    }

    if ( path.size() > 1 ) {
      // Check whether the host is ommitted: technology_protocol:/absolutepath.ext
      // or                                : technology:/dbName
      if ( path[1] != '/' ) {
        if ( m_protocol.empty() ) {
          m_databaseOrSchemaName = path.substr(1);
        }
        else {
          m_databaseOrSchemaName = path;
        }
        return;
      }
      else { // Check for the general case echnology_protocol://host:port/db
        if ( path.size() > 2 ) {
          path = path.substr( 2 );
          std::string::size_type slashPosition = path.find( "/" );
          if ( slashPosition != std::string::npos ) {
            if ( slashPosition != 0 ) { // Separate the host from the port
              std::string hostAndPort = path.substr( 0, slashPosition );
              std::string::size_type otherDoubleColonPosition = hostAndPort.find( ":" );
              if ( otherDoubleColonPosition != std::string::npos ) {
                m_hostName = hostAndPort.substr( 0, otherDoubleColonPosition );
                if ( otherDoubleColonPosition + 1 < hostAndPort.size() ) {
                  std::istringstream is( hostAndPort.substr( otherDoubleColonPosition + 1 ).c_str() );
                  is >> m_portNumber;
                }
              }
              else {
                m_hostName = hostAndPort;
              }
            }

            // Separate the db
            m_databaseOrSchemaName = path.substr( slashPosition );
            if ( m_protocol.empty() ) {
              m_databaseOrSchemaName = m_databaseOrSchemaName.substr( 1 );
            }
          }
          else { // no db specified
            std::string::size_type otherDoubleColonPosition = path.find( ":" );
            if ( otherDoubleColonPosition != std::string::npos ) {
              m_hostName = path.substr( 0, otherDoubleColonPosition );
              if ( otherDoubleColonPosition + 1 < path.size() ) {
                std::istringstream is( path.substr( otherDoubleColonPosition + 1 ).c_str() );
                is >> m_portNumber;
              }
            }
            else {
              m_hostName = path;
            }
          }
          return;
        }
      }
    }
  }

  m_technology = m_protocol = "";
  throw coral::URIException( m_URI );
}
