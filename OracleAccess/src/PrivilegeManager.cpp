#include <sstream>

#include "CoralBase/AttributeList.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "OracleStatement.h"
#include "PrivilegeManager.h"
#include "SessionProperties.h"


coral::OracleAccess::PrivilegeManager::PrivilegeManager( boost::shared_ptr<const SessionProperties> properties,
                                                         const std::string& schemaName,
                                                         const std::string& tableName ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
  m_tableName( tableName )
{
}


void
coral::OracleAccess::PrivilegeManager::grantToUser( const std::string& userName,
                                                    coral::ITablePrivilegeManager::Privilege right )
{
  std::ostringstream os;
  os << "GRANT ";
  switch ( right ) {
  case coral::ITablePrivilegeManager::Select:
    os << "SELECT ";
    break;
  case coral::ITablePrivilegeManager::Update:
    os << "UPDATE ";
    break;
  case coral::ITablePrivilegeManager::Insert:
    os << "INSERT ";
    break;
  case coral::ITablePrivilegeManager::Delete:
    os << "DELETE ";
    break;
  default:
    os << "SELECT ";
  };

  os << "ON " << m_tableName << " TO " << userName;
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );

  coral::AttributeList bindData;
  if ( ! statement.execute( bindData ) ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not grant access to a user",
                                  "ITablePrivilegeManager::grantToUser" );
  }
}


void
coral::OracleAccess::PrivilegeManager::revokeFromUser( const std::string& userName,
                                                       coral::ITablePrivilegeManager::Privilege right )
{
  std::ostringstream os;
  os << "REVOKE ";
  switch ( right ) {
  case coral::ITablePrivilegeManager::Select:
    os << "SELECT ";
    break;
  case coral::ITablePrivilegeManager::Update:
    os << "UPDATE ";
    break;
  case coral::ITablePrivilegeManager::Insert:
    os << "INSERT ";
    break;
  case coral::ITablePrivilegeManager::Delete:
    os << "DELETE ";
    break;
  default:
    os << "SELECT ";
  };

  os << "ON " << m_tableName << " FROM " << userName;
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );

  coral::AttributeList bindData;
  if ( ! statement.execute( bindData ) ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not revoke access from a user",
                                  "ITablePrivilegeManager::revokeFromUser" );
  }
}


void
coral::OracleAccess::PrivilegeManager::grantToPublic( coral::ITablePrivilegeManager::Privilege right )
{
  this->grantToUser( "PUBLIC", right );
}


void
coral::OracleAccess::PrivilegeManager::revokeFromPublic( coral::ITablePrivilegeManager::Privilege right )
{
  this->revokeFromUser( "PUBLIC", right );
}
