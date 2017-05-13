#include <cstdlib>
//#include <iostream>
#include <locale>
#include <sstream>
#include "oci.h"

#include "CoralCommon/URIParser.h"

#include "Connection.h"
#include "Domain.h"
#include "DomainProperties.h"
#include "DomainPropertyNames.h"

// class CallbackForTableSpaceForTables {
// public:
//   explicit CallbackForTableSpaceForTables( coral::OracleAccess::Domain& domain ):
//     m_domain( domain )
//   {}
//
//   void operator() ( const seal::PropertyBase& p ) {
//     std::string tableSpace = boost::any_cast< std::string >( p.get() );
//     m_domain.setTableSpaceForTables( tableSpace );
//   }
//
// private:
//   coral::OracleAccess::Domain& m_domain;
// };
//
//
// class CallbackForTableSpaceForIndices {
// public:
//   explicit CallbackForTableSpaceForIndices( coral::OracleAccess::Domain& domain ):
//     m_domain( domain )
//   {}
//
//   void operator() ( const seal::PropertyBase& p ) {
//     std::string tableSpace = boost::any_cast< std::string >( p.get() );
//     m_domain.setTableSpaceForIndices( tableSpace );
//   }
//
// private:
//   coral::OracleAccess::Domain& m_domain;
// };
//
//
// class CallbackForTableSpaceForLobs {
// public:
//   explicit CallbackForTableSpaceForLobs( coral::OracleAccess::Domain& domain ):
//     m_domain( domain )
//   {}
//
//   void operator() ( const seal::PropertyBase& p ) {
//     std::string tableSpace = boost::any_cast< std::string >( p.get() );
//     m_domain.setTableSpaceForLobs( tableSpace );
//   }
//
// private:
//   coral::OracleAccess::Domain& m_domain;
// };
//
//
// class CallbackForLobChunkSize {
// public:
//   explicit CallbackForLobChunkSize( coral::OracleAccess::Domain& domain ):
//     m_domain( domain )
//   {}
//
//   void operator() ( const seal::PropertyBase& p ) {
//     int lobChunkSize = boost::any_cast< int >( p.get() );
//     m_domain.setLobChunkSize( lobChunkSize );
//   }
//
// private:
//   coral::OracleAccess::Domain& m_domain;
// };

coral::OracleAccess::Domain::Domain( const std::string& componentName )
  : coral::Service( componentName )
  , m_flavorName( "Oracle" )
  , m_implementationName( "OCI" )
{
  //std::cout << "Create OracleAccess::Domain " << this << std::endl; // debug bug #73334
  std::stringstream version;
  // Compile-time version (e.g. 11.2).
  // See http://forums.oracle.com/forums/thread.jspa?threadID=356841
  // Run-time version (e.g. 11.2.0.1.0) is available via OCIClientVersion
  // See http://forums.oracle.com/forums/thread.jspa?threadID=1089440&tstart=15
  version << OCI_MAJOR_VERSION << "." << OCI_MINOR_VERSION;
  m_implementationVersion = version.str();
  m_properties = new coral::OracleAccess::DomainProperties( this );
  this->setProperties();
}


void
coral::OracleAccess::Domain::setProperties()
{
  // Table space for tables
  std::string defaultTableSpaceForTables = "";
  std::string tableSpaceForTables = "";
  char* cdefaultTableSpaceForTables = ::getenv( "CORAL_ORA_TS_TAB" );
  if ( cdefaultTableSpaceForTables != 0 ) defaultTableSpaceForTables = std::string( cdefaultTableSpaceForTables );
  tableSpaceForTables = defaultTableSpaceForTables;
  //   propertyManager()->declareProperty( coral::OracleAccess::DomainPropertyNames::tableSpaceForTables(),
  //                                       tableSpaceForTables,
  //                                       defaultTableSpaceForTables,
  //                                       "The name of the table space where new tables are created.",
  //                                       CallbackForTableSpaceForTables( *this ) );

  // Table space for indices
  std::string defaultTableSpaceForIndices = "";
  std::string tableSpaceForIndices = "";
  char* cdefaultTableSpaceForIndices = ::getenv( "CORAL_ORA_TS_IND" );
  if ( cdefaultTableSpaceForIndices != 0 ) defaultTableSpaceForIndices = std::string( cdefaultTableSpaceForIndices );
  tableSpaceForIndices = defaultTableSpaceForIndices;
  //   propertyManager()->declareProperty( coral::OracleAccess::DomainPropertyNames::tableSpaceForIndices(),
  //                                       tableSpaceForIndices,
  //                                       defaultTableSpaceForIndices,
  //                                       "The name of the table space where new indices are created.",
  //                                       CallbackForTableSpaceForIndices( *this ) );

  // Table space for lobs
  std::string defaultTableSpaceForLobs = "";
  std::string tableSpaceForLobs = "";
  char* cdefaultTableSpaceForLobs = ::getenv( "CORAL_ORA_TS_LOB" );
  if ( cdefaultTableSpaceForLobs != 0 ) defaultTableSpaceForLobs = std::string( cdefaultTableSpaceForLobs );
  tableSpaceForLobs = defaultTableSpaceForLobs;
  //   propertyManager()->declareProperty( coral::OracleAccess::DomainPropertyNames::tableSpaceForLobs(),
  //                                       tableSpaceForLobs,
  //                                       defaultTableSpaceForLobs,
  //                                       "The name of the table space where new indices are created.",
  //                                       CallbackForTableSpaceForLobs( *this ) );

  // Lob chunk size
  int defaultLobChunkSize = 1024;
  int lobChunkSize = 0;
  char* cdefaultLobChunkSize = ::getenv( "CORAL_ORA_LOB_CHUNK_SIZE" );
  if ( cdefaultLobChunkSize != 0 ) {
    std::istringstream is( cdefaultLobChunkSize );
    is >> defaultLobChunkSize;
  }
  lobChunkSize = defaultLobChunkSize;
  //   propertyManager()->declareProperty( coral::OracleAccess::DomainPropertyNames::lobChunkSize(),
  //                                       lobChunkSize,
  //                                       defaultLobChunkSize,
  //                                       "The size of lob chunks.",
  //                                       CallbackForLobChunkSize( *this ) );

  m_properties->setTableSpaceForTables( tableSpaceForTables );
  m_properties->setTableSpaceForIndices( tableSpaceForIndices );
  m_properties->setTableSpaceForLobs( tableSpaceForLobs );
  m_properties->setLobChunkSize( lobChunkSize );
}


coral::OracleAccess::Domain::~Domain()
{
  //std::cout << "Delete OracleAccess::Domain " << this << std::endl; // debug bug #73334
  delete m_properties;
}


std::string
coral::OracleAccess::Domain::flavorName() const
{
  return m_flavorName;
}


std::string
coral::OracleAccess::Domain::implementationName() const
{
  return m_implementationName;
}


std::string
coral::OracleAccess::Domain::implementationVersion() const
{
  return m_implementationVersion;
}


coral::IConnection*
coral::OracleAccess::Domain::newConnection( const std::string& uriString ) const
{
  return new coral::OracleAccess::Connection( *m_properties, uriString );
}


void
coral::OracleAccess::Domain::setTableSpaceForTables( const std::string& tableSpace )
{
  m_properties->setTableSpaceForTables( tableSpace );
}


void
coral::OracleAccess::Domain::setTableSpaceForIndices( const std::string& tableSpace )
{
  m_properties->setTableSpaceForIndices( tableSpace );
}


void
coral::OracleAccess::Domain::setTableSpaceForLobs( const std::string& tableSpace )
{
  m_properties->setTableSpaceForLobs( tableSpace );
}


void
coral::OracleAccess::Domain::setLobChunkSize( int lobChunkSize )
{
  m_properties->setLobChunkSize( lobChunkSize );
}


std::pair<std::string, std::string >
coral::OracleAccess::Domain::decodeUserConnectionString( const std::string& userConnectionString ) const
{
  // Processing the connection string
  coral::URIParser parser;
  parser.setURI( userConnectionString );
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

  std::string schemaName = "";
  for ( std::string::size_type i = 0; i < tempSchema.size(); ++i ) {
    schemaName += std::toupper( tempSchema[i], std::locale::classic() );
  }

  return std::make_pair( std::string( osDatabase.str() ), schemaName );
}
