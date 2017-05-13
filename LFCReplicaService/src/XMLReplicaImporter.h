#ifndef LFCREPLICASERVICE_XMLREPLICAIMPORTER_H
#define LFCREPLICASERVICE_XMLREPLICAIMPORTER_H

#include <string>

namespace coral {

  namespace LFCReplicaService {

    class ReplicaSet;

    class XMLReplicaImporter {
    public:

      XMLReplicaImporter(const std::string& serviceCallerName,
                         const std::string& lookupFile,
                         const std::string& authenticationFile);

      virtual ~XMLReplicaImporter(){}

      ReplicaSet* getData();

    private:

      std::string m_serviceCallerName;

      std::string m_lookupFile;

      std::string m_authenticationFile;

    };

  }
}

inline
coral::LFCReplicaService::XMLReplicaImporter::XMLReplicaImporter(const std::string& serviceCallerName,
                                                                 const std::string& lookupFile,
                                                                 const std::string& authenticationFile) :
  m_serviceCallerName(serviceCallerName),m_lookupFile(lookupFile),m_authenticationFile(authenticationFile){
}

#endif
