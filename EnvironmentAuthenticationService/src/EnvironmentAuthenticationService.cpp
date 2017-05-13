#include "EnvironmentAuthenticationService.h"
#include "RelationalAccess/AuthenticationCredentials.h"

#include "RelationalAccess/AuthenticationServiceException.h"

#include "CoralBase/MessageStream.h"

#include <cstdlib>

coral::EnvironmentAuthenticationService::EnvironmentAuthenticationService::EnvironmentAuthenticationService( const std::string& componentName )
  : coral::Service( componentName ),
    m_credentials( 0 )
{
}

coral::EnvironmentAuthenticationService::EnvironmentAuthenticationService::~EnvironmentAuthenticationService()
{
  if ( m_credentials ) delete m_credentials;
}


const coral::IAuthenticationCredentials&
coral::EnvironmentAuthenticationService::EnvironmentAuthenticationService::credentials( const std::string& ) const
{
  if ( m_credentials == 0 ) {
    m_credentials = new coral::AuthenticationCredentials( this->name() );

    char* userName = ::getenv( "CORAL_AUTH_USER" );
    if ( userName != 0 ) {
      m_credentials->registerItem( "user", std::string( userName ) );
    }
    else {
      delete m_credentials;
      m_credentials = 0;
      coral::MessageStream log( this->name() );
      log << coral::Error << "The CORAL_AUTH_USER environment variable is not set!" << coral::MessageStream::endmsg;
      throw coral::AuthenticationServiceException( "The CORAL_AUTH_USER environment variable is not set.",
                                                   "IAuthenticationService::credentials",
                                                   this->name() );
    }
    char* password = ::getenv( "CORAL_AUTH_PASSWORD" );
    if ( password != 0 ) {
      m_credentials->registerItem( "password", std::string( password ) );
    }
    else {
      delete m_credentials;
      m_credentials = 0;
      coral::MessageStream log( this->name() );
      log << coral::Error << "The CORAL_AUTH_PASSWORD environment variable is not set!" << coral::MessageStream::endmsg;
      throw coral::AuthenticationServiceException( "The CORAL_AUTH_PASSWORD environment variable is not set.",
                                                   "IAuthenticationService::credentials",
                                                   this->name() );
    }
  }

  return *m_credentials;
}
