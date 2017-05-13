#ifndef LFCREPLICASERVICE_LFCDATABASESERVICESET_H
#define LFCREPLICASERVICE_LFCDATABASESERVICESET_H

#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/AccessMode.h"
#include <map>
#include <vector>
#include "ReplicaSet.h"

namespace coral {

  namespace LFCReplicaService {

    class LFCDatabaseServiceDescription : virtual public IDatabaseServiceDescription
    {

    public:

      /// Constructor
      LFCDatabaseServiceDescription( ReplicaDescription* replica );

      virtual ~LFCDatabaseServiceDescription();

      std::string connectionString() const { return m_replica->physicalConnectionString(); }

      std::string authenticationMechanism() const { return m_replica->authenticationMechanism(); }

      coral::AccessMode accessMode() const { return m_replica->accessMode(); }

      std::string serviceParameter(const std::string& parameterName) const;

    private:

      ReplicaDescription* m_replica;

    };

    class LFCDatabaseServiceSet : virtual public IDatabaseServiceSet
    {

    public:

      /// Constructor
      LFCDatabaseServiceSet();

      /// Destructor
      virtual ~LFCDatabaseServiceSet();

      void appendReplica(ReplicaDescription* replica);

    public:

      /**
       * Returns the number of actual database services corresponding to
       * the logical name.
       */
      int numberOfReplicas() const;

      /**
       * Returns a reference to the service description object corresponding
       * to the specified index.
       * If the index is out of range an InvalidReplicaIdentifierException is thrown.
       */
      const IDatabaseServiceDescription& replica( int index ) const;

    private:

      mutable bool m_sorted;

      mutable std::vector<LFCDatabaseServiceDescription*> m_replicas;

      std::map<std::string,LFCDatabaseServiceDescription*> m_replicaDescriptions;

    };

  }
}

#endif
