#ifndef SQLITEACCESS_CONNECTION_H
#define SQLITEACCESS_CONNECTION_H 1

#include <string>
#include <boost/shared_ptr.hpp>
#include "CoralCommon/IDevConnection.h"

namespace coral
{

  namespace SQLiteAccess
  {

    class ConnectionProperties;
    class DomainProperties;
    class Session;

    /**
     * Class Connection
     *
     * Implementation of the IConnection interface for the SQLiteAccess module
     */
    class Connection : virtual public coral::IDevConnection
    {
    public:

      /// Constructor
      Connection( const DomainProperties& domainProperties,
                  const std::string& connectionString );

      /// Destructor
      virtual ~Connection();

      /**
       * Connects to a database server without authenticating
       * If no connection to the server can be established a ServerException is thrown.
       */
      void connect();

      /**
       * Returns a new session object.
       * In case no more sessions can be established for the current physical connection,
       * a MaximumNumberOfSessionsException is thrown.
       */
      coral::ISession* newSession( const std::string& schemaName,
                                   coral::AccessMode mode = coral::Update ) const;

      /**
       * Returns the connection status. By default this is a logical operation.
       * One should pass true as an argument to force the probing of the physical connection as well.
       */
      bool isConnected( bool probePhysicalConnection = false );

      /**
       * Drops the physical connection with the database.
       */
      void disconnect();

      /**
       * Returns the version of the database server.
       * If a connection is not yet established, a ConnectionNotActiveException is thrown.
       */
      std::string serverVersion() const;

      /**
       * Returns the C++ <-> SQL type converter relevant to he current session.
       * If a connection is not yet established, a ConnectionNotActiveException is thrown.
       */
      coral::ITypeConverter& typeConverter();

    private:

      void verifyFileName( const std::string& inFileName ) const;

    private:

      /// The connection properties
      boost::shared_ptr<ConnectionProperties> m_properties;

      /// The sqlite file name
      //mutable seal::Filename m_inFileName;

      /// Logical connection flag
      bool m_isConnected;

      /// The server version
      std::string m_serverVersion;
    };

  }

}

#endif
