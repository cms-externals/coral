#ifndef LFCREPLICASERVICE_LFCREPLICASERVICE_H
#define LFCREPLICASERVICE_LFCREPLICASERVICE_H

#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/AuthenticationCredentials.h"
#include "ReplicaStatus.h"
#include "ReplicaPermission.h"
#include "LFCNameServer.h"
#include "LFCNode.h"
#include "LFCDatabaseServiceSet.h"
#include "CoralBase/MessageStream.h"


#include <vector>
#include <map>

namespace coral {

  class AuthenticationCredentialSet;

  namespace LFCReplicaService {

    class ReplicaSet;
    class ReplicaDescription;
    class ReplicaFilter;
    class LFCNameServer;

    class LFCReplicaService : public coral::Service,
                              virtual public coral::ILookupService,
                              virtual public coral::IAuthenticationService
    {
    public:

      /// Standard Constructor with a component key
      explicit LFCReplicaService( const std::string& componentName );

      /// Standard Destructor
      virtual ~LFCReplicaService();

    public:

      /* Performs a lookup for a given logical connection string.  */
      coral::IDatabaseServiceSet* lookup( const std::string& logicalName,
                                          coral::AccessMode accessMode = coral::Update,
                                          std::string authenticationMechanism = "" ) const;
      /**
       * Sets the input file name
       */
      void setInputFileName(  const std::string& /*inputFileName*/ )
      {
      }

      /**
       * Returns a reference to the credentials object for a given connection string.
       * If the connection string is not known to the service an UnknownConnectionException is thrown.
       */
      const IAuthenticationCredentials& credentials( const std::string& connectionString ) const;

      /**
       * Returns a reference to the credentials object for a given connection string and a "role".
       * If the connection string is not known to the service an UnknownConnectionException is thrown.
       * If the role specified does not exist an UnknownRoleException is thrown.
       */
      const IAuthenticationCredentials& credentials( const std::string& connectionString,
                                                     const std::string& role ) const;

    public:

      void addReplica( const std::string& logicalConnectionString,
                       const std::string& role,
                       const ReplicaDescription& replica);

      void importReplicas( const ReplicaSet& replicas );

      ReplicaSet* listReplicas( const std::string& logicalName,
                                const std::string& role,
                                const ReplicaFilter& filter,
                                bool scanChildFolders) const;

      size_t deleteReplicas( const std::string& logicalName,
                             const std::string& role,
                             const ReplicaFilter& filter );

      void setReplicaPassword( const std::string& dbHostName,
                               const std::string& dbUserName,
                               const std::string& newDbPassword );

      bool verifyReplicaPassword( const std::string& dbHostName,
                                  const std::string& dbUserName,
                                  const std::string& currentDbPassword ) const;

      void setReplicaStatus( const std::string& dbHostName,
                             const std::string& dbUserName,
                             ReplicaStatus newStatus);

      void setReplicaAccessPermission( const std::string& logicalName,
                                       const std::string& role,
                                       const std::string& groupName,
                                       ReplicaPermission permission);

    private:

      void clearCredentials() const;

      void appendCredentials(ReplicaDescription& rd, const std::string& filename) const;

      IDatabaseServiceSet* createServiceSet(LFCNode&, ReplicaFilter& filter) const;

      LFCNode& lookupNode( const std::string& logicalName ) const;

      void listReplicasAppend(LFCNode&, ReplicaSet&, const ReplicaFilter&) const;

    private:

      mutable LFCNameServer m_nameServer;

      mutable std::map<std::string, coral::AuthenticationCredentialSet*> m_credentialMap;

      //mutable std::map<std::string, LFCNode*> m_nodes;

      mutable LFCNode* m_rootNode;

      mutable coral::MessageStream m_logger;

    };
  }

}

#endif
