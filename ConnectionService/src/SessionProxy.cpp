#include "CoralBase/MessageStream.h"
#include "RelationalAccess/ISession.h"

#include "ConnectionPool.h"
#include "ConnectionService.h"
#include "InvalidSessionProxyException.h"
#include "SessionProxy.h"

using namespace coral::ConnectionService;

/// constructor
SessionProperties::SessionProperties( const SessionHandle& session )
  : m_session( session )
{
}

/// destructor
SessionProperties::~SessionProperties()
{
}

/// Returns the name of the RDBMS flavour..
std::string SessionProperties::flavorName() const
{
  return m_session.connection().technologyName();
}

/// Returns the version of the database server.
std::string SessionProperties::serverVersion() const
{
  return m_session.connection().serverVersion();
}

/// constructor
RemoteSessionProperties::RemoteSessionProperties( const SessionHandle& session )
  : m_session( session )
{
}

/// destructor
RemoteSessionProperties::~RemoteSessionProperties()
{
}

/// Returns the name of the RDBMS flavour..
std::string RemoteSessionProperties::flavorName() const
{
  return m_session.physicalSession()->remoteTechnologyName();
}

/// Returns the version of the database server.
std::string RemoteSessionProperties::serverVersion() const
{
  return m_session.physicalSession()->remoteServerVersion();
}

SessionProxy::SessionProxy( const std::string& connectionString,
                            coral::AccessMode accessMode,
                            ConnectionService* connectionService ) :
  m_connectionString( connectionString ),
  m_role(""),
  m_accessMode( accessMode ),
  m_connectionService( connectionService ),
  m_session( connectionService->name() ),
  m_properties( m_session ),
  m_remoteProperties( m_session )
{
}

SessionProxy::SessionProxy( const std::string& connectionString,
                            const std::string& role,
                            coral::AccessMode accessMode,
                            ConnectionService* connectionService ) :
  m_connectionString( connectionString ),
  m_role( role ),
  m_accessMode( accessMode ),
  m_connectionService( connectionService ),
  m_session( connectionService->name() ),
  m_properties( m_session ),
  m_remoteProperties( m_session )
{
}

SessionProxy::~SessionProxy()
{
  m_session.close();
  if(m_connectionService)
  {
    m_connectionService->connectionPool().releaseConnection( m_session.connection() );
    m_connectionService->unRegisterSession( this );
  }
}

void SessionProxy::open( const coral::ICertificateData *cert )
{
  if( ! m_connectionService ) throw InvalidSessionProxyException(m_connectionString,"SessionProxy::properties" );
  m_session = m_connectionService->connectionPool().getValidSession( m_connectionString, m_role, m_accessMode, cert );
}

void
SessionProxy::reconnect()
{
  // nothing to do if the session is still valid...
  // [SKIP: assume always called as "if ( !m_session.isValid() ) reconnect()"]
  //if ( m_session.isValid() ) return;

  // if the connection service is gone throw here
  if( ! m_connectionService )
    throw InvalidSessionProxyException( m_connectionString,
                                        "SessionProxy::properties" );

  // some logging
  coral::MessageStream log( m_connectionService->name() );
  log << coral::Error
      << "The session is no longer valid: re-open a new session"
      << " (ConnectionService::SessionProxy::reconnect)"
      << coral::MessageStream::endmsg;

  // save the old values of the transaction which was gone
  bool wasActive = m_session.transactionProxy().wasActive();
  bool wasRO = m_session.transactionProxy().wasReadOnly();

  // remove the connection
  m_connectionService->connectionPool().removeConnection( m_session.connection() );

  // set a new session
  m_session = m_connectionService->connectionPool().getValidSession( m_connectionString, m_role, m_accessMode );

  if ( wasActive )
  {
    // if the transaction was lost restart it
    if( wasRO )
    {
      m_session.transactionProxy().start( wasRO );
    }
    // for R/W transactions throw an exception (fix bug #57639 aka bug #73688)
    else
    {
      throw InvalidSessionProxyException( m_session.connectionServiceName(), "R/W Transaction was lost in reconnection process" );
    }
  }
}

coral::ISessionProperties&
SessionProxy::properties()
{
  if ( !m_session.isValid() ) reconnect();
  return m_properties;
}

/// Returns the properties of the remote session.
const coral::ISessionProperties&
SessionProxy::remoteProperties()
{
  if ( !m_session.isValid() ) reconnect();
  return m_remoteProperties;
}

coral::ISchema&
SessionProxy::nominalSchema()
{
  if ( !m_session.isValid() ) reconnect();
  return m_session.physicalSession()->nominalSchema();
}

coral::ISchema&
SessionProxy::schema( const std::string& schemaName )
{
  if ( !m_session.isValid() ) reconnect();
  return m_session.physicalSession()->schema( schemaName );
}

bool
SessionProxy::isConnectionShared() const
{
  // [BUG or missing feature? should we not reconnect here too?...]
  if(!m_session.isValid()) {
    throw InvalidSessionProxyException(m_connectionString,
                                       "SessionProxy::isConnectionShared" );
  }
  return m_session.connection().numberOfSessions()>1;
}

coral::ITransaction&
SessionProxy::transaction()
{
  if ( !m_session.isValid() ) reconnect();
  return m_session.transactionProxy();
}

coral::ITypeConverter&
SessionProxy::typeConverter()
{
  if ( !m_session.isValid() ) reconnect();
  return m_session.connection().typeConverter();
}

coral::ISession*
SessionProxy::session()
{
  if ( !m_session.isValid() ) reconnect();
  return m_session.physicalSession();
}

void
SessionProxy::invalidate()
{
  m_connectionService = 0;
}
