#include "CoralKernel/Service.h"
#include "RelationalAccess/SessionException.h"

#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "TypeConverter.h"

coral::SQLiteAccess::ConnectionProperties::ConnectionProperties( const DomainProperties& domainProperties,
                                                                 //Connection& connection,
                                                                 const std::string& connectionString ) :
  m_domainProperties( domainProperties ),
  m_domainServiceName( domainProperties.service()->name() ),
  m_connectionString( connectionString ),
  m_typeConverter( new coral::SQLiteAccess::TypeConverter( m_domainProperties ) ),
  m_isReadOnly( false ),
  //m_connection( &connection ),
  m_mutex( domainProperties.mutex() )
{
}


/*
coral::SQLiteAccess::Connection&
coral::SQLiteAccess::ConnectionProperties::connection() const
{
  if ( !m_connection )
    throw coral::SessionException( "Connection is no longer valid",
                                   "ConnectionProperties::connection",
                                   domainServiceName() );
  return *m_connection;
}
*/


coral::SQLiteAccess::ConnectionProperties::~ConnectionProperties()
{
  delete m_typeConverter;
}


coral::ITypeConverter&
coral::SQLiteAccess::ConnectionProperties::typeConverter()
{
  return *m_typeConverter;
}


const coral::ITypeConverter&
coral::SQLiteAccess::ConnectionProperties::typeConverter() const
{
  return *m_typeConverter;
}
