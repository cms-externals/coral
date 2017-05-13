#ifndef LFCREPLICASERVICE_LFCCREDENTIALSTRING_H
#define LFCREPLICASERVICE_LFCCREDENTIALSTRING_H

#include <string>

namespace coral {

  namespace LFCReplicaService {

    class LFCCredentialString {

    public:

      static std::string password(const std::string& credentialString,
                                  const std::string& guid);

      static std::string credentials(const std::string& password,
                                     const std::string& guid);

    private:

      static std::string key(const std::string& guid);

    };

  }

}

#endif
