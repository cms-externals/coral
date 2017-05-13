#include "Domain.h"
#include "DomainPropertyNames.h"
#include "DomainProperties.h"
#include "Connection.h"

#include "CoralCommon/URIParser.h"

#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/IConnectionService.h"

#include "CoralBase/MessageStream.h"

#include "frontier_client/frontier-cpp.h"
#include "frontier_client/FrontierException.hpp"

#include <cstdlib>
#include <sstream>

/*
class CallbackForTableSpaceForTables
{
  public:
    explicit CallbackForTableSpaceForTables( coral::FrontierAccess::Domain& domain ):
      m_domain( domain )
    {}

    void operator() ( const seal::PropertyBase& p )
    {
      std::string tableSpace = boost::any_cast< std::string >( p.get() ); m_domain.setTableSpaceForTables( tableSpace );
    }

  private:
    coral::FrontierAccess::Domain& m_domain;
};

class CallbackForTableSpaceForIndices
{
  public:
    explicit CallbackForTableSpaceForIndices( coral::FrontierAccess::Domain& domain ):
      m_domain( domain )
    {}

    void operator() ( const seal::PropertyBase& p )
    {
      std::string tableSpace = boost::any_cast< std::string >( p.get() ); m_domain.setTableSpaceForIndices( tableSpace );
    }

  private:
    coral::FrontierAccess::Domain& m_domain;
};

class CallbackForTableSpaceForLobs
{
  public:
    explicit CallbackForTableSpaceForLobs( coral::FrontierAccess::Domain& domain ):
      m_domain( domain )
    {}

    void operator() ( const seal::PropertyBase& p )
    {
      std::string tableSpace = boost::any_cast< std::string >( p.get() ); m_domain.setTableSpaceForLobs( tableSpace );
    }

  private:
    coral::FrontierAccess::Domain& m_domain;
};

class CallbackForLobChunkSize
{
  public:
    explicit CallbackForLobChunkSize( coral::FrontierAccess::Domain& domain ):
      m_domain( domain )
    {}

    void operator() ( const seal::PropertyBase& p )
    {
      int lobChunkSize = boost::any_cast< int >( p.get() ); m_domain.setLobChunkSize( lobChunkSize );
    }

  private:
    coral::FrontierAccess::Domain& m_domain;
};

*/

coral::FrontierAccess::Domain::Domain( const std::string& componentName )
  : coral::Service( componentName )
  , m_flavorName( "frontier" )
  , m_implementationName( "frontier_client" )
  , m_implementationVersion( "2.4.1" )
  , m_properties( new coral::FrontierAccess::DomainProperties( this ) )
{
  this->setProperties();
}

void coral::FrontierAccess::Domain::setProperties()
{
  // Table space for tables
  std::string defaultTableSpaceForTables = "";
  std::string tableSpaceForTables = "";
  char* cdefaultTableSpaceForTables = ::getenv( "CORAL_ORA_TS_TAB" );
  if ( cdefaultTableSpaceForTables != 0 ) defaultTableSpaceForTables = std::string( cdefaultTableSpaceForTables );
  tableSpaceForTables = defaultTableSpaceForTables;

  // Table space for indices
  std::string defaultTableSpaceForIndices = "";
  std::string tableSpaceForIndices = "";
  char* cdefaultTableSpaceForIndices = ::getenv( "CORAL_ORA_TS_IND" );
  if ( cdefaultTableSpaceForIndices != 0 ) defaultTableSpaceForIndices = std::string( cdefaultTableSpaceForIndices );
  tableSpaceForIndices = defaultTableSpaceForIndices;

  // Table space for lobs
  std::string defaultTableSpaceForLobs = "";
  std::string tableSpaceForLobs = "";
  char* cdefaultTableSpaceForLobs = ::getenv( "CORAL_ORA_TS_LOB" );
  if ( cdefaultTableSpaceForLobs != 0 ) defaultTableSpaceForLobs = std::string( cdefaultTableSpaceForLobs );
  tableSpaceForLobs = defaultTableSpaceForLobs;

  // Lob chunk size
  int defaultLobChunkSize = 1024;
  int lobChunkSize = 0;
  char* cdefaultLobChunkSize = ::getenv( "CORAL_ORA_LOB_CHUNK_SIZE" );
  if ( cdefaultLobChunkSize != 0 ) {
    std::istringstream is( cdefaultLobChunkSize );
    is >> defaultLobChunkSize;
  }
  lobChunkSize = defaultLobChunkSize;

  m_properties->setTableSpaceForTables( tableSpaceForTables );
  m_properties->setTableSpaceForIndices( tableSpaceForIndices );
  m_properties->setTableSpaceForLobs( tableSpaceForLobs );
  m_properties->setLobChunkSize( lobChunkSize );
}

coral::FrontierAccess::Domain::~Domain()
{
  delete m_properties;
}

std::string coral::FrontierAccess::Domain::flavorName() const
{
  return m_flavorName;
}

std::string coral::FrontierAccess::Domain::implementationName() const
{
  return m_implementationName;
}

std::string coral::FrontierAccess::Domain::implementationVersion() const
{
  return m_implementationVersion;
}

coral::IConnection*
coral::FrontierAccess::Domain::newConnection( const std::string& uriString ) const
{
  try
  {
    // Mandatory Frontier client API initialization
    frontier::init();
  }
  catch( const frontier::FrontierException& fe )
  {
    coral::MessageStream log( this->name() );
    log << fe.what() << coral::MessageStream::endmsg;
    throw coral::SessionException( this->name(), fe.what(), "coral::FrontierAccess::Domain::newConnection" );
  }

  return( new coral::FrontierAccess::Connection( *m_properties, uriString ) );
}

std::pair< std::string, std::string >
coral::FrontierAccess::Domain::decodeUserConnectionString( const std::string& ucs ) const
{
  // Extract schema name
  std::string::size_type spos = ucs.rfind( '/' );
  std::string schemaName = ucs.substr( spos + 1 );

  // Extract frontier connection string
  std::string::size_type cspos = ucs.find( ':' );
  cspos += 3; // skip "//"
  std::string connectionString = ucs.substr( cspos, spos-cspos );

  // Make sure we support backwards compatibility by checking for the new Frontier connection string syntax
  // e.g. (serverurl=host.domain:port ...)(proxyurl=host.domain:port ...)(comporession level)
  if( connectionString.find( '(' ) == std::string::npos )
    connectionString = "http://" + connectionString;

  return std::make_pair( connectionString, schemaName );
}

void coral::FrontierAccess::Domain::setTableSpaceForTables( const std::string& tableSpace )
{
  m_properties->setTableSpaceForTables( tableSpace );
}

void coral::FrontierAccess::Domain::setTableSpaceForIndices( const std::string& tableSpace )
{
  m_properties->setTableSpaceForIndices( tableSpace );
}

void coral::FrontierAccess::Domain::setTableSpaceForLobs( const std::string& tableSpace )
{
  m_properties->setTableSpaceForLobs( tableSpace );
}

void coral::FrontierAccess::Domain::setLobChunkSize( int lobChunkSize )
{
  m_properties->setLobChunkSize( lobChunkSize );
}
