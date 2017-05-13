// $Id: PrivilegeManager.cpp,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <sstream>
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "ISessionProperties.h"
#include "PrivilegeManager.h"
#include "Statement.h"

namespace coral
{
  namespace MySQLAccess
  {

    PrivilegeManager::PrivilegeManager( const coral::MySQLAccess::ISessionProperties& sessionProperties, const std::string& tableName )
      : m_sessionProperties( sessionProperties )
      , m_tableName( tableName )
    {
    }

    void PrivilegeManager::grantToUser( const std::string& userName, coral::ITablePrivilegeManager::Privilege right )
    {
      std::ostringstream os;

      os << "GRANT ";
      switch ( right )
      {
      case coral::ITablePrivilegeManager::Select: os << "SELECT "; break;
      case coral::ITablePrivilegeManager::Update: os << "UPDATE "; break;
      case coral::ITablePrivilegeManager::Insert: os << "INSERT "; break;
      case coral::ITablePrivilegeManager::Delete: os << "DELETE "; break;
      default:                                    os << "SELECT ";
      };
      os << "ON \"" << m_sessionProperties.schemaName() << "\".\"" << m_tableName << "\" TO " << userName;

      std::string sql = os.str(); coral::MySQLAccess::Statement statement( m_sessionProperties, sql ); coral::AttributeList bindData;
      if ( ! statement.execute( bindData ) )
        throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not grant access to a user " + userName , "ITablePrivilegeManager::grantToUser" );
    }

    void PrivilegeManager::revokeFromUser( const std::string& userName, coral::ITablePrivilegeManager::Privilege right )
    {
      std::ostringstream os;

      os << "REVOKE ";
      switch ( right ) {
      case coral::ITablePrivilegeManager::Select: os << "SELECT "; break;
      case coral::ITablePrivilegeManager::Update: os << "UPDATE "; break;
      case coral::ITablePrivilegeManager::Insert: os << "INSERT "; break;
      case coral::ITablePrivilegeManager::Delete: os << "DELETE "; break;
      default:                                    os << "SELECT ";
      };
      os << "ON \"" << m_sessionProperties.schemaName() << "\".\"" << m_tableName << "\" TO " << userName;

      std::string sql = os.str(); coral::MySQLAccess::Statement statement( m_sessionProperties, sql ); coral::AttributeList bindData;
      if ( ! statement.execute( bindData ) )
        throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not revoke access from a user " + userName, "ITablePrivilegeManager::revokeFromUser" );
    }

    void PrivilegeManager::grantToPublic( coral::ITablePrivilegeManager::Privilege )
    {
      // Does not exists in MySQL
    }

    void PrivilegeManager::revokeFromPublic( coral::ITablePrivilegeManager::Privilege )
    {
      // Does not exists in MySQL
    }

  }
}
