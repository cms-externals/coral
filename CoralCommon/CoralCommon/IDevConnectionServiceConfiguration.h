#ifndef CORALCOMMON_IDEVCONNECTIONSERVICECONFIGURATION_H
#define CORALCOMMON_IDEVCONNECTIONSERVICECONFIGURATION_H

#include "RelationalAccess/IConnectionServiceConfiguration.h"

namespace coral
{

  /// developer-level extension of the interface IConnectionServiceConfiguration
  class IDevConnectionServiceConfiguration : public IConnectionServiceConfiguration {
  public:

    /// returns the reference of the instance in use. Loads default when required.
    virtual coral::IRelationalService& relationalService() const = 0;

    /// returns the reference of the instance in use. Loads default when required.
    virtual coral::IAuthenticationService& authenticationService() const = 0;

    /// returns the reference of the instance in use. Loads default when required.
    virtual coral::ILookupService& lookupService() const = 0;

    /// returns the reference of the instance in use. Loads default when required.
    virtual coral::monitor::IMonitoringService& monitoringService() const = 0;

  protected:

    /// Constructor.
    IDevConnectionServiceConfiguration(){}

    /// Destructor
    virtual ~IDevConnectionServiceConfiguration(){}


  };

}


#endif
