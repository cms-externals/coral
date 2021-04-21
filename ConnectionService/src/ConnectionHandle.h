#ifndef CONNECTIONSERVICE_CONNECTIONHANDLE_H
#define CONNECTIONSERVICE_CONNECTIONHANDLE_H

#include "ServiceSpecificConfiguration.h"

#include "CoralBase/TimeStamp.h"

#include <boost/shared_ptr.hpp>

typedef unsigned char coral_uuidt[16]; // Redefine a 'coral' uuid_t (bug #35692 on Windows)

namespace coral
{

  class IConnection;
  class ISession;
  class ITypeConverter;

  namespace ConnectionService
  {

    /// ConnectionService guid definition
    typedef std::string Guid;

    /// convenience object to store the info shared on all the handles
    class ConnectionSharedInfo
    {
    public:
      /// constructor
      ConnectionSharedInfo(const std::string& connectionServiceName) :
        m_connectionId(""),
        m_serviceName(""),
        m_open(false),
        m_startIdle(coral::TimeStamp::now(true)),
        m_idle(true),
        m_technologyName(""),
        m_configuration(),
        m_connectionServiceName(connectionServiceName){}
      /// constructor
      ConnectionSharedInfo(const ConnectionParams& connectionParams,
                           const ConnectionServiceConfiguration& configuration) :
        m_connectionId(""),
        m_serviceName(connectionParams.serviceName()),
        m_open(false),
        m_startIdle(coral::TimeStamp::now(true)),
        m_idle(true),
        m_technologyName(connectionParams.technologyName()),
        m_connString( connectionParams.connectionString() ),
        m_configuration(connectionParams,configuration ),
        m_connectionServiceName(configuration.serviceName()){}
      /// destructor
      virtual ~ConnectionSharedInfo()
      {
        showConnectionId( true ); // unregister id from the map
      }
      /// the unique identifier of the connection
      Guid m_connectionId;
      /// the service name
      std::string m_serviceName;
      /// the connection open flag
      bool m_open;
      /// the start time of idle status
      coral::TimeStamp m_startIdle;
      /// the idle flag;
      bool m_idle;
      /// the db technology of the underlying connection
      std::string m_technologyName;
      /// connection string
      std::string m_connString;
      /// the service configuration
      ServiceSpecificConfiguration m_configuration;
      /// the connection service name
      std::string m_connectionServiceName;
      /// show the unique id of the connection
      /// (format is user-controlled via environenment variables)
      const std::string showConnectionId( bool last=false ) const;
    };

    /// handle of the ISession object used internally in the proxy.
    class ConnectionHandle
    {

    public:

      /// constructor
      ConnectionHandle( const std::string& connectionServiceName );

      /// constructor
      ConnectionHandle( coral::IConnection* connection,
                        const ConnectionParams& connectionParams,
                        const ConnectionServiceConfiguration& configuration );

      /// destructor
      virtual ~ConnectionHandle();

      /// copy constructor
      ConnectionHandle( const ConnectionHandle& rhs );

      /// assignment operator
      ConnectionHandle& operator=( const ConnectionHandle& rhs );

      /// returns
      operator bool() const;

      // initialize the connection
      bool open();

      /// finalize (and invalidate) the connection.
      bool close();

      /// returns true if the connection is in open state
      bool isOpen() const;

      /// returns true if the connection is still valid (probed)
      bool isValid() const;

      /// returns a new session
      coral::ISession* newSession(const std::string& schemaName,
                                  const std::string& userName,
                                  const std::string& password,
                                  coral::AccessMode accessMode,
                                  const std::string& sessionId );

      /// returns the connection id
      Guid connectionId() const;

      /// show the connection id
      const std::string showConnectionId() const;

      /// returns the actual connection string
      std::string connectionString() const;

      /// returns the service name
      const std::string& serviceName() const;

      /// returns the technology name
      const std::string& technologyName() const;

      /// returns the number of session opened
      size_t numberOfSessions() const;

      /// returns the server version
      coral::ITypeConverter& typeConverter() const;

      /// returns the server version
      std::string serverVersion() const;

      /// sets the start of idle time
      void setIdle();

      /// retrieves the idle flag
      bool isIdle() const;

      /// returns true if timeout is reached
      bool isExpired() const;

      /// returns the specific time-out of the connection
      int specificTimeOut();

    private:

      /// default constructor
      ConnectionHandle();

      /// stored connection handle
      boost::shared_ptr<IConnection> m_connection;
      /// stored connection (shared) info
      boost::shared_ptr<ConnectionSharedInfo> m_info;

    };

  }

}

#endif
