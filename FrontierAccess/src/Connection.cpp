// $Id: Connection.cpp,v 1.12 2011/03/22 10:36:50 avalassi Exp $
#include <locale>
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/MessageStream.h"
#include "CoralCommon/URIParser.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/IWebCacheControl.h"
#include "RelationalAccess/IWebCacheInfo.h"
#include "RelationalAccess/AccessMode.h"
#include "frontier_client/frontier-cpp.h"
#include "frontier_client/FrontierException.hpp"

#include "Connection.h"
#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "Session.h"
#include "TypeConverter.h"
boost::mutex coral::FrontierAccess::Connection::s_lock{};

coral::FrontierAccess::Connection::Connection( const coral::FrontierAccess::DomainProperties& domainProperties, const std::string& connectionString )
  : m_connection(0)
  , m_domainProperties( domainProperties )
  , m_connectionString( connectionString )
  , m_connected( false )
  , m_serverVersion( "" )
  , m_typeConverter( new coral::FrontierAccess::TypeConverter( m_domainProperties ) )
{
  if( this->m_typeConverter )
    this->m_typeConverter->reset( 10 );
}

coral::FrontierAccess::Connection::~Connection()
{
  this->disconnect();
  delete this->m_typeConverter;
}

void coral::FrontierAccess::Connection::connect()
{
  coral::MessageStream log( m_domainProperties.service()->name() );

  // Locate the web cache control and set up the compression level
  try
  {
    coral::IWebCacheControl& cachectrl = const_cast<coral::IWebCacheControl&>( m_domainProperties.cacheControl() );
    const char* zipLevelStr = 0;
    zipLevelStr = ::getenv( "CORAL_WEBCACHE_ZIP_LEVEL" );
    if( zipLevelStr != 0 )
    {
      int level = 5;
      level = ::atoi( zipLevelStr );
      log << coral::Warning << "Changing the compression level from " << cachectrl.compressionLevel() << " to level " << level << coral::MessageStream::endmsg;
      cachectrl.setCompressionLevel( level );
    }
    else
    {
      log << coral::Info << "The current compresion level is " << cachectrl.compressionLevel() << coral::MessageStream::endmsg;
    }
  }
  catch( const std::exception& )
  {
    log << coral::Warning
        << "The web cache control cannot be located, check if the coral::ConnectionService is loaded or web cache control set up, running with the default compression level"
        << coral::MessageStream::endmsg;
  }

  std::string furi = "frontier://" + m_connectionString;

  // Processing the connection string
  coral::URIParser parser; parser.setURI( furi );

  std::ostringstream osDatabase;
  osDatabase << parser.hostName();
  int portNumber = parser.portNumber();
  if ( portNumber != 0 ) osDatabase << ":" << portNumber;

  std::string constr = furi;
  //   size_t      pos    = constr.rfind( "/" );

  //   if( pos == std::string::npos )
  //     throw coral::DatabaseNotAccessibleException( m_domainProperties.service()->name(),
  //                                                  "Invalid connection string: " + constr + ", can't extract schema name", "coral::FrontierAccess::Connection::connect" );

  //   std::string schema = constr.substr( pos+1 );

  //   if( schema.empty() )
  //     throw coral::DatabaseNotAccessibleException( m_domainProperties.service()->name(),
  //                                                  "Invalid connection string: " + constr + ", no schema name specified", "coral::FrontierAccess::Connection::connect" );

  //   std::string conurl = "http" + constr.substr( 8, constr.size() - schema.size() - 8 - 1 );

  //   if( conurl.empty() )
  //     throw coral::DatabaseNotAccessibleException( m_domainProperties.service()->name(),
  //                                                  "Invalid connection string: " + constr + ", can't translate into correct Frontier URL", "coral::FrontierAccess::Connection::connect" );

  //   log << coral::Verbose << "Connecting to Frontier server using URL: " << conurl << coral::MessageStream::endmsg;

  log << coral::Verbose << "Connecting to Frontier server using URL: " << this->m_connectionString << coral::MessageStream::endmsg;

  {
    boost::mutex::scoped_lock lock( s_lock );

    // Attaching the server
    m_connection = new frontier::Connection( m_connectionString );

    // With Frontier does not matter
    m_serverVersion="10.2.0.2.0";

    this->m_connected = true;
  }
}

coral::ISession*
coral::FrontierAccess::Connection::newSession( const std::string& schemaName,
                                               coral::AccessMode mode ) const
{
  if ( mode != coral::ReadOnly )
    throw coral::SessionException( "Update mode is not supported in FrontierAccess", "coral::FrontierAccess::Connection::newSession", m_domainProperties.service()->name() );
  coral::FrontierAccess::Session* session =
    new coral::FrontierAccess::Session( const_cast<coral::FrontierAccess::Connection&>(*this),
                                        m_domainProperties,
                                        m_connectionString,
                                        *m_connection,
                                        s_lock,
                                        schemaName,
                                        *m_typeConverter );
  return session;
}

bool coral::FrontierAccess::Connection::isConnected( bool probePhysicalConnection )
{
  if( probePhysicalConnection )
  {
    coral::MessageStream log( "coral::FrontierAccess::Connection::isConnected" );

    try
    {
      this->m_connected = this->ping();
    }
    catch( const frontier::FrontierException& fe )
    {
      m_connected = false;
      const char* errmsg = fe.what();
      if( errmsg != 0 )
        log << coral::Error << fe.what() << coral::MessageStream::endmsg;
      else
        log << coral::Error << "coral::FrontierAccess::Connection: Error checking connection" << coral::MessageStream::endmsg;
    }
  }

  return m_connected;
}

void coral::FrontierAccess::Connection::disconnect()
{
  boost::mutex::scoped_lock lock( s_lock );

  if( ! m_connected )
    return;

  if( m_connection )
  {
    delete m_connection;
    m_connection = 0;
  }

  m_connected = false;
}

std::string coral::FrontierAccess::Connection::serverVersion() const
{
  if ( ! const_cast<coral::FrontierAccess::Connection*>( this )->isConnected() )
    throw coral::ConnectionNotActiveException( m_domainProperties.service()->name(), "coral::FrontierAccess::Connection::serverVersion" );
  return m_serverVersion;
}

coral::ITypeConverter& coral::FrontierAccess::Connection::typeConverter()
{
  if ( ! this->isConnected() )
    throw coral::ConnectionNotActiveException( m_domainProperties.service()->name(), "coral::FrontierAccess::Connection::typeConverter" );
  return *(this->m_typeConverter);
}

bool coral::FrontierAccess::Connection::ping()
{
  bool available = true;

  //   // Use a statement to find the Oracle server version behind the Frontier app. server
  //   coral::AttributeList inBuffer;
  //   coral::FrontierAccess::Statement query( *m_properties, "SELECT 1 FROM DUAL" ); // FIXME !!!!
  //   this->m_connected = query.execute( inBuffer );

  return available;
}
