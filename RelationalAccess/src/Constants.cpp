#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/IAuthenticationCredentials.h"

// IDatabaseServiceDescription constants

/**
 * Name of the parameter for the connection retrial period
 **/
std::string coral::IDatabaseServiceDescription::connectionRetrialPeriodParam(){
  const std::string s_connectionRetrialPeriodParam = "ConnectionRetrialPeriod";
  return s_connectionRetrialPeriodParam;
}

/**
 * Name of the parameter for the connection retrial time out
 **/
std::string coral::IDatabaseServiceDescription::connectionRetrialTimeOutParam(){
  const std::string s_connectionRetrialTimeOutParam = "ConnectionRetrialTimeOut";
  return s_connectionRetrialTimeOutParam;
}

/**
 * Name of the parameter for the connection time out
 */
std::string coral::IDatabaseServiceDescription::connectionTimeOutParam(){
  const std::string s_connectionTimeOutParam = "ConnectionTimeOut";
  return s_connectionTimeOutParam;
}

/**
 * Name of the parameter for the exclusion time for the missing connections
 */
std::string coral::IDatabaseServiceDescription::missingConnectionExclusionTimeParam(){
  const std::string s_missingConnectionExclusionTimeParam = "MissingConnectionExclusionTime";
  return s_missingConnectionExclusionTimeParam;
}

/**
 * Name of the server name parameter
 */
std::string coral::IDatabaseServiceDescription::serverNameParam(){
  const std::string s_serverNameParam = "ServerName";
  return s_serverNameParam;
}

/**
 * Name of the server status parameter
 */
std::string coral::IDatabaseServiceDescription::serverStatusParam(){
  const std::string s_serverStatusParam = "ServerStatus";
  return s_serverStatusParam;
}

/**
 * Online server status
 */
std::string coral::IDatabaseServiceDescription::serverStatusOnline(){
  const std::string s_serverStatusOnline = "Online";
  return s_serverStatusOnline;

}

/**
 * Offline server status
 */
std::string coral::IDatabaseServiceDescription::serverStatusOffline(){
  const std::string s_serverStatusOffline = "Offline";
  return s_serverStatusOffline;
}

/**
 * Name of the user item
 */
std::string coral::IAuthenticationCredentials::userItem(){
  const std::string s_userItem("user");
  return s_userItem;
}

/**
 * Name of the password item
 */
std::string coral::IAuthenticationCredentials::passwordItem(){
  const std::string s_passwordItem("password");
  return s_passwordItem;
}

/**
 * Name of the default role
 */
std::string coral::IAuthenticationCredentials::defaultRole(){
  const std::string s_defaultRole("coral_default_role");
  return s_defaultRole;
}
