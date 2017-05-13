#ifndef CORAL_RELATIONALACCESS_IWEBCACHEINFO_H
#define CORAL_RELATIONALACCESS_IWEBCACHEINFO_H

#include <string>

namespace coral {

  /**
     @class IWebCacheInfo IWebCacheInfo.h RelationalAccess/IWebCacheInfo.h
     Interface for accessing the web cache policy for a given schema
  */
  class IWebCacheInfo
  {
  public:
    /// Checks if the schema info (data dictionary) is cached, i.e. it does not need to be refreshed.
    /// Deprecated.  Returns true if the default timeToLive != 1
    virtual bool isSchemaInfoCached() const = 0;

    /// Checks if a table in the schema is cached, i.e. it does not need to be refreshed.
    /// Deprecated: use instead tableTimeToLive( tableName ) != 1
    virtual bool isTableCached( const std::string& tableName ) const = 0;

    /// Returns timeToLive value (1=short, 2=long, 3=forever) for a table in the schema
    virtual int tableTimeToLive( const std::string& tableName ) const = 0;

  protected:
    /// Protected empty destructor
    virtual ~IWebCacheInfo() {}
  };

}

#endif
