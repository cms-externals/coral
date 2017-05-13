#ifndef RELATIONALACCESS_ICERTIFICATECONNECTIONSERVICE_H
#define RELATIONALACCESS_ICERTIFICATECONNECTIONSERVICE_H

#include "RelationalAccess/IConnectionService.h"

namespace coral
{

  // Forward declaration
  class ICertificateData;

  /**
   * Class ICertificateConnectionService
   *
   * Extended IConnectionService supporting certificate authentication.
   * The only difference versus IConnectionService are the connect methods
   * with an ICertificateData argument: eventually these will be added
   * to the default IConnectionService, but this is postponed as it would
   * require a binary-incompatible public API change versus CORAL_2_3_1.
   */

  class ICertificateConnectionService : virtual public IConnectionService
  {

  public:

    /**
     * Returns a session proxy object for the specified connection string
     * and access mode.
     */
    virtual ISessionProxy* connect( const std::string& connectionName,
                                    AccessMode accessMode = Update ) = 0;

    /**
     * Returns a session proxy object for the specified connection string, role
     * and access mode.
     */
    virtual ISessionProxy* connect( const std::string& connectionName,
                                    const std::string& asRole,
                                    AccessMode accessMode = Update ) = 0;

    /**
     * Returns a session proxy object for the specified connection string,
     * access mode and certificate data.
     */
    virtual coral::ISessionProxy* connect( const std::string& connectionString,
                                           AccessMode accessMode,
                                           const ICertificateData* cert ) = 0;

    /**
     * Returns a session proxy object for the specified connection string,
     * role, access mode and certificate data.
     */
    virtual coral::ISessionProxy* connect( const std::string& connectionString,
                                           const std::string& asRole,
                                           AccessMode accessMode,
                                           const ICertificateData* cert ) = 0;

    /**
     * Returns the configuration object for the service.
     */
    virtual IConnectionServiceConfiguration& configuration() = 0;

    /**
     * Cleans up the connection pool from the unused connection, according to
     * the policy defined in the configuration.
     */
    virtual void purgeConnectionPool() = 0;

    /**
     * Returns the monitoring reporter
     */
    virtual const IMonitoringReporter& monitoringReporter() const = 0;

    /**
     * Returns the object which controls the web cache
     */
    virtual IWebCacheControl& webCacheControl() = 0;

  protected:

    /// Protected empty destructor
    virtual ~ICertificateConnectionService() {}

  };

}
#endif
