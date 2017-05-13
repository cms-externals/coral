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
 */
void
coral::ConnectionService::WebCacheControl::refreshSchemaInfo( const std::string& connection ){
  std::map<std::string, WebCacheInfo >::iterator iter = m_webCacheConfiguration.find( connection );
  if( iter == m_webCacheConfiguration.end() ) {
    WebCacheInfo cacheInfo;
    cacheInfo.setSchemaInfoRefresh( true );
    m_webCacheConfiguration.insert( std::make_pair( connection, cacheInfo ) );
  } else {
    iter->second.setSchemaInfoRefresh( true );
  }
}

/**
 * Instructs the RDBMS backend that the specified table within the schema specified
 * by the physical or logical connection should be refreshed in case it is accessed.
 */
void
coral::ConnectionService::WebCacheControl::refreshTable( const std::string& connection,
                                                         const std::string& tableName ){
  std::map<std::string, WebCacheInfo >::iterator iter = m_webCacheConfiguration.find( connection );
  if( iter == m_webCacheConfiguration.end() ) {
    WebCacheInfo cacheInfo;
    cacheInfo.setTableRefresh( tableName, true );
    m_webCacheConfiguration.insert( std::make_pair(connection,cacheInfo) );
  } else {
    iter->second.setTableRefresh( tableName, true );
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
  m_schemaInfoRefresh(false),
  m_toRefreshTables(){
}

/// constructor
coral::ConnectionService::WebCacheInfo::WebCacheInfo( const WebCacheInfo& rhs ) :
  IWebCacheInfo(),
  m_schemaInfoRefresh( rhs.m_schemaInfoRefresh ),
  m_toRefreshTables( rhs.m_toRefreshTables ){
}

/// destructor
coral::ConnectionService::WebCacheInfo::~WebCacheInfo(){
}

/// assignment
coral::ConnectionService::WebCacheInfo& coral::ConnectionService::WebCacheInfo::operator=( const WebCacheInfo& rhs ){
  if(this!=&rhs) {
    m_schemaInfoRefresh = rhs.m_schemaInfoRefresh;
    m_toRefreshTables = rhs.m_toRefreshTables;
  }
  return *this;
}


/// Checks if the schema info (data dictionary) is cached, i.e.it  does not need to be refreshed
bool
coral::ConnectionService::WebCacheInfo::isSchemaInfoCached() const {
  return !m_schemaInfoRefresh;
}

/// Checks if a table in the schema is cached, i.e.it  does not need to be refreshed
bool
coral::ConnectionService::WebCacheInfo::isTableCached( const std::string& tableName ) const {
  bool ret = true;
  if( m_schemaInfoRefresh ) {
    ret = false;
  } else {
    std::set<std::string>::const_iterator iter = m_toRefreshTables.find( tableName );
    if( iter != m_toRefreshTables.end() ) ret = false;
  }
  return ret;
}

/// sets the caching of the schema info
void
coral::ConnectionService::WebCacheInfo::setSchemaInfoRefresh( bool flag ){
  m_schemaInfoRefresh = flag;
}


/// sets the caching of the specified table
void
coral::ConnectionService::WebCacheInfo::setTableRefresh( const std::string& tableName, bool flag ){
  if(flag)
  {
    m_toRefreshTables.insert( tableName );
  } else {
    m_toRefreshTables.erase( tableName );
  }
}
