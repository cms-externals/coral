#ifndef RELATIONALACCESS_DATABASESERVICEDESCRIPTION_H
#define RELATIONALACCESS_DATABASESERVICEDESCRIPTION_H

#include "RelationalAccess/IDatabaseServiceDescription.h"
#include <map>

namespace coral {

  /**
   * Class DatabaseServiceDescription
   *
   * Simple implementation for the IDatabaseServiceDescription interface
   */
  class DatabaseServiceDescription : virtual public IDatabaseServiceDescription
  {
  public:
    /// Constructor
    DatabaseServiceDescription( std::string connectionString = "",
                                std::string authenticationMechanism = "",
                                AccessMode mode = Update );

    /// Destructor
    virtual ~DatabaseServiceDescription();

    /// Copy constructor
    DatabaseServiceDescription( const DatabaseServiceDescription& rhs );

  public:
    /**
     * Returns the actual connection string for the database service.
     */
    std::string connectionString() const;

    /**
     * Returns the string describing the authentication mechanism.
     */
    std::string authenticationMechanism() const;

    /**
     * Returns the access mode of the database service
     */
    AccessMode accessMode() const;

    /**
     * Returns the specified parameter
     */
    std::string serviceParameter(const std::string& parameterName) const;

    // Access to service parameter (for filling up)
    std::map<std::string,std::string>& serviceParameters();

  private:
    /// The connection string
    std::string m_connectionString;

    /// The authentication mechanism
    std::string m_authenticationMechanism;

    /// The access mode
    AccessMode m_mode;

    /// The service parameter map
    std::map<std::string,std::string> m_parameters;

  };

}

// inline methods
inline
coral::DatabaseServiceDescription::DatabaseServiceDescription( std::string connectionString,
                                                               std::string authenticationMechanism,
                                                               AccessMode mode ) :
  m_connectionString( connectionString ),
  m_authenticationMechanism( authenticationMechanism ),
  m_mode( mode ),
  m_parameters()
{
}

inline
coral::DatabaseServiceDescription::DatabaseServiceDescription( const coral::DatabaseServiceDescription& rhs ) :
  IDatabaseServiceDescription(),
  m_connectionString( rhs.m_connectionString ),
  m_authenticationMechanism( rhs.m_authenticationMechanism ),
  m_mode( rhs.m_mode ),
  m_parameters( rhs.m_parameters )
{
}

inline
coral::DatabaseServiceDescription::~DatabaseServiceDescription()
{
}

inline std::string
coral::DatabaseServiceDescription::connectionString() const
{
  return m_connectionString;
}

inline std::string
coral::DatabaseServiceDescription::authenticationMechanism() const
{
  return m_authenticationMechanism;
}

inline coral::AccessMode
coral::DatabaseServiceDescription::accessMode() const
{
  return m_mode;
}



inline std::string
coral::DatabaseServiceDescription::serviceParameter( const std::string& parameterName ) const
{
  std::map<std::string,std::string>::const_iterator iter = m_parameters.find(parameterName);
  if( iter != m_parameters.end() )
  {
    return iter->second;
  } else
  {
    return std::string("");
  }
}

inline std::map<std::string,std::string>&
coral::DatabaseServiceDescription::serviceParameters(){
  return m_parameters;
}

#endif
