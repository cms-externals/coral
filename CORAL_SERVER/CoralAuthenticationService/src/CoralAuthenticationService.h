#ifndef CORALAUTHENTICATIONSERVICE_H
#define CORALAUTHENTICATIONSERVICE_H

#include "CoralKernel/Service.h"
#include "RelationalAccess/AuthenticationCredentials.h"
#include "CoralCommon/DatabaseServiceSet.h"
#include "RelationalAccess/../src/ICertificateAuthenticationService.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"


namespace coral {

  //forward declaration
  class ICertificateData;

  class CoralAuthenticationService : public coral::Service,
                                     virtual public coral::ILookupService,
                                     virtual public coral::ICertificateAuthenticationService
  {
  public:

    /// Standard Constructor with a component key
    explicit CoralAuthenticationService( const std::string& componentName);

    /// Standard Destructor
    virtual ~CoralAuthenticationService();

  public:

    // IAuthenticationService interface

    /**
     * Returns a reference to the credentials object for a given connnection string.
     * If the connection string is not known to the service an UnknownConnectionException is thrown.
     */
    const IAuthenticationCredentials& credentials(
                                                  const std::string& connectionString ) const;

    /**
     * Returns a reference to the credentials object for a given connection string and a "role".
     * If the connection string is not known to the service an UnknownConnectionException is thrown.
     * If the role specified does not exist an UnknownRoleException is thrown
     */
    const IAuthenticationCredentials& credentials(
                                                  const std::string& connectionString,
                                                  const std::string& role ) const
    {
      return credentials( connectionString, role, (ICertificateData *) 0 );
    };

    const IAuthenticationCredentials& credentials(
                                                  const std::string& connectionString,
                                                  const std::string& role,
                                                  const ICertificateData* cert ) const;

    // ILookupService interface
    /**
     * Performs a lookup for a given logical connection string.
     *
     * The method creates and returns a new object, the caller of the method is
     * responsible for managing the lifetime of it.
     */
    virtual IDatabaseServiceSet* lookup( const std::string& logicalName,
                                         AccessMode accessMode = Update,
                                         std::string authenticationMechanism = "" ) const;

    /**
     * Sets the input file name
     * FIXME currently does nothing..
     */
    virtual void setInputFileName(  const std::string& inputFileName );

  private:
    // local cache for the credentials, needed for the interface returning references...
    // the cache has to be modified by the const credentials() method...
    mutable std::map< std::string, boost::shared_ptr< AuthenticationCredentials > > m_store;

  };

}

#endif
