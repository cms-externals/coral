#ifndef CONNECTIONSERVICE_REPLICACATALOGUE_H
#define CONNECTIONSERVICE_REPLICACATALOGUE_H

#include <map>
#include <vector>

#include "CoralBase/boost_datetime_headers.h"
#include "CoralBase/boost_thread_headers.h"
#include "RelationalAccess/AccessMode.h"

#include "ConnectionParams.h"

namespace coral {

  namespace ConnectionService {

    class ConnectionServiceConfiguration;

    /// local service incharged of resolve the logical connection and provide the list of replicas
    class ReplicaCatalogue {

    public:
      typedef boost::posix_time::ptime PosixTime;

    public:

      /// constructor
      ReplicaCatalogue( const ConnectionServiceConfiguration& serviceConfiguration );

      /// destructor
      virtual ~ReplicaCatalogue();

      /// resolves the technology specified in the connection string, checking if it is supported by an
      /// existing plug-in...
      bool isConnectionPhysical( const std::string& connectionString ) const;

      /// returns an array of connection strings - the replicas of the specified connection
      std::vector<ConnectionParams>* getReplicas( const std::string& connectionString,
                                                  AccessMode accessMode ) const;
      /// sets the specified connection as unavailable
      void setConnectionExcluded( const std::string& technologyName,
                                  const std::string& serviceName );

      /// checks if the specified connection appears in the list of unavailable
      bool isConnectionExcluded( const std::string& technologyName,
                                 const std::string& serviceName ) const;

    private:

      /// the reference to the service configuration
      const ConnectionServiceConfiguration& m_serviceConfiguration;

      /// the map of (temporarely) unavailable connections
      mutable std::map<std::string,PosixTime> m_excludedConnections;

      /// semaphore
      mutable boost::mutex m_mutexForExclusionList;

    };

  }

}

#endif
