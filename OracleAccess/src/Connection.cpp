#include <cstring> // fix bug #58581
#include <iostream>
#include <sstream>
#include "oci.h"

#include "CoralBase/boost_datetime_headers.h"
#include "CoralBase/MessageStream.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/Utilities.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SessionException.h"

#include "Connection.h"
#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "OracleErrorHandler.h"
#include "Session.h"

coral::OracleAccess::Connection::Connection( const coral::OracleAccess::DomainProperties& domainProperties,
                                             const std::string& connectionString ) :
  m_properties(), // fix Windows warning C4355 ('this' in initializer list)
  m_serverVersion( "" ),
  m_mutex()
{
  //std::cout << "Create Connection " << this << std::endl; // debug bug #73334
  m_properties.reset( new ConnectionProperties( domainProperties, connectionString, *this ) ); // fix Windows warning C4355 ('this' in initializer list)
}


coral::OracleAccess::Connection::~Connection()
{
  //std::cout << "Delete Connection " << this << std::endl; // debug bug #73334
  if ( this->isConnected() ) this->disconnect();
  m_properties->invalidateConnection(); // see bug #79883 and bug #73834...
  m_properties.reset();
}


coral::ISession*
coral::OracleAccess::Connection::newSession( const std::string& schemaName,
                                             coral::AccessMode mode ) const
{
  if ( ! const_cast<coral::OracleAccess::Connection*>( this )->isConnected() )
    throw coral::ConnectionNotActiveException( m_properties->domainServiceName(),
                                               "IConnection::newSession" );

  boost::mutex::scoped_lock lock( m_mutex );
  return new coral::OracleAccess::Session( *( static_cast<coral::IDevConnection*>( const_cast<coral::OracleAccess::Connection*>( this ) ) ),
                                           m_properties,
                                           schemaName,
                                           mode );
}


void
coral::OracleAccess::Connection::connect()
{
  if ( this->isConnected() ) {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Warning << "A connection is already present for \""
        << m_properties->connectionString() << "\"" << coral::MessageStream::endmsg;
    return;
  }

  boost::mutex::scoped_lock lock( m_mutex );

  const std::string database = m_properties->connectionString();

  // Create the environment
  ub4 mode = OCI_OBJECT | OCI_THREADED; // 0x00000002 | 0x00000001
  static bool first = true;
  if ( first )
  {
    first = false;
    coral::MessageStream log( m_properties->domainServiceName() );
    if ( getenv( "CORAL_ORA_NO_OCI_THREADED" ) )
    {
      mode = OCI_OBJECT; // 0x00000002
      log << coral::Warning
          << "Env CORAL_ORA_NO_OCI_THREADED was specified: "
          << "OCIEnvCreate will use OCI_OBJECT "
          << "instead of OCI_OBJECT | OCI_THREADED" << coral::MessageStream::endmsg;
    }
    else if ( getenv( "CORAL_ORA_OCI_NO_MUTEX" )  )
    {
      mode = OCI_OBJECT | OCI_THREADED | OCI_NO_MUTEX; // 0x00000002 | 0x00000001 | 0x00000080
      log << coral::Warning
          << "Env CORAL_ORA_OCI_NO_MUTEX was specified: "
          << "OCIEnvCreate will use OCI_OBJECT | OCI_THREADED | OCI_NO_MUTEX "
          << "instead of OCI_OBJECT | OCI_THREADED" << coral::MessageStream::endmsg;
    }
  }
  OCIEnv* ociEnvHandle = 0;
#ifdef linux
  static unsigned int ociDebug = 0;
  if ( ociDebug < 2 && !getenv( "CORAL_ORA_OCI_DEBUG_OCIENVCREATE2" ) && getenv( "CORAL_ORA_OCI_DEBUG_OCIENVCREATE" ) )
  {
    std::stringstream cmd;
    cmd << "strace -p " << getpid() << " 2>&1 &";
    std::cout << "***** OracleAccess CORAL_ORA_OCI_DEBUG_OCIENVCREATE is set  *****" << std::endl;
    if ( getenv( "LD_LIBRARY_PATH" ) ) std::cout << "LD_LIBRARY_PATH = " << std::string( getenv( "LD_LIBRARY_PATH" ) ) << std::endl;
    std::cout << "***** OracleAccess DEBUG: strace OCIEnvCreate calls (START) *****" << std::endl;
    system( cmd.str().c_str() );
    // Sleep 1 second (else OCIEnvCreate may be executed before strace is attached)
    coral::sleepSeconds( 1 );
  }
#endif
  sword status = OCIEnvCreate( &ociEnvHandle, mode, 0,0,0,0,0,0 );
#ifdef linux
  if ( ( ociDebug < 1 && getenv( "CORAL_ORA_OCI_DEBUG_OCIENVCREATE2" ) ) ||
       ( ociDebug < 2 && getenv( "CORAL_ORA_OCI_DEBUG_OCIENVCREATE" ) ) )
  {
    ociDebug++;
    std::stringstream cmd;
    //cmd << "killall strace";
    cmd << "kill `ps -C strace -f | grep 'strace -p " << getpid() << "' | awk '{print $2}'` &> /dev/null";
    system( cmd.str().c_str() );
    std::cout << "***** OracleAccess DEBUG: strace OCIEnvCreate calls (END) *****" << std::endl;
    if ( getenv( "LD_LIBRARY_PATH" ) ) std::cout << "LD_LIBRARY_PATH = " << std::string( getenv( "LD_LIBRARY_PATH" ) ) << std::endl;
  }
#endif
  // First failure - try again (workaround for bug #31554)
  if ( status != OCI_SUCCESS )
  {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Error
        << "Could not allocate an OCI environment handle: status=" << status
        << ", handle=" << ociEnvHandle << coral::MessageStream::endmsg;
    if ( status == OCI_SUCCESS_WITH_INFO || status == OCI_ERROR )
    {
      if ( status == OCI_SUCCESS_WITH_INFO )
        log << coral::Error << "OCIEnvCreate status=OCI_SUCCESS_WITH_INFO" << coral::MessageStream::endmsg;
      else if ( status == OCI_ERROR )
        log << coral::Error << "OCIEnvCreate status=OCI_ERROR" << coral::MessageStream::endmsg;
    }
    log << coral::Error << "Try a second time" << coral::MessageStream::endmsg;
    ociEnvHandle = 0;
    status = OCIEnvCreate( &ociEnvHandle, mode, 0,0,0,0,0,0 );
  }
  // Second failure - give up
  if ( status != OCI_SUCCESS )
  {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Error
        << "Could not allocate an OCI environment handle: status=" << status
        << ", handle=" << ociEnvHandle << coral::MessageStream::endmsg;
    if ( status == OCI_SUCCESS_WITH_INFO || status == OCI_ERROR )
    {
      if ( status == OCI_SUCCESS_WITH_INFO )
        log << coral::Error << "OCIEnvCreate status=OCI_SUCCESS_WITH_INFO" << coral::MessageStream::endmsg;
      else if ( status == OCI_ERROR )
        log << coral::Error << "OCIEnvCreate status=OCI_ERROR" << coral::MessageStream::endmsg;
    }
    throw coral::ServerException( m_properties->domainServiceName(),
                                  "Could not allocate an OCI environment handle" );
  }

  // Creating the error handle
  void* temporaryPointer = 0;
  status = OCIHandleAlloc( ociEnvHandle, &temporaryPointer,
                           OCI_HTYPE_ERROR, 0, 0 );
  if ( status != OCI_SUCCESS ) {
    OCIHandleFree( ociEnvHandle, OCI_HTYPE_ENV );
    throw coral::ServerException( m_properties->domainServiceName(),
                                  "Could not allocate an OCI error handle" );
  }
  OCIError* ociErrorHandle = static_cast< OCIError* >( temporaryPointer );
  //std::cout << "OCIError created " << this << std::endl; // bug #83601

  // Creating a server handle
  temporaryPointer = 0;
  status = OCIHandleAlloc( ociEnvHandle, &temporaryPointer,
                           OCI_HTYPE_SERVER, 0, 0 );
  if ( status != OCI_SUCCESS ) {
    OCIHandleFree( ociErrorHandle, OCI_HTYPE_ERROR );
    OCIHandleFree( ociEnvHandle, OCI_HTYPE_ENV );
    throw coral::ServerException( m_properties->domainServiceName(),
                                  "Could not allocate an OCI server handle" );
  }

  OCIServer* ociServerHandle = static_cast< OCIServer* >( temporaryPointer );

  // Creating an error handler to be used in the subsequent calls.
  coral::OracleAccess::OracleErrorHandler errorHandler( ociErrorHandle );

  // Attaching the server
  status = OCIServerAttach( ociServerHandle, ociErrorHandle,
                            reinterpret_cast<const text *>( database.c_str() ),
                            ::strlen( database.c_str() ),
                            OCI_DEFAULT );
  if ( status != OCI_SUCCESS ) {
    errorHandler.handleCase( status, "attaching a server '"+database+"'" );
    coral::MessageStream log( m_properties->domainServiceName() );
    if ( errorHandler.isError() ) {
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      OCIHandleFree( ociServerHandle, OCI_HTYPE_SERVER );
      OCIHandleFree( ociErrorHandle, OCI_HTYPE_ERROR );
      OCIHandleFree( ociEnvHandle, OCI_HTYPE_ENV );
      const sb4 lastErrorCode = errorHandler.lastErrorCode();
      if ( lastErrorCode == 12154 || lastErrorCode == 12538 ||
           lastErrorCode == 12545 || lastErrorCode == 12505) { // Wrong db specification
        throw coral::DatabaseNotAccessibleException( m_properties->domainServiceName(),
                                                     "IConnection::connect" );
      }
      if ( true ) { // Retry in these cases
        throw coral::ConnectionException( m_properties->domainServiceName(),
                                          "IConnection::connect" );
      }
      else { // Do not retry
        throw coral::ServerException( m_properties->domainServiceName() );
      }
    }
    else {
      log << coral::Warning << errorHandler.message() << coral::MessageStream::endmsg;
    }
  }


  // Debug
  {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Verbose << "New connection started with OCIServerAttach"
        << " (OCIServer*=" << ociServerHandle << ")" << coral::MessageStream::endmsg;
  }

  // Retrieving the server version
  text serverVersion[1000];
  status = OCIServerVersion( ociServerHandle, ociErrorHandle, serverVersion, 1000, OCI_HTYPE_SERVER );
  if ( status != OCI_SUCCESS ) {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Error << "Failed to retrieve the server version" << coral::MessageStream::endmsg;
    OCIServerDetach( ociServerHandle, ociErrorHandle, OCI_DEFAULT );
    OCIHandleFree( ociServerHandle, OCI_HTYPE_SERVER );
    OCIHandleFree( ociErrorHandle, OCI_HTYPE_ERROR );
    OCIHandleFree( ociEnvHandle, OCI_HTYPE_ENV );
    throw coral::ConnectionException( m_properties->domainServiceName(),
                                      "ISession::connect",
                                      "Could not retrieve the Oracle server version" );
  }
  std::ostringstream os;
  os << serverVersion;
  int majorVersionNumber = this->parseVersionString( os.str() );

  m_properties->setHandles( ociEnvHandle, ociErrorHandle, ociServerHandle, majorVersionNumber );

  //  // Record the beginning of the connection using the Monitoring service
  //  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = coral::Context::instance().query<coral::monitor::IMonitoringService>();
  //
  //  if ( monitoringService.isValid() )
  //  {
  //    monitoringService->setLevel( "oracle://" + m_properties->connectionString(), coral::monitor::Default );
  //    monitoringService->enable( "oracle://" + m_properties->connectionString() );
  //    monitoringService->record( "oracle://" + m_properties->connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.connectionBegin() );
  //  }

  // Test bug #58522 (aka bug #65709, bug #75596)
  // Most likely ORA-24327 happens if the glitch is anywhere between
  // OCIServerAttach (init Connection) and OCISessionBegin (init Session).
  if ( getenv( "CORAL_ORA_TEST_ORA24327_SLEEP10S" ) )
  {
    std::string time1 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__Connection::connect() @"
              << time1 << " sleep 10s (test ORA-24327)" << std::endl;
    coral::sleepSeconds(10);
    std::string time2 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__Connection::connect() @"
              << time2 << " slept 10s (test ORA-24327)" << std::endl;
  }
}

bool
coral::OracleAccess::Connection::isConnected( bool probePhysicalConnection )
{
  boost::mutex::scoped_lock lock( m_mutex );

  bool logicalConnection = ( m_properties->ociEnvHandle() != 0 );
  if ( ! logicalConnection ) return false;
  if ( probePhysicalConnection ) {
    text serverVersion[1000];
    sword status = OCIServerVersion( m_properties->ociServerHandle(), m_properties->ociErrorHandle(),
                                     serverVersion, 1000, OCI_HTYPE_SERVER );
    if ( status != OCI_SUCCESS ) {
      this->__disconnect( true );
      logicalConnection = false;
    }
  }

  return logicalConnection;
}


void
coral::OracleAccess::Connection::disconnect()
{
  if ( this->isConnected() ) {
    boost::mutex::scoped_lock lock( m_mutex );
    this->__disconnect();
  }
}


void
coral::OracleAccess::Connection::__disconnect( bool lostConnection )
{
  if ( lostConnection )
  {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Error << "The connection has been lost!?" << coral::MessageStream::endmsg;
  }

  // Clean up all the active sessions !!!
  this->invalidateAllSessions();

  // Debug
  {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Verbose << "End connection with OCIServerDetach"
        << " (OCIServer*=" << m_properties->ociServerHandle() << ")"
        << coral::MessageStream::endmsg;
  }

  // Close all handles.
  OCIEnv* ociEnvHandle = m_properties->ociEnvHandle();
  OCIError* ociErrorHandle = m_properties->ociErrorHandle();
  OCIServer* ociServerHandle = m_properties->ociServerHandle();
  OCIServerDetach( ociServerHandle, ociErrorHandle, OCI_DEFAULT );
  if ( ! lostConnection ) { // THIS IS NECESSARY OTHERWISE OCI WILL DOUBLE FREE THE HANDLE!!!
    OCIHandleFree( ociServerHandle, OCI_HTYPE_SERVER );
  }
  //std::cout << "OCIError deleted " << this << std::endl; // bug #83601
  OCIHandleFree( ociErrorHandle, OCI_HTYPE_ERROR );
  OCIHandleFree( ociEnvHandle, OCI_HTYPE_ENV );

  //  // Record the ending of the connection
  //  coral::IHandle<coral::monitor::IMonitoringService> monitoringService = coral::Context::instance().query<coral::monitor::IMonitoringService>();
  //
  //  if ( monitoringService.isValid() )
  //  {
  //    monitoringService->record( "oracle://" + m_properties->connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.connectionEnd() );
  //    monitoringService->disable( "oracle://" + m_properties->connectionString() );
  //  }

  m_properties->setHandles( 0, 0, 0, false );
}


std::string
coral::OracleAccess::Connection::serverVersion() const
{
  if ( ! const_cast<coral::OracleAccess::Connection*>( this )->isConnected() )
    throw coral::ConnectionNotActiveException( m_properties->domainServiceName(),
                                               "IConnection::serverVersion" );
  boost::mutex::scoped_lock lock( m_mutex );
  return m_serverVersion;
}


coral::ITypeConverter&
coral::OracleAccess::Connection::typeConverter()
{
  if ( ! this->isConnected() )
    throw coral::ConnectionNotActiveException( m_properties->domainServiceName(),
                                               "Connection::typeConverter" );
  boost::mutex::scoped_lock lock( m_mutex );
  return m_properties->typeConverter();
}


int
coral::OracleAccess::Connection::parseVersionString( const std::string& versionString )
{
  // Trying to find a number of the format "i1.i2.i3.i4.i5"
  std::istringstream is( versionString.c_str() );
  while ( ! is.eof() ) {
    std::string result;
    is >> result >> std::ws;
    std::istringstream isnumbers( result.c_str() );
    int i1, i2, i3, i4, i5;
    char c = ' ';
    isnumbers >> i1 >> c;
    if ( c != '.' ) continue;
    c = ' ';
    isnumbers >> i2 >> c;
    if ( c != '.' ) continue;
    c = ' ';
    isnumbers >> i3 >> c;
    if ( c != '.' ) continue;
    c = ' ';
    isnumbers >> i4 >> c;
    if ( c != '.' ) continue;
    isnumbers >> i5;
    std::ostringstream os;
    os << i1 << c << i2 << c << i3 << c << i4 << c << i5;
    m_serverVersion = os.str();
    return i1;
  }
  return 0;
}
