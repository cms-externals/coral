#ifndef LFCREPLICASERVICE_XMLAUTHENTICATIONFILECONTENT_H
#define LFCREPLICASERVICE_XMLAUTHENTICATIONFILECONTENT_H
#include <iostream>

namespace coral {

  namespace LFCReplicaService {

    class XMLAuthenticationFileContent {

    public:

      explicit XMLAuthenticationFileContent(std::ostream& out);

      bool openConnectionEntry(const std::string& pcs);

      bool closeConnectionEntry();

      bool openRoleEntry(const std::string& roleName);

      bool closeRoleEntry();

      bool addCredentialEntry(const std::string& userName,
                              const std::string& password );

      void close();

    private:

      std::ostream& m_out;

      bool m_connectionListOpen;

      bool m_connectionEntryOpen;

      bool m_roleEntryOpen;

      unsigned int m_ind;

    };

  }
}

#endif
