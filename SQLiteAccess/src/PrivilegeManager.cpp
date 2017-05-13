#include <iostream>
#include "PrivilegeManager.h"
#include "SessionProperties.h"
#include <sstream>
//#include <iostream>
coral::SQLiteAccess::PrivilegeManager::PrivilegeManager( boost::shared_ptr<const SessionProperties> properties,const std::string& tableName) :
  m_properties(properties), m_tableName( tableName ){

}

coral::SQLiteAccess::PrivilegeManager::~PrivilegeManager()
{

}

void
coral::SQLiteAccess::PrivilegeManager::grantToUser( const std::string& /* user */ ,
                                                    coral::ITablePrivilegeManager::Privilege /* accessRight */)
{

  return;
}

void
coral::SQLiteAccess::PrivilegeManager::grantToPublic( coral::ITablePrivilegeManager::Privilege /* accessRight */ )
{

  return;
}

void
coral::SQLiteAccess::PrivilegeManager::revokeFromUser( const std::string& /* user */ ,
                                                       coral::ITablePrivilegeManager::Privilege /* accessRight */ )
{

  return;
}

void
coral::SQLiteAccess::PrivilegeManager::revokeFromPublic( coral::ITablePrivilegeManager::Privilege /* accessRight */ )
{

  return;
}
