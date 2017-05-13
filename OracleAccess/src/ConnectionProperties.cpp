//#include <iostream>
#include "oci.h"

#include "CoralKernel/Service.h"
#include "RelationalAccess/SessionException.h"

#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "TypeConverter.h"

coral::OracleAccess::ConnectionProperties::ConnectionProperties( const DomainProperties& domainProperties,
                                                                 const std::string& connectionString,
                                                                 Connection& connection ) :
  m_domainProperties( domainProperties ),
  m_domainServiceName( domainProperties.service()->name() ),
  m_connectionString( connectionString ),
  m_typeConverter( new coral::OracleAccess::TypeConverter( m_domainProperties ) ),
  m_ociEnvHandle( 0 ),
  m_ociErrorHandle( 0 ),
  m_ociServerHandle( 0 ),
  m_serverVersion( 0 ),
  m_connection( &connection ),
  //m_connectionMutex(),
  m_mutex()
{
  //std::cout << "Create ConnectionProperties " << this << std::endl; // debug bug #73334
}



coral::OracleAccess::ConnectionProperties::~ConnectionProperties()
{
  //std::cout << "Delete ConnectionProperties " << this << std::endl; // debug bug #73334
  delete m_typeConverter;
}


coral::OracleAccess::Connection&
coral::OracleAccess::ConnectionProperties::connection() const
{
  if ( !m_connection )
    throw coral::SessionException( "Connection is no longer valid",
                                   "ConnectionProperties::connection",
                                   domainServiceName() );
  return *m_connection;
}


void
coral::OracleAccess::ConnectionProperties::setHandles( OCIEnv* ociEnvHandle,
                                                       OCIError* ociErrorHandle,
                                                       OCIServer* ociServerHandle,
                                                       int majorServerVersion )
{
  boost::mutex::scoped_lock lock( m_mutex );
  m_ociEnvHandle = ociEnvHandle;
  m_ociErrorHandle = ociErrorHandle;
  m_ociServerHandle = ociServerHandle;
  m_typeConverter->reset( majorServerVersion );
  m_serverVersion = majorServerVersion;
}


coral::ITypeConverter&
coral::OracleAccess::ConnectionProperties::typeConverter()
{
  boost::mutex::scoped_lock lock( m_mutex );
  return *m_typeConverter;
}


const coral::ITypeConverter&
coral::OracleAccess::ConnectionProperties::typeConverter() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return *m_typeConverter;
}
