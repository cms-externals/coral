#ifndef CORAL_QUERYMGR_H
#define CORAL_QUERYMGR_H

#include "CoralKernel/Service.h"
#include "RelationalAccess/AuthenticationCredentials.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/../src/ICertificateData.h"

#include "LogConTable.h"
#include "PhysConTable.h"
#include "LC2PCTable.h"
#include "CredentialsTable.h"
#include "PermissionsTable.h"

#include "CoralCommon/DatabaseServiceSet.h"

namespace coral {
  class QueryMgrTest;

  class QueryMgr
  {
    friend class coral::QueryMgrTest;

  public:
    // Standard Constructor
    QueryMgr(ISchema& schema);

    /// Standard Destructor
    virtual ~QueryMgr();

    void
    queryConnections( const std::string& lcString,
                      const std::string& pcString="",
                      const std::string& user="",
                      const std::string& dbRole="");

    const AuthenticationCredentials getCredentials( const std::string& physConStr,
                                                    const std::string& dbRole, const ICertificateData* cert );

    DatabaseServiceSet *lookup( const std::string& logicalName, AccessMode accessMode,
                                const std::string& authenticationMechanism );

    /**
     * Intializes the database
     */
    static void initDatabase( ISchema &schema);

    /**
     * Delete the database
     */
    static void deleteDatabase( ISchema &schema);

    /**
     * adds a connection, an alias or credentials to the database
     *
     * Depending on the parameters this methods adds a connection, an alias,
     * credentials or all of them to the database.
     * Empty parameters are ignored. For example only lcString and pcString are specified,
     * the connection String pcString is added to the alias lcString, and the alias
     * lcString is created if necessary.
     */
    void addConnection(const std::string& lcString, const std::string& pcString,
                       const std::string &user, const std::string& passwd, const std::string& role,
                       const std::string &accessMode, const std::string& authMechanism );

    void delConnection(const std::string& lcString, const std::string& pcString,
                       const std::string &user, const std::string& role);

    void addPermission( const std::string& lcString, const std::string& pcString,
                        const std::string& role_,
                        const std::string& dn, const std::string& vomsFQAN );

    int delPermission( const std::string& lcString, const std::string& pcString,
                       const std::string& role_,
                       const std::string& dn, const std::string& vomsFQAN );

  protected:
    /// the schema in which the tables resides
    ISchema &m_schema;

    ///  the tables
    LogConTable m_lcTable;
    PhysConTable m_pcTable;
    LC2PCTable m_lc2pcTable;
    CredentialsTable m_crTable;
    PermissionsTable m_permTable;

  };

}

#endif
