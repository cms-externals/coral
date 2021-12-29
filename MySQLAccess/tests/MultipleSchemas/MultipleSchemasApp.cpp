#include "MultipleSchemasApp.h"

#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/SessionException.h"

#include <iostream>
#include <stdexcept>
#include <memory>


MultipleSchemasApp::MultipleSchemasApp( const std::string& connectionString, const std::string& userName, const std::string& password, const std::string& otherSchema, coral::AccessMode mode )
  : TestBase(),
    m_connectionString( connectionString ),
    m_userName( userName ),
    m_password( password ),
    m_otherSchema( otherSchema ),
    m_accessMode( mode )
{
}

MultipleSchemasApp::~MultipleSchemasApp()
{
}

void MultipleSchemasApp::run()
{
  std::unique_ptr< coral::ISession> session( this->connect( m_connectionString, m_userName, m_password, m_accessMode ) );

  if ( ! session->isUserSessionActive() )
  {
    throw std::runtime_error( "Connection lost..." );
  }

  session->transaction().start( true );

  std::cout << "Tables in the nominal schema:" << std::endl;
  std::set< std::string > listOfTables = session->nominalSchema().listTables();
  for ( std::set< std::string >::const_iterator iTable = listOfTables.begin();
        iTable != listOfTables.end(); ++iTable ) std::cout << *iTable << std::endl;

  std::cout << "Tables in the other schema:" << std::endl;
  listOfTables = session->schema( m_otherSchema ).listTables();
  for ( std::set< std::string >::const_iterator iTable = listOfTables.begin();
        iTable != listOfTables.end(); ++iTable ) std::cout << *iTable << std::endl;

  std::cout << "Attempting to catch a specific exception for accessing an unknown schema" << std::endl;
  try {
    session->schema( m_otherSchema + "X" );
  }
  catch ( coral::InvalidSchemaNameException& ) {}
  std::cout << "Exception caught." << std::endl;
}
