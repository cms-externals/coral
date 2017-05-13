#ifndef TESTBASE_H
#define TESTBASE_H

#include <iostream>
#include <string>
#include "CoralKernel/Context.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/IConnection.h"
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
                            coral::AccessMode );

};

//-----------------------------------------------------------------------------

inline TestBase::TestBase()
{
  coral::Context& ctx = coral::Context::instance();
  ctx.loadComponent( "CORAL/RelationalPlugins/mysql" );
  const char* authvar = getenv( "CORAL_AUTH_PATH" );
  if( authvar )
  {
    ctx.loadComponent( "CORAL/Services/XMLAuthenticationService" );
  }
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
                   coral::AccessMode mode )
{
  coral::Context& ctx = coral::Context::instance();
  coral::IHandle<coral::IRelationalDomain> domain = ctx.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/mysql" );
  if ( ! domain.isValid() )
  {
    throw coral::NonExistingDomainException( "mysql" );
  }
  std::cout << "Retrieved relational domain \"" << domain->flavorName()
            << "\" implementation \"" << domain->implementationName()
            << "\" version " << domain->implementationVersion() << std::endl;
  std::pair<std::string,std::string> cs = domain->decodeUserConnectionString( connectionString );
  std::cout << "Connecting to: " << cs.first << " ing schema: " << cs.second << std::endl;
  coral::IConnection* connection = domain->newConnection( cs.first );
  // FIXME !!! call in try..catch..block
  coral::ISession* session = connection->newSession( cs.second /* the scheman name */, mode );
  if ( session ) {
    session->startUserSession( userName, password );
  }
  return session;
}

//-----------------------------------------------------------------------------

#endif
