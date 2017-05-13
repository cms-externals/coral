#ifndef SQLITEACCESS_SQLITESESSION_H
#define SQLITEACCESS_SQLITESESSION_H 1

#include <string>
#include "CoralCommon/IDevSession.h"
#include "RelationalAccess/AccessMode.h"

namespace coral
{
  class ISchema;
  class ITransaction;
  class ITypeConverter;

  namespace SQLiteAccess
  {
    class ConnectionProperties;
    class MonitorController;
    class Schema;
    class SessionProperties;
    class Transaction;

    /**
       @class Session Session.h

       Implementation of the ISession interface for the SQLiteAccess module
       @author Zhen Xie
    */

    class Session : virtual public coral::IDevSession
    {
    public:

      /// Constructor
      Session(coral::IDevConnection& connection,
              boost::shared_ptr<ConnectionProperties> connectionProperties,
              const std::string& schemaName,
              coral::AccessMode mode );

      /// Destructor
      virtual ~Session();

      /**
       * Returns the reference to the underlying IMonitoring object.
       */
      coral::IMonitoring& monitoring();

      /**
       * Authenticates with the database server using a user/password pair.
       * If the authentication fails an AuthenticationFailureException is thrown.
       */
      void startUserSession( const std::string& userName,
                             const std::string& password );

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
        return "SQLite"; // same as Domain.flavorName()
      }

    private:

      /// The session properties
      boost::shared_ptr<SessionProperties> m_properties;

      /// The monitoring controller
      MonitorController* m_monitorController;

      /// The Transaction handle
      Transaction* m_transaction;

      /// The schema object
      Schema* m_schema;
    };
  }
}
#endif
