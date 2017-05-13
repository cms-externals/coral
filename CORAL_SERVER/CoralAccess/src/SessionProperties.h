// $Id: SessionProperties.h,v 1.5.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_SESSIONPROPERTIES_H
#define CORALACCESS_SESSIONPROPERTIES_H 1

// Include files
#include <string>
#include "CoralServerBase/ICoralFacade.h"
#include "RelationalAccess/AccessMode.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declaration
    class ConnectionProperties;
    class Session;

    /** @class SessionProperties
     *
     *  A class holding the parameters of a remote database session
     *  established over a CORAL server connection.
     *
     *  This class is similar to the CoralAccess class with the same name,
     *  but it does not implement the coral::ISessionProperties interface.
     *
     *  @author Andrea Valassi
     *  @date   2008-12-18
     */

    class SessionProperties
    {

    public:

      /// Constructor
      SessionProperties( const ConnectionProperties& connectionProperties,
                         const std::string& databaseUrl,
                         const AccessMode accessMode,
                         const Session& session );

      /// Destructor
      virtual ~SessionProperties();

      /// Sets the state of the remote database session.
      void setState( Token sessionID,
                     bool fromProxy,
                     bool isUserSessionActive );

      /// Sets the remote session properties.
      void setRemoteProperties( const std::string& remoteTechnologyName,
                                const std::string& remoteServerVersion,
                                const std::string& remoteNominalSchemaName );

      /// Returns the connection properties
      const ConnectionProperties& connectionProperties() const;

      /// Returns the URL of the remote database session.
      const std::string& databaseUrl() const;

      /// Returns the access mode of the remote database session.
      AccessMode accessMode() const;

      /// Returns the sessionID of the remote database session.
      Token sessionID() const;

      /// Is there a CORAL server proxy between client and server?
      bool fromProxy() const;

      /// Is the remote user session active?
      bool isUserSessionActive() const;

      /// Returns the technology name of the remote database session.
      const std::string& remoteTechnologyName() const;

      /// Returns the server version of the remote database session.
      const std::string& remoteServerVersion() const;

      /// Returns the nominal schema name of the remote database session.
      const std::string& remoteNominalSchemaName() const;

      /// Returns the transaction state
      bool isTransactionActive() const;

    private:

      /// The properties of the connection to the CORAL server.
      const ConnectionProperties& m_connectionProperties;

      /// The URL for the remote database session.
      const std::string m_databaseUrl;

      /// The access mode for this session.
      const AccessMode m_accessMode;

      /// The session reference.
      const Session& m_session;

      /// The sessionID for this session.
      Token m_sessionID;

      /// Is there a CORAL server proxy between client and server?
      bool m_fromProxy;

      /// Is the remote user session active?
      bool m_isUserSessionActive;

      /// The technology name of the remote database session.
      std::string m_remoteTechnologyName;

      /// The server version of the remote database session.
      std::string m_remoteServerVersion;

      /// The nominal schema name of the remote database session.
      std::string m_remoteNominalSchemaName;

    };

  }

}
#endif // CORALACCESS_SESSIONPROPERTIES_H
