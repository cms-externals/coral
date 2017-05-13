#ifndef LFCREPLICASERVICE_LFCREPLICA_H
#define LFCREPLICASERVICE_LFCREPLICA_H

#include <string>
#include <vector>

#define NSTYPE_LFC
#include <sys/stat.h> //mode_t in lfc_api.h
#include <lfc_api.h>

namespace coral {

  namespace LFCReplicaService {

    class ReplicaDescription;

    struct LFCFileId {
      LFCFileId();
      LFCFileId(const LFCFileId& rhs);
      LFCFileId& operator=(const LFCFileId& rhs);
      u_signed64 id;
      std::string lfcHost;
    };

    class LFCReplica
    {

    public:

      LFCReplica(lfc_filereplica* replica);

      LFCReplica(const LFCReplica& rhs);

      LFCReplica(const ReplicaDescription& rd, const std::string& guid);

      ~LFCReplica();

    public:

      u_signed64 fileid;

      std::string sfn;

      bool status;

      std::string host;

      char f_type;

      std::string poolname;

      std::string fs;

    };

    class LFCReplicaSet {

    public:

      LFCReplicaSet() : m_replicas(){}

      virtual ~LFCReplicaSet();

      const std::vector<LFCReplica*>& replicas() const { return m_replicas; };

      std::vector<LFCReplica*>& replicas() { return m_replicas; };

    private:

      std::vector<LFCReplica*> m_replicas;
    };

  }
}

inline
coral::LFCReplicaService::LFCFileId::LFCFileId() : id(0),lfcHost(""){
}

inline
coral::LFCReplicaService::LFCFileId::LFCFileId(const LFCFileId& rhs) : id(rhs.id),lfcHost(rhs.lfcHost){
}

inline
coral::LFCReplicaService::LFCFileId& coral::LFCReplicaService::LFCFileId::operator=(const LFCFileId& rhs){
  id = rhs.id;
  lfcHost = rhs.lfcHost;
  return *this;
}



#endif
