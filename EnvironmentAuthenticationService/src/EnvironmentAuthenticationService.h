#ifndef ENVAUTHENTITACTIONSERVICE_ENVAUTHENTITACTIONSERVICE_H
#define ENVAUTHENTITACTIONSERVICE_ENVAUTHENTITACTIONSERVICE_H

#include "RelationalAccess/IAuthenticationService.h"

#include "CoralKernel/Service.h"

namespace coral
{

  class AuthenticationCredentials;

  namespace EnvironmentAuthenticationService
  {

    /**
     * @class AuthenticationService AuthenticationService.h
     *
     * A simple implementation of the IAuthenticationService interface based on reading an two environment variables
     */
    class EnvironmentAuthenticationService : public coral::Service, virtual public coral::IAuthenticationService
    {
    public:
      /// Standard Constructor
      EnvironmentAuthenticationService( const std::string& componentName );

      /// Standard Destructor
      virtual ~EnvironmentAuthenticationService();

    public:
      /**
       * Returns a reference to the credentials object
       */
      const IAuthenticationCredentials& credentials( const std::string& ) const;
      const IAuthenticationCredentials& credentials( const std::string& connection,
                                                     const std::string& ) const
      { return this->credentials( connection ); }

    private:
      mutable coral::AuthenticationCredentials* m_credentials;
    };

  }

}

#endif
