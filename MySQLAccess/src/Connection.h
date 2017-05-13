// $Id: Connection.h,v 1.10 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_CONNECTION_H
#define MYSQLACCESS_CONNECTION_H 1

#include "MySQL_headers.h"

#include <map>
#include <string>
#include "CoralBase/boost_thread_headers.h"
#include "CoralCommon/IDevConnection.h"

namespace coral
{
  namespace MySQLAccess
  {
    class DomainProperties;
    class Schema;
    class Transaction;

    typedef std::map<std::string,Schema*> SchemaRegistry;

    /**
     * Class Session
     *
     * Implementation of the ISession interface for the MySQLAccess module
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

      virtual coral::ISession* newSession(const std::string&, coral::AccessMode) const;

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

      void initialize(void);

    private:
      /// The MySQL connection
      MYSQL*                     m_mysql;
      /// The physical connection string
      std::string m_connectionString;
      /// Server version
      mutable std::string m_serverVersion;
      /// The domain props
      const DomainProperties&    m_domainProperties;
      /// Connected flag
      bool m_connected;
      /// lock
      mutable boost::mutex m_lock;
    };
  }
}

#endif // MYSQLACCESS_CONNECTION_H
