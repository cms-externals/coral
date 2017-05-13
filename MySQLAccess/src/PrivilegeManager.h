// $Id: PrivilegeManager.h,v 1.4 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_PRIVILEGEMANAGER_H
#define MYSQLACCESS_PRIVILEGEMANAGER_H 1

#include "RelationalAccess/ITablePrivilegeManager.h"

namespace coral
{
  namespace MySQLAccess
  {
    class ISessionProperties;

    class PrivilegeManager : virtual public coral::ITablePrivilegeManager
    {
    public:

      /**
       * Constructor
       */
      PrivilegeManager( const coral::MySQLAccess::ISessionProperties&, const std::string& tableName );
      /*
       * Destructor
       */
      virtual ~PrivilegeManager() {}
      /**
       * Grants an access right to a specific user.
       */
      virtual void grantToUser( const std::string& userName, coral::ITablePrivilegeManager::Privilege right );
      /**
       * Revokes a right from the specified user.
       */
      virtual void revokeFromUser( const std::string& userName, coral::ITablePrivilegeManager::Privilege right );
      /**
       * Grants the specified right to all users.
       */
      virtual void grantToPublic( coral::ITablePrivilegeManager::Privilege right );
      /**
       * Revokes the specified right from all users.
       */
      virtual void revokeFromPublic( coral::ITablePrivilegeManager::Privilege right );

    private:

      /// The session properties
      const coral::MySQLAccess::ISessionProperties& m_sessionProperties;
      std::string m_tableName;
    };
  }
}

#endif // MYSQLACCESS_PRIVILEGEMANAGER_H
