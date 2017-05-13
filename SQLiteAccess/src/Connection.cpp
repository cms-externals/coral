#include <iostream>
#include "sqlite3.h"

#include "CoralBase/MessageStream.h"
#include "CoralCommon/IDevConnectionServiceConfiguration.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/SearchPath.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SessionException.h"

#include "Connection.h"
#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "Session.h"

#ifdef WIN32
#include <io.h>
namespace coral
{
  bool isWritable( const std::string& fName )
  {
    return( ::_access( fName.c_str(), 04 ) != -1 );
  }
}
#else // U**x like
#include <unistd.h>
namespace coral
{
  bool isWritable( const std::string& fName )
  {
    return( ::access( fName.c_str(), W_OK ) != -1 );
  }
}
#endif


coral::SQLiteAccess::Connection::Connection( const coral::SQLiteAccess::DomainProperties& domainProperties,
                                             const std::string& connectionString)
//: m_properties( new ConnectionProperties( domainProperties, *this, connectionString ) )
  : m_properties( new ConnectionProperties( domainProperties, connectionString ) )
  , m_isConnected( false )
  , m_serverVersion( SQLITE_VERSION )
{
}


coral::SQLiteAccess::Connection::~Connection()
{
  if ( this->isConnected() ) this->disconnect();
  //m_properties->invalidateConnection(); // see bug #79883 and bug #73834...
  m_properties.reset();
}


coral::ISession*
coral::SQLiteAccess::Connection::newSession( const std::string& schemaName,
                                             coral::AccessMode mode ) const
{
  if ( ! const_cast<coral::SQLiteAccess::Connection*>( this )->isConnected() )
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name(), "IConnection::newSession" );
  //m_properties->setReadOnly( ( mode == coral::Update ) ? false : true );
  //if ( m_properties->isReadOnly() && !m_inFileName.isReadable() ){
  if ( mode==coral::ReadOnly && !boost::filesystem::exists(m_properties->databaseFileName()) )
  {
    const_cast<coral::SQLiteAccess::Connection*>(this)->disconnect();
    throw coral::DatabaseNotAccessibleException(std::string(" cannot open read-only session to non-existing ")+m_properties->databaseFileName().string(),"Connection::newSession",m_properties->domainProperties().service()->name() );
  }
  if( mode==coral::Update && !boost::filesystem::exists( m_properties->databaseFileName().parent_path() ) )
  {
    const_cast<coral::SQLiteAccess::Connection*>(this)->disconnect();
    throw coral::DatabaseNotAccessibleException(std::string("directory ")+std::string(m_properties->databaseFileName().parent_path().string())+std::string(" does not exist"), "Connection::newSession",m_properties->domainProperties().service()->name());
  }
  /*if(this->size()>0){
    //const_cast<coral::SQLiteAccess::Connection*>(this)->disconnect();
    throw coral::MaximumNumberOfSessionsException(m_properties->domainProperties().service()->name());
  }
  */
  return new coral::SQLiteAccess::Session( *( static_cast<coral::IDevConnection*>( const_cast<coral::SQLiteAccess::Connection*>( this ) ) ), m_properties,schemaName,mode );
}


void
coral::SQLiteAccess::Connection::connect()
{
  if ( this->isConnected() ) {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log << coral::Warning << "A connection is already present for \""
        << m_properties->connectionString() << "\"" << coral::MessageStream::endmsg;
  }
  /**
     throw coral::DatabaseNotAccessibleException if the file does not exist and the directory is unwritable
  */
  const std::string database = m_properties->connectionString();
  this->verifyFileName(database);
  if( !boost::filesystem::exists( m_properties->databaseFileName() ) && !coral::isWritable( m_properties->databaseFileName().parent_path().string() ) )
  {
    throw coral::DatabaseNotAccessibleException(m_properties->databaseFileName().parent_path().string()+std::string(" is not writable"),"Connection::connect",m_properties->domainProperties().service()->name() );
  }
  m_isConnected=true; //set logical connection

  // Debug
  {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log << coral::Verbose << "New connection started for \""
        << m_properties->connectionString() << "\""
        << coral::MessageStream::endmsg;
  }

  //  //Record the beginning of the connection using the Monitoring service
  //  coral::IHandle<coral::IConnectionService> connService = coral::Context::instance().query<coral::IConnectionService>();
  //  if(connService.isValid()){
  //    coral::monitor::IMonitoringService& monitoringService = dynamic_cast<coral::IDevConnectionServiceConfiguration&>(connService->configuration()).monitoringService();
  //    monitoringService.setLevel( m_properties->connectionString(), connService->configuration().monitoringLevel() );
  //    monitoringService.record( m_properties->connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.connectionBegin() );
  //  }
}


bool
coral::SQLiteAccess::Connection::isConnected( bool /* probePhysicalConnection */)
{
  return m_isConnected;
}


void
coral::SQLiteAccess::Connection::disconnect()
{
  if ( this->isConnected() ) {
    // Clean up all the active sessions !!!
    this->invalidateAllSessions(); ;
    // Record the ending of the connection
  }
  // Debug
  {
    coral::MessageStream log( m_properties->domainServiceName() );
    log << coral::Verbose << "End connection for \""
        << m_properties->connectionString() << "\""
        << coral::MessageStream::endmsg;
  }
  m_isConnected=false;

  //  // Record the ending of the connection
  //  coral::IHandle<coral::IConnectionService> connService = coral::Context::instance().query<coral::IConnectionService>();
  //  if(connService.isValid())
  //  {
  //    coral::monitor::IMonitoringService& monitoringService = dynamic_cast<coral::IDevConnectionServiceConfiguration&>(connService->configuration()).monitoringService();
  //    monitoringService.record( m_properties->connectionString(), coral::monitor::Session, coral::monitor::Info, monitoringEventDescription.connectionEnd() );
  //    monitoringService.disable( m_properties->connectionString() );
  //  }
}


std::string
coral::SQLiteAccess::Connection::serverVersion() const
{
  if ( ! const_cast<coral::SQLiteAccess::Connection*>( this )->isConnected() )
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name(), "IConnection::serverVersion" );
  return m_serverVersion;
}


coral::ITypeConverter&
coral::SQLiteAccess::Connection::typeConverter()
{
  if ( ! this->isConnected() )
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name(), "Connection::typeConverter" );
  return m_properties->typeConverter();
}


void
coral::SQLiteAccess::Connection::verifyFileName( const std::string& inFileName ) const
{
  const boost::filesystem::path fileNameFirstGuess( inFileName );
  if( fileNameFirstGuess.is_complete() )
  {
    //m_inFileName=fileNameFirstGuess;
    m_properties->setFileName(fileNameFirstGuess.string());
    return;
  }
  else if( fileNameFirstGuess.has_relative_path() )
  {
    boost::filesystem::path absolutePath( boost::filesystem::current_path()/fileNameFirstGuess );
    m_properties->setFileName(absolutePath.string());
    //m_inFileName=fileNameFirstGuess.expand(seal::Filename("."));
    return;
  }

  const char* thePathVariable = ::getenv( "SQLITE_FILE_PATH" );
  if( thePathVariable )
  {
    coral::SearchPath path( thePathVariable );
    for ( coral::SearchPath::const_iterator iDirectory = path.begin(); iDirectory != path.end(); ++iDirectory )
    {
      const boost::filesystem::path guess( *iDirectory/fileNameFirstGuess );
      m_properties->setFileName(guess.string());
      //m_inFileName=guess;
      return;
    }
  }

  m_properties->setFileName(fileNameFirstGuess.string());
  //m_inFileName=fileNameFirstGuess;
  return;
}
