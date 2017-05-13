//#ifdef WIN32
//// Keep the 'class sqlite3' forward declaration even if it is a struct
//#pragma warning ( disable : 4099 )
//#endif

#include "sqlite3.h"

#include "CoralKernel/Service.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/SessionException.h"

#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "Session.h"
#include "SessionProperties.h"
#include "TypeConverter.h"

coral::SQLiteAccess::SessionProperties::SessionProperties( boost::shared_ptr<ConnectionProperties> connectionProperties,
                                                           coral::SQLiteAccess::Session& session,
                                                           bool readOnly,
                                                           boost::mutex* mutex )
  : m_connectionProperties( connectionProperties )
  , m_session( &session )
  , m_dbHandle(0)
  , m_monitoringService( 0 )
  , m_isReadOnly( readOnly )
  , m_mutex( mutex )
{
}


coral::SQLiteAccess::SessionProperties::~SessionProperties()
{
}


bool
coral::SQLiteAccess::SessionProperties::isTransactionActive() const
{
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::isTransactionActive",
                                   domainProperties().service()->name() );
  return m_session->transaction().isActive();
}


bool
coral::SQLiteAccess::SessionProperties::isTransactionReadOnly() const
{
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::isTransactionReadOnly",
                                   domainProperties().service()->name() );
  return m_session->transaction().isReadOnly();
}


boost::shared_ptr<coral::SQLiteAccess::ConnectionProperties>
coral::SQLiteAccess::SessionProperties::connectionProperties()
{
  return m_connectionProperties;
}


const coral::SQLiteAccess::DomainProperties&
coral::SQLiteAccess::SessionProperties::domainProperties() const
{
  return m_connectionProperties->domainProperties();
}


std::string
coral::SQLiteAccess::SessionProperties::connectionString() const
{
  return m_connectionString;
}


coral::ITypeConverter&
coral::SQLiteAccess::SessionProperties::typeConverter()
{
  return m_connectionProperties->typeConverter();
}


const coral::ITypeConverter&
coral::SQLiteAccess::SessionProperties::typeConverter() const
{
  return m_connectionProperties->typeConverter();
}


void
coral::SQLiteAccess::SessionProperties::setDbHandle( sqlite3* dbHandle )
{
  m_dbHandle = dbHandle;
}


sqlite3*
coral::SQLiteAccess::SessionProperties::dbHandle() const
{
  return m_dbHandle;
}


/*
int
coral::SQLiteAccess::SessionProperties::serverVersion() const
{
  return m_serverVersion;
}
*/


void
coral::SQLiteAccess::SessionProperties::setMonitoringService( coral::monitor::IMonitoringService* monitoringService )
{
  m_monitoringService = monitoringService;
}


coral::monitor::IMonitoringService*
coral::SQLiteAccess::SessionProperties::monitoringService() const
{
  return m_monitoringService;
}


bool
coral::SQLiteAccess::SessionProperties::isReadOnly() const
{
  return m_isReadOnly;
}


std::string
coral::SQLiteAccess::SessionProperties::schemaName() const
{
  return "";
}


coral::ISchema&
coral::SQLiteAccess::SessionProperties::schema() const
{
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::schema",
                                   domainProperties().service()->name() );
  return m_session->nominalSchema();
}
