#ifndef SQLITEACCESS_PRIVILEGEMANAGER_H
#define SQLITEACCESS_PRIVILEGEMANAGER_H

#include "RelationalAccess/ITablePrivilegeManager.h"
#include <boost/shared_ptr.hpp>

namespace coral {
  namespace SQLiteAccess {
    class SessionProperties;
    /**
     * @class PrivilegeManager PrivilegeManager.h
     *
     * Implementation of the ITablePrivilegeManager interface for the SQLiteAccess module
     */
    class PrivilegeManager : virtual public coral::ITablePrivilegeManager
    {
    public:
      /// Constructor
      PrivilegeManager( boost::shared_ptr<const SessionProperties> properties,
                        const std::string& tableName);

      /// Destructor
      virtual ~PrivilegeManager();

      /**
       * Grants a privilage to a user.
       *
       * @param accessRight The specific privilege to be granted.
       *
       * @user The user name of the grantee.
       */
      void grantToUser( const std::string& userName ,
                        coral::ITablePrivilegeManager::Privilege accessRight
                        );

      /**
       * Grants a privilage to the public (to everybody).
       *
       * @param accessRight The specific privilege to be granted.
       */
      void grantToPublic( coral::ITablePrivilegeManager::Privilege accessRight );

      /**
       * Revokes a privilage from a user
       *
       * @param accessRight The specific privilege to be revoked.
       *
       * @user The name of the user who is revoked the privilege.
       */
      void revokeFromUser( const std::string& userName,
                           coral::ITablePrivilegeManager::Privilege accessRight);

      /**
       * Revokes a privilage from the public (from everybody).
       *
       * @param accessRight The specific privilege to be revoked.
       */
      void revokeFromPublic( coral::ITablePrivilegeManager::Privilege accessRight );

    private:
      /// The session properties
      boost::shared_ptr<const SessionProperties> m_properties;
      /// The fully qualified table name
      std::string m_tableName;
    };
  } //ns SQLiteAccess
} //ns coral
#endif
