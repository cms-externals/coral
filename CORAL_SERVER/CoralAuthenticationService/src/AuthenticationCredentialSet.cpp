#include "AuthenticationCredentialSet.h"
#include "RelationalAccess/AuthenticationCredentials.h"
#include "RelationalAccess/AuthenticationServiceException.h"

coral::AuthenticationCredentialSet::AuthenticationCredentialSet( const std::string& serviceName,
                                                                 const std::string& connectionName ) :
  m_serviceName( serviceName ),
  m_connectionName( connectionName ),
  m_default( new coral::AuthenticationCredentials( serviceName ) ),
  m_data()
{
}


coral::AuthenticationCredentialSet::~AuthenticationCredentialSet()
{
  delete m_default;
  for ( std::map< std::string, coral::AuthenticationCredentials* >::iterator iData = m_data.begin();
        iData != m_data.end(); ++iData )
    delete iData->second;
}


void
coral::AuthenticationCredentialSet::appendCredentialItem( const std::string& item,
                                                          const std::string& value )
{
  m_default->registerItem( item, value );
}


void
coral::AuthenticationCredentialSet::appendCredentialItemForRole( const std::string& item,
                                                                 const std::string& value,
                                                                 const std::string& role )
{
  std::map< std::string, coral::AuthenticationCredentials* >::iterator iRole = m_data.find( role );
  if ( iRole == m_data.end() ) {
    iRole = m_data.insert( std::make_pair( role, new coral::AuthenticationCredentials( m_serviceName ) ) ).first;
  }
  iRole->second->registerItem( item, value );
}


const coral::IAuthenticationCredentials&
coral::AuthenticationCredentialSet::credentials() const
{
  if(!m_default) {
    throw coral::AuthenticationServiceException( "Default credential not found.",
                                                 "coral::AuthenticationCredentialSet::credentials",
                                                 "LFCReplicaService" );
  }
  return *m_default;
}


const coral::IAuthenticationCredentials&
coral::AuthenticationCredentialSet::credentials( const std::string& role ) const
{
  std::map< std::string, coral::AuthenticationCredentials* >::const_iterator iRole = m_data.find( role );
  if ( iRole == m_data.end() )
    throw coral::UnknownRoleException( m_serviceName,
                                       m_connectionName,
                                       role );
  return *( iRole->second );
}
