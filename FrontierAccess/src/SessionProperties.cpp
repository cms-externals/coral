// $Id: SessionProperties.cpp,v 1.9 2011/03/22 10:36:50 avalassi Exp $

#include "DomainProperties.h"
#include "SessionProperties.h"
#include "TypeConverter.h"

#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"

#include "CoralBase/Exception.h"

#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"

#include <stdexcept>
#include <locale>

coral::FrontierAccess::SessionProperties::SessionProperties( const DomainProperties& domainProperties,
                                                             const std::string& connectionString,
                                                             frontier::Connection& fconnection,
                                                             boost::mutex& flock,
                                                             const std::string& schemaName,
                                                             coral::ITypeConverter& converter,
                                                             coral::ISession& session )
  : m_domainProperties( domainProperties )
  , m_connectionString( connectionString )
  , m_connectionURL( connectionString )
  , m_connection( fconnection )
  , m_session( session )
  , m_typeConverter( converter )
  , m_monitoringService( 0 )
  , m_serverVersion( 0 )
  , m_schemaName( "" )
  , m_lock( flock )
{
  if( ! schemaName.empty() )
    setSchemaName( schemaName );
}

coral::FrontierAccess::SessionProperties::~SessionProperties()
{
}

void coral::FrontierAccess::SessionProperties::setServerVersion( int majorServerVersion )
{
  m_serverVersion = majorServerVersion;
}

coral::ITypeConverter& coral::FrontierAccess::SessionProperties::typeConverter()
{
  return m_typeConverter;
}

const coral::ITypeConverter& coral::FrontierAccess::SessionProperties::typeConverter() const
{
  return m_typeConverter;
}

void coral::FrontierAccess::SessionProperties::setMonitoringService( coral::monitor::IMonitoringService* monitoringService )
{
  m_monitoringService = monitoringService;
}

bool coral::FrontierAccess::SessionProperties::isTransactionActive() const
{
  return m_session.transaction().isActive();
}

coral::ISchema& coral::FrontierAccess::SessionProperties::schema() const
{
  return m_session.nominalSchema();
}

std::string coral::FrontierAccess::SessionProperties::schemaName() const
{
  return m_schemaName;
}

void coral::FrontierAccess::SessionProperties::setSchemaName( const std::string& newSchemaName )
{
  m_schemaName           = "";
  std::string tempSchema = newSchemaName;
  for ( std::string::size_type i = 0; i < tempSchema.size(); ++i )
  {
    m_schemaName += std::toupper( tempSchema[i], std::locale::classic() );
  }
}

coral::IHandle<coral::IConnectionService> coral::FrontierAccess::SessionProperties::connectionService() const
{
  coral::IHandle<coral::IConnectionService> cv = coral::Context::instance().query<coral::IConnectionService>();;

  if( ! cv.isValid() )
    throw coral::Exception( "", "Did not find coral::ConnectionService", "" );
  //return coral::IHandle<coral::IConnectionService>( coral::IHandle<coral::Service> );

  return cv;
}
