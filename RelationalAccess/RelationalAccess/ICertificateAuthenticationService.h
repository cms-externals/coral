#ifndef RELATIONALACCESS_ICERTIFICATEAUTHENTICATIONSERVICE_H
#define RELATIONALACCESS_ICERTIFICATEAUTHENTICATIONSERVICE_H

#include "RelationalAccess/IAuthenticationService.h"

namespace coral
{

  // Forward declaration
  class ICertificateData;

  /**
   * Class ICertificateAuthenticationService
   *
   * Extended IAuthenticationService supporting certificate authentication.
   * The only difference versus IAuthenticationService is the credentials
   * method with an ICertificateData argument: eventually this will be added
   * to the default IAuthenticationService, but this is postponed as it would
   * require a binary-incompatible public API change versus CORAL_2_3_1.
   */

  class ICertificateAuthenticationService : virtual public IAuthenticationService
  {

  public:

    /**
     * Returns a reference to the credentials object for a given connection string.
     * If the connection string is not known to the service an UnknownConnectionException is thrown.
     */
    virtual const IAuthenticationCredentials& credentials( const std::string& connectionString ) const = 0;

    /**
     * Returns a reference to the credentials object for a given connection string and a "role".
     * If the connection string is not known to the service an UnknownConnectionException is thrown.
     * If the role specified does not exist an UnknownRoleException is thrown.
     */
    virtual const IAuthenticationCredentials& credentials( const std::string& connectionString,
                                                           const std::string& role ) const = 0;

    /**
     * Returns a reference to the credentials object for a given connection string, "role" and certificate data.
     * If the connection string is not known to the service an UnknownConnectionException is thrown.
     * If the role specified does not exist an UnknownRoleException is thrown.
     */
    virtual const IAuthenticationCredentials& credentials( const std::string& connectionString,
                                                           const std::string& role,
                                                           const ICertificateData* cert ) const = 0;

  protected:

    /// Protected empty destructor
    virtual ~ICertificateAuthenticationService() {}

  };

}
#endif
