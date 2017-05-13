// -*- C++ -*-
// $Id: Connection.h,v 1.8 2011/03/22 10:36:50 avalassi Exp $
#ifndef FRONTIER_ACCESS_CONNECTION_H
#define FRONTIER_ACCESS_CONNECTION_H 1

#include "CoralCommon/IDevConnection.h"

#include "CoralBase/boost_thread_headers.h"

#include <map>

namespace frontier
{
  class Connection;
}

namespace coral
{
  class ISession;

  namespace FrontierAccess
  {
    class DomainProperties;
    class TypeConverter;

    /**
     * Class Connection
     *
     * Implementation of the IConnection interface for the FrontierAccess module
     */
    class Connection : public coral::IDevConnection
    {
    public:
      /// Constructor
      Connection( const DomainProperties& domainProperties, const std::string& connectionString );

      /// Destructor
      virtual ~Connection();

      /**
       * Connects to a database server without authenticating
       * If no connection to the server can be established a ServerException is thrown.
       */
      virtual void connect();

      /**
       * Returns a new session object.
       * In case no more sessions can be established for the current physical connection,
       * a MaximumNumberOfSessionsException is thrown.
       */
      virtual coral::ISession* newSession( const std::string& schemaName, coral::AccessMode mode = coral::Update ) const;

      /**
       * Returns the connection status. By default this is a logical operation.
       * One should pass true as an argument to force the probing of the physical connection as well.
       */
      virtual bool isConnected( bool probePhysicalConnection = false );

      /**
       * Drops the physical connection with the database.
       */
      virtual void disconnect();

      /**
       * Returns the version of the database server.
       * If a connection is not yet established, a ConnectionNotActiveException is thrown.
       */
      virtual std::string serverVersion() const;

      /**
       * Returns the C++ <-> SQL type converter relevant to he current session.
       * If a connection is not yet established, a ConnectionNotActiveException is thrown.
       */
      virtual coral::ITypeConverter& typeConverter();

    private:
      int parseVersionString( const std::string& versionString );
      bool ping();

    private:
      /// The Frontier physical connection
      frontier::Connection*                            m_connection;
      /// Domain properties
      const coral::FrontierAccess::DomainProperties&   m_domainProperties;
      /// Connection string
      const std::string m_connectionString;
      /// "Physical" connectoin established flag
      bool m_connected;
      /// The server version
      std::string m_serverVersion;
      /// The type converter
      TypeConverter*                                   m_typeConverter;
      /// The connection lock
      static boost::mutex s_lock;
    };
  } // FrontierAccess namespace
} // coral namespace

#endif //  FRONTIER_ACCESS_CONNECTION_H
