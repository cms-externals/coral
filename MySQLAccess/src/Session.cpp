// $Id: Session.cpp,v 1.42 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"
#include "errmsg.h"
#include "mysqld_error.h"

#include "CoralBase/MessageStream.h"
#include "CoralKernel/Service.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/URIParser.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "MonitorController.h"
#include "Schema.h"
#include "Session.h"
#include "SessionProperties.h"
#include "Transaction.h"

namespace coral
{
  namespace MySQLAccess
  {

    enum TxIsolationLevel
      {
        ReadUncommitted, // READ-UNCOMMITTED
        ReadCommitted, // READ-COMMITTED
        RepeatableRead, // REPEATABLE-READ
        Serializable // SERIALIZABLE
      };

#if (  MYSQL_VERSION_ID > 40100 )

    // Version using MySQL C API 4.1.x or higher
    struct SessionConfigurator
    {
      SessionConfigurator( const ISessionProperties& props ) : m_props( props )  { }

      bool autocommit( bool OnOff )
      {
        boost::mutex::scoped_lock lock( m_props.lock() );

        if( mysql_autocommit( m_props.connectionHandle(), (OnOff ? 1 : 0) ) )
          return false;

        return true;
      }

      bool txIsolationLevel( TxIsolationLevel level=ReadCommitted )
      {
        boost::mutex::scoped_lock lock( m_props.lock() );

        std::string sql = "SET SESSION TRANSACTION ISOLATION LEVEL ";

        switch( level )
        {
        case ReadUncommitted: sql += "READ UNCOMMITTED";
          break;
        case ReadCommitted:   sql += "READ COMMITTED";
          break;
        case RepeatableRead:  sql += "REPEATABLE READ";
          break;
        case Serializable:    sql += "SERIALIZABLE";
          break;
        };

        if( mysql_query( m_props.connectionHandle(), sql.c_str() ) )
          return false;

        return true;
      }

      const ISessionProperties& m_props;
    };

#else

    // Simple version going via SQL
    // Works for any MySQL server version
    struct SessionConfigurator
    {
      SessionConfigurator( const ISessionProperties& props ) : m_props( props )  { }

      bool autocommit( bool OnOff )
      {
        boost::mutex::scoped_lock lock( m_props.lock() );

        std::string sql = "SET AUTOCOMMIT=";

        if( OnOff == true )
          sql += "1";
        else
          sql += "0";

        if( mysql_query( m_props.connectionHandle(), sql.c_str() ) )
          return false;

        return true;
      }

      bool txIsolationLevel( TxIsolationLevel level=ReadCommitted )
      {
        boost::mutex::scoped_lock lock( m_props.lock() );

        std::string sql = "SET SESSION TRANSACTION ISOLATION LEVEL ";

        switch( level )
        {
        case ReadUncommitted: sql += "READ UNCOMMITTED";
          break;
        case ReadCommitted:   sql += "READ COMMITTED";
          break;
        case RepeatableRead: sql += "REPEATABLE READ";
          break;
        case Serializable:   sql += "SERIALIZABLE";
          break;
        };

        if( mysql_query( m_props.connectionHandle(), sql.c_str() ) )
          return false;

        return true;
      }

      const ISessionProperties& m_props;
    };

#endif

    // We try without conditional compilation as it seems that the API is stable
    struct SchemaLoader
    {
      SchemaLoader( const ISessionProperties& props ) : m_props( props )  { }

      bool schemaExists( const std::string& schemaName )
      {
        boost::mutex::scoped_lock lock( m_props.lock() );

        MYSQL_RES* result = 0;

        if( (result = mysql_list_dbs( m_props.connectionHandle(), NULL /* LIST ALL DBS */ )) == 0 )
        {
          coral::MySQLAccess::ErrorHandler errorHandler;
          MYSQL_HANDLE_ERR(errorHandler,m_props.connectionHandle(),"Getting list of schemas");
          throw coral::SessionException( errorHandler.message(), "MySQLAccess::SchemaLoader", m_props.domainProperties().service()->name() );
        }

        MYSQL_ROW row; bool found = false;

        while( (row = mysql_fetch_row(result)) )
        {
          std::string foundSchema( row[0] );
          if( foundSchema == schemaName )
          {
            found = true; break;
          }
        }

        mysql_free_result( result );

        return found;
      }

      const ISessionProperties& m_props;
    };

  }
}

coral::MySQLAccess::Session::Session( coral::IDevConnection& connection,
                                      const DomainProperties& domainProperties,
                                      const std::string& connectionString, const std::string& schemaName, MYSQL*& handle, bool& connected,
                                      boost::mutex& conlock,
                                      coral::AccessMode mode )
  : coral::IDevSession( connection )
  , m_properties( 0 )
  , m_monitorController( 0 )
  , m_nominalSchemaName( schemaName )
  , m_schema( 0 )
  , m_schreg()
  , m_transaction( 0 )
  , m_accessMode( mode )
  , m_lock( conlock )
{
  m_properties        = new coral::MySQLAccess::SessionProperties( domainProperties, connectionString, schemaName, handle, connected, conlock, *this, mode );
  m_monitorController = new coral::MySQLAccess::MonitorController( *m_properties ) ;
  m_schema            = new coral::MySQLAccess::Schema( *m_properties, schemaName );

  m_schreg.clear();
  m_transaction       = new coral::MySQLAccess::Transaction( *m_properties, *m_schema );
}

coral::MySQLAccess::Session::~Session()
{
  if ( this->isUserSessionActive() )
    this->endUserSession();

  if( this->m_transaction != 0 )
    delete this->m_transaction;

  if( this->m_monitorController != 0 )
    delete this->m_monitorController;

  // Delete is safe here, it got the value from the constructor
  delete this->m_properties;

  if( this->m_schema != 0 )
    delete this->m_schema;
}

coral::IMonitoring& coral::MySQLAccess::Session::monitoring()
{
  return *m_monitorController;
}

std::string coral::MySQLAccess::Session::nominalSchemaNameForConnection( const std::string& connectionName ) const
{
  coral::URIParser parser;
  parser.setURI( connectionName );
  return parser.databaseOrSchemaName();
}

void coral::MySQLAccess::Session::startUserSession( const std::string& userName, const std::string& password )
{
  MYSQL*& handle = m_properties->connectionHandle();

  if ( ! this->isUserSessionActive() )
  {
    if( ! m_properties->isConnected() )
    {
      boost::mutex::scoped_lock lock( m_lock );

      coral::MessageStream log( m_properties->domainProperties().service()->name() );

      coral::URIParser parser;

      std::string dburi = "mysql://" + (m_properties->connectionString());

      parser.setURI( dburi );

      log << coral::Debug << "Starting user session by connecting to: " << dburi << " as \"" << userName << "\"" << std::endl << coral::MessageStream::endmsg;

      // Make "real" conection with authenticated user
      if ( ! mysql_real_connect( handle,
                                 parser.hostName().c_str(),
                                 userName.c_str(), password.c_str(), 0 /* no database */,
                                 parser.portNumber(),
                                 0,
                                 CLIENT_FOUND_ROWS ) ) // This connect time setting ensures compatible behavior with OracleAccess for UPDATE
      {
        // Could not connect to the server
        coral::MySQLAccess::ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler, handle,"Starting a user session");
        if( errorHandler.lastErrorCode() == CR_SERVER_LOST ||
            errorHandler.lastErrorCode() == CR_CONN_HOST_ERROR ||
            errorHandler.lastErrorCode() == CR_CONNECTION_ERROR )
          throw coral::StartSessionException( m_properties->domainProperties().service()->name(), errorHandler.message() );
        else
          throw coral::SessionException( m_properties->domainProperties().service()->name(), errorHandler.message(), "coral::MySQLAccess::startUserSession|connect" );
      }

      m_properties->setConnected( true );
    } // End of critical section
    else
    {
      boost::mutex::scoped_lock lock( m_lock );

      coral::MessageStream log( m_properties->domainProperties().service()->name() );
      log << coral::Debug << "Starting user session by re-authentication as: " << userName << std::endl << coral::MessageStream::endmsg;

      // Just re-authenticate
      if ( mysql_change_user( handle, userName.c_str(), password.c_str(), 0 ) != 0 )
      {
        coral::MySQLAccess::ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,handle,"Re-authenticating");
        if( errorHandler.lastErrorCode() == CR_SERVER_LOST )
          throw coral::StartSessionException( m_properties->domainProperties().service()->name(), errorHandler.message() );
        else
          throw coral::SessionException( m_properties->domainProperties().service()->name(), errorHandler.message(), "coral::MySQLAccess::startUserSession|re-authenticate" );
      }

      // Make sure the cached schema metadata are refreshed
      m_schema->setTableListStale();
    }
  }

  m_properties->setUserSession();

  // Configure default sesion options
  SessionConfigurator sesconf( *m_properties );

  // Retrieve server version
#if (  MYSQL_VERSION_ID > 40100 )
  unsigned long srv_version = mysql_get_server_version( handle );
  unsigned long major       = srv_version / 10000;
  unsigned long minor       = ( srv_version - major * 10000 ) / 100 ;
  unsigned long revision    = srv_version - major * 10000 - minor * 100;

  std::string srv_info = mysql_get_server_info( handle );

  m_properties->setServerVersion( srv_info );
  m_properties->setServerRevision( major, minor, revision );
#else
  std::string srv_info = mysql_get_server_info( handle );
  m_properties->setServerVersion( srv_info );

  // Try the best with MySQL 4.0.x client API where server version string is formatted as: 'major'.'minor'.'revision'-(standard/max)-log
  std::string str_version( srv_info.substr( 0, srv_info.find_first_of( '-' ) ) );

  unsigned long major     = atol( str_version.substr( 0, str_version.find_first_of( '.' ) ).c_str() ) ;

  size_t mb = str_version.find_first_of( '.' ) + 1; size_t me = str_version.find( '.', mb );
  unsigned long minor     = atol( str_version.substr( mb, me-mb ).c_str() );

  unsigned long revision  = atol( str_version.substr( str_version.find_last_of( '.' ) + 1 ).c_str() );

  m_properties->setServerRevision( major, minor, revision );
#endif

  // Switch off auto-commit
  if( ! sesconf.autocommit( 0 ) )
  {
    coral::MySQLAccess::ErrorHandler errorHandler;
    MYSQL_HANDLE_ERR(errorHandler,handle,"Disabling autocommit");
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log << coral::Error << errorHandler.message() << std::endl << coral::MessageStream::endmsg;
  }

  // Set session transaction isolation level to be compatible with Oracle
  if( ! sesconf.txIsolationLevel( ReadCommitted ) )
  {
    coral::MySQLAccess::ErrorHandler errorHandler;
    MYSQL_HANDLE_ERR(errorHandler,handle,"Setting session transaction isolation level");
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log << coral::Error << errorHandler.message() << std::endl << coral::MessageStream::endmsg;
  }

  // Record the beginning of the session
  if ( m_properties->monitoringService() )
  {
    m_properties->monitoringService()->record( m_properties->connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.sessionBegin() );
  }
}

bool coral::MySQLAccess::Session::isUserSessionActive() const
{
  return m_properties->hasUserSessionStarted();
}

void coral::MySQLAccess::Session::endUserSession()
{
  // If a transaction is active, do a rollback before locking the mutex
  // (rollback also locks a mutex, resulting in a hang? see bug #72979)
  if ( m_transaction->isActive() ) m_transaction->rollback();

  // Lock the mutex here (FIXME: lock design does not seem very consistent)
  boost::mutex::scoped_lock lock( m_lock );
  if ( m_properties->hasUserSessionStarted() )
  {
    //if( m_transaction->isActive() ) m_transaction->rollback(); // bug #72979?
    m_properties->setUserSession( false );
    if ( m_properties->monitoringService() )
    {
      m_properties->monitoringService()->record( m_properties->connectionString(),
                                                 coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.sessionEnd() );
    }
  }
}

coral::ITransaction& coral::MySQLAccess::Session::transaction()
{
  return *m_transaction;
}

coral::ISchema& coral::MySQLAccess::Session::nominalSchema()
{
  if ( ! this->isUserSessionActive() )
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name() );
  return *m_schema;
}

coral::ISchema& coral::MySQLAccess::Session::schema( const std::string& schemaName )
{
  if ( ! m_properties->isConnected() )
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name() );

  if( schemaName == m_nominalSchemaName )
    return this->nominalSchema();

  coral::MySQLAccess::Schema* schema = 0;

  SchemaRegistry::iterator schri = m_schreg.find( schemaName );

  if( schri != m_schreg.end() )
    return *((*schri).second);

  // Check in the database otherwise
  SchemaLoader sl( *m_properties );

  if( sl.schemaExists( schemaName ) )
  {
    schema = new coral::MySQLAccess::Schema( *m_properties, schemaName );
    m_schreg.insert( std::make_pair( schemaName, schema ) );
  }
  else
  {
    throw coral::InvalidSchemaNameException( m_properties->domainProperties().service()->name() );
  }

  return *schema;
}
