#ifndef CONNECTIONSERVICE_CONNECTIONPARAMS_H
#define CONNECTIONSERVICE_CONNECTIONPARAMS_H

#include <map>
#include <string>
#include "RelationalAccess/AccessMode.h"

namespace coral
{

  namespace ConnectionService
  {

    /// encapsulation of the parameters describing the connection request
    class ConnectionParams
    {

    public:

      /// constructor
      ConnectionParams( const std::string& connectionString );

      /// constructor
      ConnectionParams( const std::string& connectionString,
                        const std::string& serviceName,
                        const std::string& technologyName,
                        const std::string& schemaName,
                        const std::string& userName,
                        const std::string& password );

      /// destructor
      virtual ~ConnectionParams();

      /// copy constructor
      ConnectionParams( const ConnectionParams& rhs);

      /// assignment operator
      ConnectionParams& operator=(const ConnectionParams& rhs);

      /// setter for db parameters
      void setDbParams(const std::string& serviceName,
                       const std::string& technologyName,
                       const std::string& schemaName);

      /// setter for authentication parameters
      void setAuthenticationParams(const std::string& userName,
                                   const std::string& password );

      /// returns the user associated to the connection request
      const std::string& userName() const;

      /// returns the password associated to the connection request
      const std::string& password() const;

      /// returns the connection string associated to the connection request
      const std::string& connectionString() const;

      /// returns the service connection string associated to the connection request
      const std::string& serviceKey() const;

      /// returns the service name associated to the connection request
      const std::string& serviceName() const;

      /// returns the technology name associated to the connection request
      const std::string& technologyName() const;

      /// returns the schema name associated to the connection request
      const std::string& schemaName() const;

      /// returns the reference to the service parameter map
      std::map<std::string,std::string>& serviceParameters();

      /// returns the reference to the service parameter map
      const std::map<std::string,std::string>& serviceParameters() const;

    private:

      /// the stored connectionString
      std::string m_connectionString;

      /// the stored service name
      std::string m_serviceName;

      /// the stored service name
      std::string m_serviceKey;

      /// the stored technology name
      std::string m_technologyName;

      /// the stored schema name
      std::string m_schemaName;

      /// the username
      std::string m_userName;

      /// the password
      std::string m_password;

      /// the service parameter map
      std::map<std::string, std::string> m_serviceParameters;

    };

  }

}
#endif
