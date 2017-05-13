// $Id: SessionProperties.cpp,v 1.21 2011/03/21 21:29:17 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralKernel/Service.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "SessionProperties.h"
#include "TypeConverter.h"

coral::MySQLAccess::SessionProperties::SessionProperties( const DomainProperties& domainProperties,
                                                          const std::string& connectionString, const std::string& schemaName,
                                                          MYSQL*& mysql, bool& connected, boost::mutex& conlock,
                                                          coral::ISession& session,
                                                          coral::AccessMode mode )
  : m_domainProperties( domainProperties )
  , m_connectionString( connectionString )
  , m_session( session )
  , m_accessMode( mode )
  , m_connectionHandle( mysql )
  , m_isConnected( connected )
  , m_userSessionStarted( false )
  , m_monitoringService( 0 )
  , m_typeConverter( domainProperties.typeConverter() )
  , m_schemaName( schemaName )
  , m_serverVersion()
  , m_serverRevision()
  , m_lock( conlock )
{
}

coral::MySQLAccess::SessionProperties::~SessionProperties()
{
}

bool coral::MySQLAccess::SessionProperties::isTransactionActive() const
{
  return m_session.transaction().isActive();
}

bool coral::MySQLAccess::SessionProperties::isTransactionReadOnly() const
{
  return m_session.transaction().isReadOnly();
}

coral::ITypeConverter& coral::MySQLAccess::SessionProperties::typeConverter()
{
  return m_typeConverter;
}

const coral::ITypeConverter& coral::MySQLAccess::SessionProperties::typeConverter() const
{
  return m_typeConverter;
}

coral::ISchema& coral::MySQLAccess::SessionProperties::schema() const
{
  return m_session.nominalSchema();
}

std::string coral::MySQLAccess::SessionProperties::schemaName() const
{
  return m_schemaName;
}

std::string coral::MySQLAccess::SessionProperties::serverVersion() const
{
  return m_serverVersion;
}

void coral::MySQLAccess::SessionProperties::setServerVersion( const std::string& serverVersion )
{
  m_serverVersion = serverVersion;
}

coral::MySQLAccess::ServerRevision coral::MySQLAccess::SessionProperties::serverRevision() const
{
  return m_serverRevision;
}

void coral::MySQLAccess::SessionProperties::setServerRevision( unsigned long major, unsigned long minor, unsigned long revision )
{
  m_serverRevision.major    = major;
  m_serverRevision.minor    = minor;
  m_serverRevision.revision = revision;
}

bool
coral::MySQLAccess::SessionProperties::isReadOnly() const
{
  return m_accessMode == coral::ReadOnly;
}
