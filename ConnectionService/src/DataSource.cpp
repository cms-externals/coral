#include "DataSource.h"
#include "ConnectionServiceConfiguration.h"
#include "CoralBase/MessageStream.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/ICertificateAuthenticationService.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/IRelationalService.h"

/// constructor
coral::ConnectionService::DataSource::DataSource( const ConnectionServiceConfiguration& serviceConfiguration ) :
  m_serviceConfiguration( serviceConfiguration ){
}


/// loads the required plugin library and retrieves the connection parametes
bool coral::ConnectionService::DataSource::setupForConnection( ConnectionParams& connectionParameters,
                                                               const std::string& role,
                                                               const coral::ICertificateData *cert ){
  bool isValid = false;
  coral::MessageStream log( m_serviceConfiguration.serviceName() );
  coral::IRelationalDomain* domain = 0;
  const std::string& connectionString = connectionParameters.connectionString();
  try {
    domain = &m_serviceConfiguration.relationalService().domainForConnection( connectionString );
  } catch (const std::runtime_error& e) {
    log << coral::Error << "Cannot find plugin for connection string \""<<connectionString<<"\": "<<e.what()<<coral::MessageStream::endmsg;
    domain = 0;
  }
  if(domain) {
    std::pair<std::string,std::string> connTokens = domain->decodeUserConnectionString( connectionString );
    bool authenticated = false;

    connectionParameters.setDbParams(connTokens.first,
                                     domain->flavorName(),
                                     connTokens.second);
    if(domain->isAuthenticationRequired( connTokens.first ) || cert!=0 ) {
      try {
        std::pair<std::string,std::string>* credentials = getCredentials( connectionString, role, cert );
        if(credentials) {
          authenticated = true;
          connectionParameters.setAuthenticationParams(credentials->first,credentials->second);
          delete credentials;
        }
      } catch (const AuthenticationServiceException& exc) {
        log << coral::Debug << "Could not retrieve credentials for connection \""<<connectionString<<"\" from authentication service: "<< exc.what() << coral::MessageStream::endmsg;
      }
    } else {
      authenticated = true;
    }
    if(authenticated) {
      isValid = true;
    } else {
      log << coral::Warning << "Replica on connection \""<<connectionString<<"\" cannot be used (no authentication provided)." << coral::MessageStream::endmsg;
    }
  }
  return isValid;
}


/// returns the credentials for the specified connection string
std::pair<std::string,std::string>*
coral::ConnectionService::DataSource::getCredentials( const std::string& connectionString,
                                                      const std::string& role,
                                                      const coral::ICertificateData *cert ) const {
  coral::IAuthenticationService& authenticationService = m_serviceConfiguration.authenticationService();
  std::pair<std::string, std::string>* credentials =  new std::pair<std::string,std::string>("","");
  //  coral::MessageStream log( m_serviceConfiguration.serviceContext(),
  //                           m_serviceConfiguration.serviceName(),
  //                           coral::Verbose );
  coral::MessageStream log( m_serviceConfiguration.serviceName() );
  try {
    if(role.empty()) {
      credentials->first = authenticationService.credentials( connectionString ).valueForItem( IAuthenticationCredentials::userItem() );
      credentials->second = authenticationService.credentials( connectionString ).valueForItem( IAuthenticationCredentials::passwordItem());

    } else {
      if ( cert != 0 )
      {
        ICertificateAuthenticationService* certAuthServ =
          dynamic_cast<ICertificateAuthenticationService* >( &authenticationService );
        if ( !certAuthServ )
          throw coral::Exception( "Underlying authentication service doesn't support certificate data!",
                                  "DataSource::getCredentials",
                                  "ConnectionService");
        credentials->first = certAuthServ->credentials( connectionString, role, cert ).valueForItem( IAuthenticationCredentials::userItem() );
        credentials->second = certAuthServ->credentials( connectionString, role, cert ).valueForItem( IAuthenticationCredentials::passwordItem() );
      }
      else
      {
        credentials->first = authenticationService.credentials( connectionString, role ).valueForItem( IAuthenticationCredentials::userItem() );
        credentials->second = authenticationService.credentials( connectionString, role ).valueForItem( IAuthenticationCredentials::passwordItem()  );
      }
    }
  } catch (const coral::AuthenticationServiceException&) {
    delete credentials;
    credentials = 0;
    throw;
  }
  return credentials;
}
