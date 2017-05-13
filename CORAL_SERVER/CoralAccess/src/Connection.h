// $Id: Connection.h,v 1.3.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_CONNECTION_H
#define CORALACCESS_CONNECTION_H 1

// Include files
#include "CoralCommon/IDevConnection.h"
#include "CoralServerBase/IRequestHandler.h"
#include "CoralServerBase/ICoralFacade.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class ConnectionProperties;
    class DomainProperties;

    /** @class Connection
     *
     *  @author Andrea Valassi
     *  @date   2007-12-04
     */

    class Connection : virtual public IDevConnection
    {

    public:

      /// Constructor
      Connection( const DomainProperties& domainProperties,
                  const std::string& coralServerUrl );

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
      ISession* newSession( const std::string& schemaName,
                            AccessMode mode = Update ) const;

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
      ITypeConverter& typeConverter();

    private:

      /// The connection properties (owned by this Connection).
      ConnectionProperties* m_properties;

      /// The request handler owned by this Connection (if it exists).
      /// For connections to a CORAL server, this is the socket client.
      /// For local CoralServerFacade connections, this does not exist.
      IRequestHandler* m_handler;

      /// The CORAL server facade owned by this Connection (if it exists).
      /// For connections to a CORAL server, this is the client stub.
      /// For local CoralServerFacade connections, this does not exist
      /// (the CoralServerFacade is a plugin owned by the Context).
      ICoralFacade* m_facade;

    };

  }

}
#endif // CORALACCESS_CONNECTION_H
