#ifndef LFCREPLICASERVICE_LFCNAMESERVER_H
#define LFCREPLICASERVICE_LFCNAMESERVER_H

#include <string>

namespace coral {

  namespace LFCReplicaService {

    class LFCReplica;
    class LFCReplicaSet;

    struct LFCFileId;

    class LFCNameServer {

    public:

      LFCNameServer();

      virtual ~LFCNameServer();

      void startTransaction(const std::string& message);

      void commitTransaction();

      void abortTransaction();

      bool isTransactionActive();

      void makeDir(const std::string& absolutePath);

      void rmDir(const std::string& absolutePatg);

      std::string makeFile(const std::string& absolutePath);

      void setPermissionToGroup(const std::string& absolutePath,
                                const std::string& groupName,
                                int mode);

      int getPermissionToGroup(const std::string& absolutePath,
                               const std::string& groupName);

      void rmFile(const std::string& absolutePath);

      void addReplica(const std::string& guid, const LFCReplica& replica);

      void addReplica(const LFCReplica& replica);

      //const LFCReplicaSet* listReplicas( const std::string& absolutePath ) const;

      //LFCReplicaSet* listReplicas( const std::string& poolnameParameter,
      //                             const std::string& serverParameter,
      //                             const std::string& fsParameter);

      size_t deleteReplicas( const std::string& absolutePath );

      size_t deleteReplica( const std::string& absolutePath, const std::string& pfn );

      void deleteReplicaByGuid( const std::string& guid, const std::string& pfn );

      void deleteReplicaByFileId( const LFCFileId& fileId, const std::string& pfn );

      void setReplicaStatus( const std::string& pfn, const std::string& status);

      const std::string lfcHost() const;

    private:

      bool m_transactionActive;
    };

  }
}

#endif
