#include "ConnectionParams.h"

coral::ConnectionService::ConnectionParams::ConnectionParams( const std::string& connectionString ) :
  m_connectionString( connectionString ),
  m_serviceName( "" ),
  m_serviceKey( "" ),
  m_technologyName( "" ),
  m_schemaName( "" ),
  m_userName( "" ),
  m_password( "" ),
  m_serviceParameters(){
}

coral::ConnectionService::ConnectionParams::ConnectionParams( const std::string& connectionString,
                                                              const std::string& serviceName,
                                                              const std::string& technologyName,
                                                              const std::string& schemaName,
                                                              const std::string& userName,
                                                              const std::string& password) :
  m_connectionString( connectionString ),
  m_serviceName( serviceName ),
  m_serviceKey( technologyName ),
  m_technologyName( technologyName ),
  m_schemaName( schemaName ),
  m_userName( userName ),
  m_password( password ),
  m_serviceParameters(){
  m_serviceKey.append(":").append(m_serviceName);
}

/// destructor
coral::ConnectionService::ConnectionParams::~ConnectionParams(){
}

/// copy constructor
coral::ConnectionService::ConnectionParams::ConnectionParams( const ConnectionParams& rhs) :
  m_connectionString( rhs.m_connectionString ),
  m_serviceName(  rhs.m_serviceName ),
  m_serviceKey( rhs.m_serviceKey ),
  m_technologyName(  rhs.m_technologyName ),
  m_schemaName(  rhs.m_schemaName ),
  m_userName( rhs.m_userName ),
  m_password( rhs.m_password ),
  m_serviceParameters( rhs.m_serviceParameters ){
}

/// assignment operator
coral::ConnectionService::ConnectionParams&
coral::ConnectionService::ConnectionParams::operator=(const ConnectionParams& rhs){
  m_connectionString = rhs.m_connectionString;
  m_serviceName = rhs.m_serviceName;
  m_serviceKey = rhs.m_serviceKey;
  m_technologyName = rhs.m_technologyName;
  m_schemaName = rhs.m_schemaName;
  m_userName = rhs.m_userName;
  m_password = rhs.m_password;
  m_serviceParameters = rhs.m_serviceParameters;
  return *this;
}

void
coral::ConnectionService::ConnectionParams::setDbParams(const std::string& serviceName,
                                                        const std::string& technologyName,
                                                        const std::string& schemaName){
  m_serviceName = serviceName;
  m_serviceKey = technologyName;
  m_serviceKey.append(":").append(m_serviceName);
  m_technologyName = technologyName;
  m_schemaName = schemaName;
}

void
coral::ConnectionService::ConnectionParams::setAuthenticationParams(const std::string& userName,
                                                                    const std::string& password ){
  m_userName = userName;
  m_password = password;
}

/// returns the user associated to the connection request
const std::string&
coral::ConnectionService::ConnectionParams::userName() const {
  return m_userName;
}

/// returns the user password associated to the connection request
const std::string&
coral::ConnectionService::ConnectionParams::password() const {
  return m_password;
}

/// returns the service connection name associated to the connection request
const std::string&
coral::ConnectionService::ConnectionParams::connectionString() const {
  return m_connectionString;
}

/// returns the service connection string associated to the connection request
const std::string&
coral::ConnectionService::ConnectionParams::serviceName() const {
  return m_serviceName;
}

/// returns the service connection string associated to the connection request
const std::string&
coral::ConnectionService::ConnectionParams::serviceKey() const {
  return m_serviceKey;
}

/// returns the service connection string associated to the connection request
const std::string&
coral::ConnectionService::ConnectionParams::technologyName() const {
  return m_technologyName;
}

/// returns the schema name associated to the connection request
const std::string&
coral::ConnectionService::ConnectionParams::schemaName() const {
  return m_schemaName;
}

/// returns the reference to the service parameter map
std::map<std::string,std::string>&
coral::ConnectionService::ConnectionParams::serviceParameters(){
  return m_serviceParameters;
}

/// returns the reference to the service parameter map
const std::map<std::string,std::string>&
coral::ConnectionService::ConnectionParams::serviceParameters() const {
  return m_serviceParameters;
}
