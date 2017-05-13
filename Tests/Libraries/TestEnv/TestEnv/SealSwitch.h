#ifndef TESTCORE_SEALSWITCH_H
#define TESTCORE_SEALSWITCH_H 1

#ifdef CORAL_SEALED

#include "SealKernel/Context.h"
#include "SealKernel/ComponentLoader.h"
#include "SealKernel/IMessageService.h"
#include "PluginManager/PluginManager.h"

#else

#include "CoralKernel/Context.h"

#endif

#include "RelationalAccess/AccessMode.h"

#include <vector>

namespace coral {

  class IAuthenticationCredentials;
  class IRelationalDomain;
  class IConnectionService;
  class IMonitoringReporter;

  class SealSwitcher {
  public:
    SealSwitcher();

    ~SealSwitcher();

    void loadComponent(std::string component);

    coral::IConnectionService& getConnectionService();

    coral::IRelationalDomain& getDomain(const std::string& connectionString);

    const coral::IAuthenticationCredentials& getCredentials(const std::string& connectionString);

    std::vector<std::string> getAllReplicas(const std::string serviceName, coral::AccessMode accessMode = coral::Update, size_t minreplicas = 0);

    bool hasServiceName(const std::string serviceName);

    const coral::IMonitoringReporter& getMonitoringReporter();
#ifdef CORAL_SEALED
    seal::Handle<seal::Context>& getSealContext();
#endif
  private:
#ifdef CORAL_SEALED
    seal::Handle<seal::Context> m_context;
#endif

  };

}

#endif
