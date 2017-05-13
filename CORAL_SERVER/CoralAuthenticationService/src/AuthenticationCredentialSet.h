#ifndef LFCLOOKUPSERVICE_AUTHENTITACTIONCREDENTIALSET_H
#define LFCLOOKUPSERVICE_AUTHENTITACTIONCREDENTIALSET_H

#include <map>
#include <string>

namespace coral {

  class AuthenticationCredentials;
  class IAuthenticationCredentials;

  /**
   * @class AuthenticationCredentialSet
   *
   * A simple class holding the roles and the credentials corresponding to a database service
   */
  class AuthenticationCredentialSet
  {
  public:
    /// Constructor
    AuthenticationCredentialSet( const std::string& serviceName,
                                 const std::string& connectionName );

    /// Destructor
    virtual ~AuthenticationCredentialSet();

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

    const std::map< std::string, coral::AuthenticationCredentials* >& roleMap() const;

    bool hasDefault() const;

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
    AuthenticationCredentialSet();
    AuthenticationCredentialSet( const AuthenticationCredentialSet& );
    AuthenticationCredentialSet& operator=( const AuthenticationCredentialSet& );
  };

}

inline
const std::map< std::string, coral::AuthenticationCredentials* >& coral::AuthenticationCredentialSet::roleMap() const {
  return m_data;
}

inline
bool coral::AuthenticationCredentialSet::hasDefault() const {
  return m_default;
}

#endif
