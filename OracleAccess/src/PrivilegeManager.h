#ifndef ORACLEACCESS_PRIVILEGEMANAGER_H
#define ORACLEACCESS_PRIVILEGEMANAGER_H 1

#include <boost/shared_ptr.hpp>
#include "RelationalAccess/ITablePrivilegeManager.h"

namespace coral
{

  namespace OracleAccess
  {

    class SessionProperties;

    /**
     * Class PrivilegeManager
     *
     * Implementation of the ITablePrivilegeManager interface for the OracleAccess module
     */

    class PrivilegeManager : virtual public coral::ITablePrivilegeManager
    {
    public:
      /// Constructor
      PrivilegeManager( boost::shared_ptr<const SessionProperties> properties,
                        const std::string& schemaName,
                        const std::string& tableName );

      /// Destructor
      virtual ~PrivilegeManager() {}

      /**
       * Grants an access right to a specific user.
       */
      void grantToUser( const std::string& userName,
                        coral::ITablePrivilegeManager::Privilege right );

      /**
       * Revokes a right from the specified user.
       */
      void revokeFromUser( const std::string& userName,
                           coral::ITablePrivilegeManager::Privilege right );

      /**
       * Grants the specified right to all users.
       */
      void grantToPublic( coral::ITablePrivilegeManager::Privilege right );

      /**
       * Revokes the specified right from all users.
       */
      void revokeFromPublic( coral::ITablePrivilegeManager::Privilege right );

    private:
      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The schema name for this table
      std::string m_schemaName;

      /// The fully qualified table name
      std::string m_tableName;

    };

  }

}

#endif
