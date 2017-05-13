// -*- C++ -*-
// $Id: Session.h,v 1.15 2011/03/22 10:36:50 avalassi Exp $
#ifndef FRONTIER_ACCESS_SESSION_H
#define FRONTIER_ACCESS_SESSION_H 1

#include "CoralCommon/IDevSession.h"

#include "RelationalAccess/AccessMode.h"

#include "CoralBase/boost_thread_headers.h"

#include <map>
#include <string>

namespace frontier
{
  class Connection;
}

namespace coral
{
  class IDevConnection;
  class ITypeConverter;

  namespace FrontierAccess
  {
    class DomainProperties;
    class SessionProperties;
    class SessionPropertiesProxy;
    class MonitorController;
    class Transaction;
    class Schema;

    /**
     * Class Session
     *
     * Implementation of the ISession interface for the FrontierAccess module
     */
    class Session : public coral::IDevSession
    {
    public:
      /// Constructor
      Session( coral::IDevConnection& connection,
               const DomainProperties& domainProperties,
               const std::string& connectionString,
               frontier::Connection& fconnection,
               boost::mutex& flock,
               const std::string& schemaName,
               const coral::ITypeConverter& converter );

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
        return "frontier"; // same as Domain.flavorName()
      }

    private:
      /// The properties of the session
      coral::FrontierAccess::SessionProperties* m_properties;

      /// "Physical" connection established flag
      bool m_connected;

      /// The monitoring controller
      coral::FrontierAccess::MonitorController* m_monitorController;

      /// The server version
      std::string m_serverVersion;

      /// The nominal schema object
      coral::FrontierAccess::Schema* m_schema;

      /// The transaction object
      coral::FrontierAccess::Transaction* m_transaction;

      /// A map of the other schemas
      std::map< std::string, std::pair< SessionPropertiesProxy*, Schema* > > m_schemas;
    };
  }
}

#endif
