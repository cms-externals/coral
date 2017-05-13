#include "WebCacheControl.h"

#include "RelationalAccess/WebCacheControlException.h"

#include "CoralKernel/Context.h"

/// constructor
coral::ConnectionService::WebCacheControl::WebCacheControl( coral::Context* context, const std::string& serviceName ) :
  m_serviceContext( context ),
  m_serviceName( serviceName ),
  m_webCacheConfiguration(),
  m_compressionLevel( 5 ),
  m_proxyList()
{
}

/// destructor
coral::ConnectionService::WebCacheControl::~WebCacheControl(){
}

/**
 * Instructs the RDBMS backend that all the tables within the schema specified
 * by the physical or logical connection should be refreshed, in case they are accessed.
 * Deprecated.  Sets default timeToLive to 1 for tables not identified.
 */
void
coral::ConnectionService::WebCacheControl::refreshSchemaInfo( const std::string& connection ){
  std::map<std::string, WebCacheInfo >::iterator iter = m_webCacheConfiguration.find( connection );
  if( iter == m_webCacheConfiguration.end() ) {
    WebCacheInfo cacheInfo;
    cacheInfo.setDefaultTimeToLive( 1 );
    m_webCacheConfiguration.insert( std::make_pair( connection, cacheInfo ) );
  } else {
    iter->second.setDefaultTimeToLive( 1 );
  }
}

/**
 * Instructs the RDBMS backend that the specified table within the schema specified
 * by the physical or logical connection should be refreshed in case it is accessed.
 * Deprecated: use instead setTableTimeToLive( connection, tableName, 1 )
 */
void
coral::ConnectionService::WebCacheControl::refreshTable( const std::string& connection,
                                                         const std::string& tableName ){
  setTableTimeToLive( connection, tableName, 1 );
}

/**
 * Instructs the RDBMS backend to cache queries that use the table specified by tableName
 * within the schema specified by connection for the time length specified by timeToLive,
 * 1=short, 2=long, 3=forever.  Default 2.
 */
void
coral::ConnectionService::WebCacheControl::setTableTimeToLive( const std::string& connection,
                                                               const std::string& tableName,
                                                               int timeToLive ){
  std::map<std::string, WebCacheInfo >::iterator iter = m_webCacheConfiguration.find( connection );
  if( iter == m_webCacheConfiguration.end() ) {
    WebCacheInfo cacheInfo;
    cacheInfo.setTableTimeToLive( tableName, timeToLive );
    m_webCacheConfiguration.insert( std::make_pair(connection,cacheInfo) );
  } else {
    iter->second.setTableTimeToLive( tableName, timeToLive );
  }
}

/**
 * Returns the web cache information for a schema given the corresponding physical or
 * logical connection.
 */
const coral::IWebCacheInfo&
coral::ConnectionService::WebCacheControl::webCacheInfo( const std::string& connection ) const {
  std::map<std::string, WebCacheInfo >::const_iterator iter = m_webCacheConfiguration.find( connection );
  if( iter == m_webCacheConfiguration.end() ) {
    throw coral::WebCacheControlException("No information found for connection "+connection,
                                          "WebCacheControl::webCacheInfo",
                                          "CORAL/Services/ConnectionService");
  }
  return iter->second;
}

int
coral::ConnectionService::WebCacheControl::compressionLevel()
{
  return m_compressionLevel;
}

void
coral::ConnectionService::WebCacheControl::setCompressionLevel( int level )
{
  m_compressionLevel = level;
}

void
coral::ConnectionService::WebCacheControl::setProxyList( const std::vector<std::string>& proxyList )
{
  m_proxyList = proxyList;
}

/// constructor
coral::ConnectionService::WebCacheInfo::WebCacheInfo() :
  m_defaultTimeToLive(2),
  m_tablesTimeToLive(){
}

/// constructor
coral::ConnectionService::WebCacheInfo::WebCacheInfo( const WebCacheInfo& rhs ) :
  IWebCacheInfo(),
  m_defaultTimeToLive( rhs.m_defaultTimeToLive ),
  m_tablesTimeToLive( rhs.m_tablesTimeToLive ){
}

/// destructor
coral::ConnectionService::WebCacheInfo::~WebCacheInfo(){
}

/// assignment
coral::ConnectionService::WebCacheInfo& coral::ConnectionService::WebCacheInfo::operator=( const WebCacheInfo& rhs ){
  if(this!=&rhs) {
    m_defaultTimeToLive = rhs.m_defaultTimeToLive;
    m_tablesTimeToLive = rhs.m_tablesTimeToLive;
  }
  return *this;
}


/// Checks if the schema info (data dictionary) is cached, i.e. it does not need to be refreshed
/// Deprecated.  Returns true if the default timeToLive != 1
bool
coral::ConnectionService::WebCacheInfo::isSchemaInfoCached() const {
  return m_defaultTimeToLive != 1;
}

/// Checks if a table in the schema is cached, i.e. it  does not need to be refreshed
/// Deprecated: use instead tableTimeToLive( tableName ) != 1
bool
coral::ConnectionService::WebCacheInfo::isTableCached( const std::string& tableName ) const {
  return tableTimeToLive( tableName) != 1;
}

/// Returns timeToLive value (1=short, 2=long, 3=forever) for a table in the schema
int
coral::ConnectionService::WebCacheInfo::tableTimeToLive( const std::string& tableName ) const {
  std::map<std::string, int>::const_iterator iter = m_tablesTimeToLive.find( tableName );
  if( iter == m_tablesTimeToLive.end() ) {
    // not found, return the default
    return m_defaultTimeToLive;
  }
  return iter->second;
}


/// sets default timeToLive.   only used by deprecated function.
void
coral::ConnectionService::WebCacheInfo::setDefaultTimeToLive( int timeToLive ){
  m_defaultTimeToLive = timeToLive;
}

/// sets the timeToLive of the specified table
void
coral::ConnectionService::WebCacheInfo::setTableTimeToLive( const std::string& tableName, int timeToLive ){
  m_tablesTimeToLive[ tableName ] = timeToLive;
}
