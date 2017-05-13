// $Id: Session.h,v 1.16 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_SESSION_H
#define MYSQLACCESS_SESSION_H 1

#include "MySQL_headers.h"

#include <map>
#include <string>
#include "CoralBase/boost_thread_headers.h"
#include "CoralCommon/IDevSession.h"
#include "RelationalAccess/AccessMode.h"

namespace coral
{
  class IDevConnection;

  namespace MySQLAccess
  {

    class DomainProperties;
    class SessionProperties;
    class MonitorController;
    class Schema;
    class Transaction;

    typedef std::map<std::string,Schema*> SchemaRegistry;

    /**
     * Class Session
     *
     * Implementation of the ISession interface for the MySQLAccess module
     */
    class Session : public coral::IDevSession
    {
    public:
      /// Constructor
      Session(  coral::IDevConnection& connection,
                const DomainProperties& domainProperties,
                const std::string& connectionString, const std::string& schemaName,
                MYSQL*& handle, bool& connected,
                boost::mutex& lock,
                coral::AccessMode mode );

      /// Destructor
      virtual ~Session();

      /**
       * Returns the reference to the underlying IMonitoring object.
       */
      coral::IMonitoring& monitoring();

      /**
       * Returns the nominal schema name for the given connection string
       */
      std::string nominalSchemaNameForConnection( const std::string& connectionName ) const;

      /**
       * Authenticates with the database server using a user/password pair.
       * If the authentication fails an AuthenticationFailureException is thrown.
       */
      void startUserSession( const std::string& userName, const std::string& password );

      /**
       * Terminates a user session without dropping the connection to the database server
       */
      void endUserSession();

      /**
       * Returns the status of a user session.
       */
      bool isUserSessionActive() const;

      /**
       * Returns the corresponding ITransaction object.
       * If a connection is not yet established, a ConnectionNotActiveException is thrown.
       */
      coral::ITransaction& transaction();

      /**
       * Returns a reference to the working ISchema object.
       * If a connection is not yet established, a ConnectionNotActiveException is thrown.
       */
      coral::ISchema& nominalSchema();

      /**
       * Returns a reference to the ISchema object corresponding to the specified name.
       * If a connection is not yet established, a ConnectionNotActiveException is thrown.
       * If no schema exists corresponding to the specified name an InvalidSchemaNameException is thrown.
       */
      coral::ISchema& schema( const std::string& schemaName );

      /**
       * Returns the technology name for the remote session.
       * For plugins establishing a database connection through a middle-tier
       * (e.g. CoralAccess), this is discovered when establishing the remote session.
       */
      std::string remoteTechnologyName() const
      {
        return "MySQL"; // same as Domain.flavorName()
      }

    private:
      /// The session properties
      SessionProperties* m_properties;

      /// The monitoring controller
      MonitorController* m_monitorController;

      /// The schema object
      std::string m_nominalSchemaName;

      /// The schema object
      Schema*            m_schema;

      /// The registry of opened schemas
      SchemaRegistry m_schreg;

      /// The transaction object
      Transaction*       m_transaction;

      /// Access mode
      coral::AccessMode m_accessMode;

      /// lock
      boost::mutex&      m_lock;
    };
  }
}

#endif // MYSQLACCESS_SESSION_H
