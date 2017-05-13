// -*- C++ -*-
#ifndef CONNECTIONSERVICE_EMPTYWEBCACHECONTROL_H
#define CONNECTIONSERVICE_EMPTYWEBCACHECONTROL_H

#include "RelationalAccess/IWebCacheControl.h"
#include "RelationalAccess/IWebCacheInfo.h"

#include <map>
#include <set>


namespace coral
{

  class Context;

  namespace ConnectionService
  {

    class WebCacheInfo : virtual public IWebCacheInfo
    {

    public:

      /// default constructor
      WebCacheInfo();

      /// copy constructor
      WebCacheInfo( const WebCacheInfo& rhs );

      /// destructor
      virtual ~WebCacheInfo();

      /// assignment
      WebCacheInfo& operator=( const WebCacheInfo& rhs );

      /// Checks if the schema info (data dictionary) is cached, i.e. it does not need to be refreshed.
      /// Deprecated.  Returns true if the default timeToLive != 1
      bool isSchemaInfoCached() const;

      /// Checks if a table in the schema is cached, i.e. it does not need to be refreshed
      /// Deprecated: use instead tableTimeToLive( tableName ) != 1
      bool isTableCached( const std::string& tableName ) const;

      /// Returns timeToLive value (1=short, 2=long, 3=forever) for a table in the schema
      int tableTimeToLive( const std::string& tableName ) const;

      /// sets the default timeToLive. only used by deprecated function.
      void setDefaultTimeToLive( int timeToLive );

      /// sets the timeToLive of the specified table
      void setTableTimeToLive( const std::string& tableName, int timeToLive );

    private:

      /// default timeToLive for tables not specified
      int m_defaultTimeToLive;

      /// associative array of tables to their timeToLive
      std::map<std::string, int> m_tablesTimeToLive;
    };


    /// implementation of the interface IWebCacheControl
    class WebCacheControl : virtual public coral::IWebCacheControl
    {

    public:

      /// constructor
      WebCacheControl( coral::Context* context, const std::string& serviceName );

      /// destructor
      virtual ~WebCacheControl();

      /**
         Instructs the RDBMS backend that all the tables within the schema specified
         by the physical or logical connection should be refreshed, in case they are accessed.
	 Deprecated.  Sets default timeToLive to 1 for tables not identified.
      */
      void refreshSchemaInfo( const std::string& connection );

      /**
         Instructs the RDBMS backend that the specified table within the schema specified
         by the physical or logical connection should be refreshed in case it is accessed.
	 Deprecated: use instead setTableTimeToLive( connection, tableName, 1 )
      */
      void refreshTable( const std::string& connection,
                         const std::string& tableName );

      /**
         Instructs the RDBMS backend to cache queries that use the table specified by tableName
	 within the schema specified by connection for the time length specified by timeToLive,
	 1=short, 2=long, 3=forever.  Default 2.
      */
      void setTableTimeToLive( const std::string& connection,
                               const std::string& tableName,
			       int timeToLive );

      /**
         Returns the web cache information for a schema given the corresponding physical or
         logical connection.
      */
      const IWebCacheInfo& webCacheInfo( const std::string& connection ) const;

      /**
         Returns the previous compression level
      */
      int compressionLevel();

      /**
         Sets the compression level for data transfer, 0 - off, 1 - fast, 5 - default, 9 - maximum
      */
      void setCompressionLevel( int level=5 );

      /**
         Sets the list of the web cache proxies for the fail-over mechanism
      */
      void setProxyList( const std::vector<std::string>& proxyList );

    private:

      /// the service context
      coral::Context*                      m_serviceContext;

      /// the upper level service name
      std::string m_serviceName;

      /// the map of Web Cache info per connection
      std::map<std::string, WebCacheInfo > m_webCacheConfiguration;

      ///
      int m_compressionLevel;

      ///
      std::vector<std::string>             m_proxyList;
    };
  }

}

#endif
