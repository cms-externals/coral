#include <iostream>
#include "Domain.h"
#include "DomainProperties.h"
#include "Connection.h"

#include "CoralCommon/URIParser.h"

#include "sqlite3.h"

coral::SQLiteAccess::Domain::Domain( const std::string& componentName )
  : coral::Service( componentName ),
    m_flavorName( "SQLite" ),
    m_implementationName( "sqlite" ),
    m_implementationVersion( sqlite3_libversion() ),
    m_properties( NULL )
{

  //removed compilation warning for windows
  m_properties = new coral::SQLiteAccess::DomainProperties( this );
}

coral::SQLiteAccess::Domain::~Domain()
{

  delete m_properties;
}


std::string
coral::SQLiteAccess::Domain::flavorName() const
{

  return m_flavorName;
}

std::string
coral::SQLiteAccess::Domain::implementationName() const
{

  return m_implementationName;
}

std::string
coral::SQLiteAccess::Domain::implementationVersion() const
{

  return m_implementationVersion;
}

coral::IConnection*
coral::SQLiteAccess::Domain::newConnection(const std::string& uriString) const
{

  return new coral::SQLiteAccess::Connection( *m_properties, uriString );
}

std::pair<std::string, std::string >
coral::SQLiteAccess::Domain::decodeUserConnectionString( const std::string& userConnectionString ) const
{

  // Processing the connection string
  coral::URIParser parser;
  parser.setURI( userConnectionString );
  return std::make_pair(parser.databaseOrSchemaName(), "");
}

bool coral::SQLiteAccess::Domain::isAuthenticationRequired( const std::string& /* connectionString */ ) const {

  return false;
}
