#ifndef CONNECTIONSERVICE_SERVICESPECIFICCONFIGURATION_H
#define CONNECTIONSERVICE_SERVICESPECIFICCONFIGURATION_H

#include "ConnectionParams.h"
#include "RelationalAccess/IMonitoring.h"
#include "ConnectionServiceConfiguration.h"

namespace coral
{
  class Context;
}

namespace coral {

  namespace ConnectionService {

    class ServiceSpecificConfiguration {

    public:

      ServiceSpecificConfiguration();

      ServiceSpecificConfiguration(const ConnectionParams& params, const ConnectionServiceConfiguration& configuration);

      virtual ~ServiceSpecificConfiguration(){}

      ServiceSpecificConfiguration(const ServiceSpecificConfiguration& rhs);

      ServiceSpecificConfiguration& operator=(const ServiceSpecificConfiguration& rhs);

      /**
       * Returns the rate of connection retrials (time interval between two retrials).
       */
      int connectionRetrialPeriod() const;

      /**
       * Returns the time out for the connection retrials before the connection
       * service fails over to the next available replica or quits.
       */
      int connectionRetrialTimeOut() const;

      /**
       * Retrieves the connection time out in seconds.
       */
      int connectionTimeOut() const;

      /// returns true if the monitoring level has been set
      bool isMonitoringEnabled() const;

      /**
       * Retrieves the current monitoring level.
       */
      coral::monitor::Level monitoringLevel() const;

      /// access to the service Context
      coral::Context* serviceContext() const;

      /// upper level service name
      const std::string& serviceName() const;

    private:

      void overWriteIfExistsSpecific(int& value, const std::string& paramName) const;

    private:

      std::map<std::string,std::string> m_serviceParams;

      const ConnectionServiceConfiguration* m_configuration;

      std::string m_emptyString;

    };

  }

}

#endif
