#ifndef MYSQLACCESS_MONITORCONTROLLER_H
#define MYSQLACCESS_MONITORCONTROLLER_H

#include "RelationalAccess/IMonitoring.h"

namespace coral {

  namespace MySQLAccess {

    class SessionProperties;

    /**
     * Class MonitorController
     *
     * Implementation of the IMonitoring interface for the MySQLAccess module
     */
    class MonitorController : virtual public coral::IMonitoring
    {
    public:
      /// Constructor
      explicit MonitorController( SessionProperties& properties );

      /// Destructor
      virtual ~MonitorController();

      /**
       * Starts the client-side monitoring for the current session.
       * Throws a MonitoringServiceNotFoundException if there is no monitoring service available.
       */
      void start( coral::monitor::Level level = coral::monitor::Default );

      /**
       * Stops the client side monitoring.
       * Throws a monitoring exception if something went wrong.
       */
      void stop();

      /**
       * Reports whatever has been gather by the monitoring service to an std::ostream.
       * Throws a MonitoringServiceNotFoundException if there is no monitoring service available.
       */
      void reportToOutputStream( std::ostream& os ) const;

      /**
       * Triggers the reporting of the underlying monitoring service.
       * Throws a MonitoringServiceNotFoundException if there is no monitoring service available.
       */
      void report() const;

      /**
       * Returns the status of the monitoring.
       */
      bool isActive() const;

    private:
      /// Reference to the session properties.
      SessionProperties& m_properties;
    };

  }

}

#endif
