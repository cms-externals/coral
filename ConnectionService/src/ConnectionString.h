#ifndef CONNECTIONSERVICE_CONNECTIONSTRING_H
#define CONNECTIONSERVICE_CONNECTIONSTRING_H

#include <string>

namespace coral {

  namespace ConnectionService {

    /// encapsulation of the connection parameters
    class ConnectionString {

    public:

      ///  parses the connection string and extract the service part
      static std::string serviceConnectionString(const std::string& connectionString);

      ///  parses the connection string and extract the nominal schema
      static std::string nominalSchema(const std::string& connectionString);

      /// recompose the full connection string
      static std::string connectionString(const std::string& serviceConnectionString,const std::string& nominalSchema);

      /// compose a string from technology and service name
      static std::string serviceKey(const std::string& technologyName, const std::string& serviceName);

    };

  }
}

#endif
