#ifndef MONITORING_EVENT_DESCRIPTION_H
#define MONITORING_EVENT_DESCRIPTION_H

#include <string>

namespace coral {

  namespace monitor {

    /**
     * Helper class which holds the string literals for the monitoring event descriptions
     */
    class MonitoringEventDescription
    {
    public:
      /// Dummy constructor
      MonitoringEventDescription() {}

      /// Destructor
      ~MonitoringEventDescription() {}

      /// The beginning of a connection
      std::string connectionBegin() const { return "CONNECTION BEGIN"; }

      /// The ending of a connection
      std::string connectionEnd() const { return "CONNECTION END"; }

      /// The beginning of a user session
      std::string sessionBegin() const { return "SESSION BEGIN"; }

      /// The ending of a user session
      std::string sessionEnd() const { return "SESSION END"; }

      /// The starting of a transaction
      std::string transactionStart() const { return "TRANSACTION START"; }

      /// The modes of a transaction
      std::string transactionReadOnly() const { return "READ ONLY"; }
      std::string transactionUpdate() const { return "UPDATE"; }

      /// The committing of a transaction
      std::string transactionCommit() const { return "TRANSACTION COMMIT"; }

      /// The rolling-back of a transaction
      std::string transactionRollback() const { return "TRANSACTION ROLLBACK"; }
    };

  }

}

static const coral::monitor::MonitoringEventDescription monitoringEventDescription;

#endif
