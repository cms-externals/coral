// $Id: Session.h,v 1.3.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_SESSION_H
#define CORALACCESS_SESSION_H 1

// Include files
#include <string>
#include "CoralCommon/IDevSession.h"
#include "CoralServerBase/ICoralFacade.h"
#include "RelationalAccess/AccessMode.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class ConnectionProperties;
    class Schema;
    class SessionProperties;
    class Transaction;

    /** @class Session
     *
     *  @author Andrea Valassi
     *  @date   2007-12-05
     */

    class Session : virtual public IDevSession
    {

    public:

      /// Constructor
      Session( coral::IDevConnection& connection,
               const ConnectionProperties& connectionProperties,
               const std::string& dbUrl,
               const coral::AccessMode mode );

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
      std::string remoteTechnologyName() const;

      /**
       * Returns the server version for the remote session.
       * For plugins establishing a database connection through a middle-tier
       * (e.g. CoralAccess), this is discovered when establishing the remote session.
       */
      std::string remoteServerVersion() const;

      /// Returns the facade for the CORAL server connection
      const coral::ICoralFacade& facade() const;

      /// Returns the transaction state
      bool isTransactionActive() const;

    private:

      /// The session properties (owned by this session).
      SessionProperties* m_properties;

      /// The schema map (all schemas are owned by this session).
      std::map<std::string,Schema*> m_schemas;

      /// The transaction (owned by this session).
      Transaction* m_transaction;

    };

  }

}
#endif // CORALACCESS_SESSION_H
