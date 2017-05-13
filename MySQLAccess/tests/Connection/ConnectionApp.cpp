#include "ConnectionApp.h"

#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/AccessMode.h"

#include "CoralKernel/Context.h"

#include "../../src/TypeConverter.h"

#include <iostream>
#include <stdexcept>
#include <set>
#include <vector>

ConnectionApp::ConnectionApp( const std::string& connectionString, const std::string& userName, const std::string& password )
  : m_connectionString( connectionString ), m_userName( userName ), m_password( password ) {
}

ConnectionApp::~ConnectionApp() {
}

void ConnectionApp::run()
{
  // Prepare LCG CORAL run-time
  coral::Context& ctx = coral::Context::instance();

  ctx.loadComponent( "CORAL/RelationalPlugins/mysql" );
  const char* authvar = getenv( "CORAL_AUTH_PATH" );

  if( authvar )
    ctx.loadComponent( "CORAL/Services/XMLAuthenticationService" );

  ctx.loadComponent( "CORAL/Services/XMLLookupService" );

  // Initialize CORAL connection

  coral::IHandle<coral::IRelationalDomain> domain = ctx.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/mysql" );

  if ( ! domain.isValid() )
  {
    throw coral::NonExistingDomainException( "mysql" );
  }

  std::cout << "Retrieved relational domain \"" << domain->flavorName() << "\" implementation \"" << domain->implementationName() << "\" version " << domain->implementationVersion() << std::endl;

  std::pair<std::string,std::string> cs = domain->decodeUserConnectionString( m_connectionString );

  coral::IConnection* connection = domain->newConnection( cs.first );

  // FIXME !!! call in try..catch..block
  coral::ISession* session = connection->newSession( cs.second /* the scheman name */, coral::ReadOnly );

  if ( session )
  {
    session->startUserSession( m_userName, m_password );
  }

  if ( ! session->isUserSessionActive() )
  {
    throw std::runtime_error( "Connection lost..." );
  }

  std::cout << "Connected to a server with version " << connection->serverVersion() << std::endl;

  std::cout << "Supported C++ types: " << std::endl;
  std::set<std::string> cppTypes = connection->typeConverter().supportedCppTypes();
  for ( std::set<std::string>::const_iterator iType = cppTypes.begin(); iType != cppTypes.end(); ++iType )
  {
    std::cout << *iType << std::endl;
    std::cout << "                      -> "
              << connection->typeConverter().sqlTypeForCppType( *iType ) << std::endl;
  }

  try
  {
    // The tested use cases
    std::vector<std::string> testSql;
    testSql.push_back( "DECIMAL(10,5)" ); // should be float
    testSql.push_back( "DECIMAL(11,5)" ); // should be float
    testSql.push_back( "CHAR" ); // should be signed char
    testSql.push_back( "CHAR(1)" ); // should be signed char
    testSql.push_back( "TINYINT(4)" ); // should be signed char
    testSql.push_back( "TINYINT(3)" ); // should be unsigned char
    testSql.push_back( "INT(11)" );
    testSql.push_back( "INTEGER(10)" ); // should be int
    testSql.push_back( "INT(10)" ); // should be int
    testSql.push_back( "float" );
    testSql.push_back( "varchar(255)" );
    testSql.push_back( "varchar(123)" );
    testSql.push_back( "double" );
    testSql.push_back( "tinyint(1)" );
    testSql.push_back( "date" );
    testSql.push_back( "BLOB" );
    testSql.push_back( "blob" );
    testSql.push_back( "MEDIUMBLOB" );
    testSql.push_back( "mediumblob" );
    testSql.push_back( "TINYBLOB" );
    testSql.push_back( "tinyblob" );
    testSql.push_back( "LONGBLOB" );
    testSql.push_back( "longblob" );

    const coral::MySQLAccess::TypeConverter& tc = dynamic_cast<const coral::MySQLAccess::TypeConverter&>( connection->typeConverter() );

    for( std::vector<std::string>::const_iterator sti = testSql.begin(); sti != testSql.end(); ++sti )
    {
      std::string sqlType( *sti ); std::transform(sqlType.begin(), sqlType.end(), sqlType.begin(), (int (*)(int))toupper);

      std::cout << "Checking SQL type " << (*sti) << std::endl;
      if( tc.deduceCppType( sqlType ) )
        std::cout << "Recognized mapping from SQL type " << (*sti) << " to C++ type " << tc.cppTypeForSqlType( sqlType ) << std::endl;
      else
        std::cout << "Failed to recognize mapping for SQL type " << (*sti) << std::endl;
    }
  }
  catch( const std::exception& e )
  {
    std::cerr << "Caught standard exception: " << e.what() << std::endl;
  }

  session->transaction().start();

  session->transaction().commit();

  session->transaction().start( true );

  session->transaction().commit();

  session->endUserSession();

  delete session;

  connection->disconnect();

  if ( connection->isConnected() )
  {
    throw std::runtime_error( "Connection appears to be up and running !!!" );
  }

  connection->connect();

  session = connection->newSession( cs.second, coral::ReadOnly );

  session->startUserSession( m_userName, m_password );

  if ( ! connection->isConnected( true ) )
  {
    throw std::runtime_error( "Connection lost..." );
  }

  session->transaction().start();

  std::set<std::string> nlt = session->nominalSchema().listTables();

  std::cout << "Nominal schema tables: " << std::endl;

  for( std::set<std::string>::iterator ssi = nlt.begin(); ssi != nlt.end(); ++ssi )
    std::cout << (*ssi) << std::endl;

  std::set<std::string> slt = session->schema( "test" ).listTables();

  std::cout << "\"test\" schema tables: " << std::endl;

  for( std::set<std::string>::iterator ssi = slt.begin(); ssi != slt.end(); ++ssi )
    std::cout << (*ssi) << std::endl;

  session->transaction().commit();

  session->endUserSession();

  session->startUserSession( m_userName, m_password );

  session->transaction().start();

  slt = session->schema( "test" ).listTables();

  std::cout << "\"test\" schema tables: " << std::endl;

  for( std::set<std::string>::iterator ssi = slt.begin(); ssi != slt.end(); ++ssi )
    std::cout << (*ssi) << std::endl;

  session->transaction().commit();

  session->endUserSession();

  connection->disconnect();

  if ( connection->isConnected() )
  {
    throw std::runtime_error( "Connection appears to be up and running !!!" );
  }

  delete session;
  delete connection;
}
