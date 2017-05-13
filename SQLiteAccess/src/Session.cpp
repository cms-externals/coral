#include <cstdlib>
#include <iostream>
#include <string>
#include "sqlite3.h"

#include "CoralBase/MessageStream.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/SearchPath.h"
#include "CoralCommon/URIParser.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/MonitoringException.h"
#include "RelationalAccess/SessionException.h"

#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "MonitorController.h"
#include "Schema.h"
#include "Session.h"
#include "SessionProperties.h"
#include "Transaction.h"

#ifdef WIN32
#include <io.h>
namespace coral
{
  bool isReadonly( const std::string& fName )
  {
    return( ::_access( fName.c_str(), 02 ) != -1 );
  }
}
#else // U**x like
#include <unistd.h>
namespace coral
{
  bool isReadonly( const std::string& fName )
  {
    return( ::access( fName.c_str(), R_OK ) != -1 );
  }
}
#endif

coral::SQLiteAccess::Session::Session( coral::IDevConnection& connection,
                                       boost::shared_ptr<ConnectionProperties> connectionProperties,
                                       const std::string& /* schemaName */,
                                       coral::AccessMode mode )
  : coral::IDevSession( connection )
  , m_transaction(0)
  , m_schema(0)
{
  //std::cout << "Create Session " << this << std::endl; // debug bug #79983
  m_properties.reset( new coral::SQLiteAccess::SessionProperties(connectionProperties,*this , mode == coral::ReadOnly, connectionProperties->mutex() ));
  m_monitorController = new coral::SQLiteAccess::MonitorController( m_properties );
}


coral::SQLiteAccess::Session::~Session()
{
  //std::cout << "Delete Session " << this << std::endl; // debug bug #79983
  if ( this->isUserSessionActive() ) this->endUserSession();
  delete m_monitorController;
  if ( m_schema )
  {
    delete m_schema;
    m_schema = 0;
  }
  m_properties->invalidateSession(); // fix bug #73834!
  m_properties.reset();
}


coral::IMonitoring&
coral::SQLiteAccess::Session::monitoring()
{
  return *m_monitorController;
}


void
coral::SQLiteAccess::Session::startUserSession( const std::string& /* userName */,
                                                const std::string& /* password */ )
{
  if ( this->isUserSessionActive() ) return;
  //  if ( m_properties->isReadOnly() && !m_properties->connectionProperties().databaseFileName().isReadable() )
  if ( m_properties->isReadOnly() && !coral::isReadonly( m_properties->connectionProperties()->databaseFileName().string() ) )
  {
    throw coral::AuthenticationFailureException( m_properties->domainProperties().service()->name() );
  }
  int rstat=-1;
  //lock the sqlite3 methods
  m_properties->mutex()->lock();
  //std::cout << "Open sqlite3 handle" << std::endl; // debug bug #79983
  sqlite3* dbHandle;
#if SQLITE_VERSION_NUMBER < 3006000
  rstat=sqlite3_open(m_properties->connectionProperties()->databaseFileName().string().c_str(), &dbHandle);
#else
  if( m_properties->isReadOnly() )
  {
#ifndef _WIN32
    rstat=sqlite3_open_v2(m_properties->connectionProperties()->databaseFileName().string().c_str(),&dbHandle,SQLITE_OPEN_READONLY,"unix-none");
#else
    rstat=sqlite3_open_v2(m_properties->connectionProperties()->databaseFileName().string().c_str(),&dbHandle,SQLITE_OPEN_READONLY,NULL);
#endif
  }
  else
  {
    rstat=sqlite3_open_v2(m_properties->connectionProperties()->databaseFileName().string().c_str(),&dbHandle,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,NULL);
  }
#endif
  m_properties->setDbHandle( dbHandle );
  m_properties->mutex()->unlock();
  if( rstat != SQLITE_OK )
  {
    this->endUserSession();
    throw coral::SessionException( "Could not open file "+std::string(m_properties->connectionProperties()->databaseFileName().string()), "Session::startUserSession", m_properties->domainProperties().service()->name() );
  }
  // Set pragma for strict foreign keys
  if( getenv ( "CORAL_SQLITE_FOREIGN_KEYS_ON" ) )
  {
    std::stringstream cmd;
    cmd << "PRAGMA foreign_keys=ON";
    //lock the sqlite3 methods
    m_properties->mutex()->lock();
    sqlite3_exec( m_properties->dbHandle(), cmd.str().c_str(), 0, 0, 0 );
    m_properties->mutex()->unlock();
  }
  if ( getenv ( "CORAL_SQLITE_TEMP_STORE_MEMORY" ) )
  {
    std::stringstream cmd;
    cmd << "PRAGMA temp_store=MEMORY";
    //std::cout << cmd.str() << std::endl;
    //lock the sqlite3 methods
    m_properties->mutex()->lock();
    sqlite3_exec( m_properties->dbHandle(), cmd.str().c_str(), 0, 0, 0 );
    m_properties->mutex()->unlock();
  }
  else if ( getenv ( "CORAL_SQLITE_TEMP_STORE_DIRECTORY" ) )
  {
    std::stringstream cmd;
    cmd << "PRAGMA temp_store_directory='"
        << getenv ( "CORAL_SQLITE_TEMP_STORE_DIRECTORY" ) << "'";
    //std::cout << cmd.str() << std::endl;
    //lock the sqlite3 methods
    m_properties->mutex()->lock();
    sqlite3_exec( m_properties->dbHandle(), cmd.str().c_str(), 0, 0, 0 );
    m_properties->mutex()->unlock();
  }
  // Enforce foreign key constraints (from SQLite 3.6.19 onwards)
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
#if SQLITE_VERSION_NUMBER < 3006019
  log << Verbose << "Foreign key constraints are NOT supported"
      << " (SQLITE_VERSION=" << SQLITE_VERSION << ")"
      << coral::MessageStream::endmsg;
#else
  log << Verbose << "Foreign key constraints are supported"
      << "but will NOT be enforced (SQLITE_VERSION=" << SQLITE_VERSION << ")"
      << coral::MessageStream::endmsg;
  /*
  log << Verbose << "Foreign key constraints are supported"
      << "and will be enforced (SQLITE_VERSION=" << SQLITE_VERSION << ")"
      << coral::MessageStream::endmsg;
  {
    std::stringstream cmd;
    cmd << "PRAGMA foreign_keys = ON";
    //std::cout << cmd.str() << std::endl;
    //lock the sqlite3 methods
    lockMutex( m_properties->mutex() );
    sqlite3_exec( m_properties->dbHandle(), cmd.str().c_str(), 0, 0, 0 );
    unlockMutex( m_properties->mutex() );
  }
  */
#endif
  // Create new schema and transaction objects.
  m_schema = new coral::SQLiteAccess::Schema( m_properties );
  m_transaction = new coral::SQLiteAccess::Transaction( m_properties );
  // Record the beginning of the session
  if ( m_properties->monitoringService() )
  {
    try
    {
      m_properties->monitoringService()->record( m_properties->connectionString(),
                                                 coral::monitor::Session,
                                                 coral::monitor::Info,
                                                 monitoringEventDescription.sessionBegin() );
    }
    catch( const coral::MonitoringException& e )
    {
      log << coral::Error << "Caught coral::MonitoringException in Session::startUserSession" << e.what() << coral::MessageStream::endmsg;
    }
  }
  return;
}


void
coral::SQLiteAccess::Session::endUserSession()
{
  if ( this->isUserSessionActive() )
  {
    if ( m_transaction )
    {
      // Rollback any active tx (ONLY if m_transaction != 0 : fix bug #62946)
      if ( m_transaction->isActive() ) m_transaction->rollback();
      delete m_transaction;
      m_transaction = 0;
    }
    if ( m_schema )
    {
      delete m_schema;
      m_schema = 0;
    }
    if ( m_properties->monitoringService() )
    {
      m_properties->monitoringService()->record( m_properties->connectionString(),
                                                 coral::monitor::Session,
                                                 coral::monitor::Info,
                                                 monitoringEventDescription.sessionEnd() );
    }
    //lock the sqlite3 methods
    m_properties->mutex()->lock();
    //std::cout << "Close sqlite3 handle" << std::endl; // debug bug #79983
    sqlite3_close( m_properties->dbHandle() );
    m_properties->setDbHandle( 0 );
    m_properties->mutex()->unlock();
  }
}


bool
coral::SQLiteAccess::Session::isUserSessionActive() const
{
  return ( m_properties->dbHandle() != 0 );
}


coral::ITransaction&
coral::SQLiteAccess::Session::transaction()
{
  if ( ! this->isUserSessionActive() ) {
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name(), "ISession::transaction" );
  }
  return *m_transaction;
}


coral::ISchema&
coral::SQLiteAccess::Session::nominalSchema()
{
  if ( ! this->isUserSessionActive() )
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name() );
  return *m_schema;
}


coral::ISchema&
coral::SQLiteAccess::Session::schema( const std::string& /* schemaName */)
{
  if ( ! this->isUserSessionActive() )
    throw coral::ConnectionNotActiveException( m_properties->domainProperties().service()->name() );
  return *m_schema;
}
