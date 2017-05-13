#ifndef TESTBASE_H
#define TESTBASE_H

#include <stdexcept>
#include <string>
#include "CoralKernel/Context.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/RelationalServiceException.h"

//-----------------------------------------------------------------------------

class TestBase
{

public:

  // Constructor
  TestBase();

  // Destructor
  virtual ~TestBase();

  // The actual run method
  virtual void run() = 0;

protected:

  // Returns a session object
  coral::ISession* connect( const std::string& connectionString,
                            const std::string& userName,
                            const std::string& password,
                            coral::AccessMode mode=coral::ReadOnly );

  // Disconnect
  void disconnect();

  // Returns CORAL type converter
  coral::ITypeConverter& typeConverter() const;

  // Returns CORAL connection service handle
  coral::IHandle<coral::IConnectionService> connectionService();

private:

  coral::IConnection* m_connection;

};

//-----------------------------------------------------------------------------

inline TestBase::TestBase() : m_connection ( 0 )
{
  coral::Context& ctx = coral::Context::instance();
  ctx.loadComponent( "CORAL/RelationalPlugins/frontier" );
  ctx.loadComponent( "CORAL/RelationalPlugins/oracle" );
  if( getenv( "CORAL_AUTH_PATH" ) )
    ctx.loadComponent( "CORAL/Services/XMLAuthenticationService" );
  else
    ctx.loadComponent( "CORAL/Services/EnvironmentAuthenticationService" );
  ctx.loadComponent( "CORAL/Services/ConnectionService" );
}

//-----------------------------------------------------------------------------

inline TestBase::~TestBase()
{
}

//-----------------------------------------------------------------------------

inline coral::ISession*
TestBase::connect( const std::string& connectionString,
                   const std::string& userName,
                   const std::string& password,
                   coral::AccessMode mode  )
{
  coral::Context& ctx = coral::Context::instance();
  std::pair<std::string,std::string> cs;
  if( connectionString.find( "oracle" ) != std::string::npos )
  {
    coral::IHandle<coral::IRelationalDomain> iHandle = ctx.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/oracle" );
    if ( ! iHandle.isValid() )
    {
      throw coral::NonExistingDomainException( "oracle" );
    }
    cs = iHandle->decodeUserConnectionString( connectionString );
    m_connection = iHandle->newConnection( cs.first );
  }
  else if( connectionString.find( "frontier" ) != std::string::npos )
  {
    coral::IHandle<coral::IRelationalDomain> iHandle = ctx.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/frontier" );
    if ( ! iHandle.isValid() )
    {
      throw coral::NonExistingDomainException( "frontier" );
    }
    cs = iHandle->decodeUserConnectionString( connectionString );
    m_connection = iHandle->newConnection( cs.first );
  }
  m_connection->connect();
  coral::ISession* session = m_connection->newSession( cs.second, mode );
  if ( session )
  {
    session->startUserSession( userName, password );
  }
  return session;
}

//-----------------------------------------------------------------------------

inline void TestBase::disconnect()
{
  if( m_connection->isConnected() )
    m_connection->disconnect();
  delete m_connection;
  m_connection = 0;
}

//-----------------------------------------------------------------------------

inline coral::ITypeConverter& TestBase::typeConverter() const
{
  return m_connection->typeConverter();
}

//-----------------------------------------------------------------------------

inline coral::IHandle<coral::IConnectionService> TestBase::connectionService()
{
  coral::Context& ctx = coral::Context::instance();
  coral::IHandle<coral::IConnectionService> svc = ctx.query<coral::IConnectionService>();
  if( ! svc.isValid() )
  {
    throw std::runtime_error( "Could not locate the connection service" );
  }
  return svc;
}

//-----------------------------------------------------------------------------

#endif
