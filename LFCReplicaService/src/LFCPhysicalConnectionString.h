#ifndef LFCREPLICASERVICE_LFCPHYSICALCONNECTIONSTRING_H
#define LFCREPLICASERVICE_LFCPHYSICALCONNECTIONSTRING_H

#include <string>

namespace coral {

  namespace LFCReplicaService {

    class LFCPhysicalConnectionString {

    public:

      LFCPhysicalConnectionString();

      explicit LFCPhysicalConnectionString(const std::string& pfn);

      virtual ~LFCPhysicalConnectionString(){}

      bool isCoralPfn();

      bool parse();

      const std::string& connectionString() const;

      const std::string& userName() const;

      const std::string& guid() const;

      static std::string pfn(const std::string& physicalConnectionString,
                             const std::string& userName);

      static std::string pfn(const std::string& physicalConnectionString,
                             const std::string& userName,
                             const std::string& guid);

      static const std::string CoralPfnTag;

    private:
      std::string m_pfn;

      std::string m_dbPcs;

      std::string m_userName;

      std::string m_guid;
    };

  }

}

#endif
