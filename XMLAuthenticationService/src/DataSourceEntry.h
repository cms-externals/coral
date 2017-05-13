#ifndef XMLAUTHENTITACTIONSERVICE_DATASOURCEENTRY_H
#define XMLAUTHENTITACTIONSERVICE_DATASOURCEENTRY_H

#include <map>
#include <string>

namespace coral {

  class AuthenticationCredentials;
  class IAuthenticationCredentials;

  namespace XMLAuthenticationService {

    /**
     * @class DataSourceEntry
     *
     * A simple class holding the roles and the credentials corresponding to a database service
     */
    class DataSourceEntry
    {
    public:
      /// Constructor
      DataSourceEntry( const std::string& serviceName,
                       const std::string& connectionName );

      /// Destructor
      ~DataSourceEntry();

      /**
       * Adds a credential item to the default role.
       */
      void appendCredentialItem( const std::string& item,
                                 const std::string& value );

      /**
       * Adds a credential item to the default role.
       */
      void appendCredentialItemForRole( const std::string& item,
                                        const std::string& value,
                                        const std::string& role );

      /**
       * Returns a reference to the credentials object for the default role.
       */
      const coral::IAuthenticationCredentials& credentials() const;

      /**
       * Returns a reference to the credentials object for a given role.
       * If the role is not known to the service an UnknownRoleException is thrown.
       */
      const coral::IAuthenticationCredentials& credentials( const std::string& role ) const;

    private:
      /// The service name
      std::string m_serviceName;

      /// The connection name
      std::string m_connectionName;

      /// The input file with the data
      coral::AuthenticationCredentials* m_default;

      /// The structure with the authentication data for the various roles
      std::map< std::string, coral::AuthenticationCredentials* > m_data;

    private:
      DataSourceEntry();
      DataSourceEntry( const DataSourceEntry& );
      DataSourceEntry& operator=( const DataSourceEntry& );
    };

  }

}

#endif
