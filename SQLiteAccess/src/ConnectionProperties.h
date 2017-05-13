#ifndef SQLITEACCESS_CONNECTIONPROPERTIES_H
#define SQLITEACCESS_CONNECTIONPROPERTIES_H 1

#include <string>
#include "CoralBase/boost_filesystem_headers.h"
#include "CoralBase/boost_thread_headers.h"

namespace coral
{

  // Forward declaration
  class ITypeConverter;

  namespace SQLiteAccess
  {

    class Connection;
    class DomainProperties;
    class TypeConverter;

    /**
     * Class ConnectionProperties
     *
     * A class holding the running parameters of a database connection
     */

    class ConnectionProperties
    {

    public:
      /// Constructor
      ConnectionProperties( const DomainProperties& domainProperties,
                            //Connection& connection,
                            const std::string& connectionString );

      /// Destructor
      virtual ~ConnectionProperties();


      /// Invalidates the connection (FIXME: choose another method name?)
      //void invalidateConnection();

      /// Sets the read-only flag
      void setReadOnly( bool flag );

      /// Sets the database file name
      void setFileName( const boost::filesystem::path& inFileName );

      /// Returns the database file name
      boost::filesystem::path databaseFileName() const;

      /// Returns the domain properties
      const DomainProperties& domainProperties() const;

      /// Returns the domain service name
      const std::string& domainServiceName() const;

      /// Returns the connection string
      std::string connectionString() const;

      /// Returns the connection object
      //Connection& connection() const;

      /// Returns the type converter
      coral::ITypeConverter& typeConverter();

      /// Returns the type converter
      const coral::ITypeConverter& typeConverter() const;

      /// Returns the readOnly flag
      bool isReadOnly() const;

      /// Returns the mutex
      boost::mutex* mutex() const { return m_mutex; };

    private:

      /// The domain properties
      const DomainProperties& m_domainProperties;

      /// The domain service name (this is retrieved from the DomainProperties
      /// and is guaranteed to be valid at destruction time - fix bug #71210)
      const std::string m_domainServiceName;

      /// The connection string
      std::string m_connectionString;

      /// The type converter
      TypeConverter* m_typeConverter;

      /// The read-only flag
      bool m_isReadOnly;

      /// The sqlite file name
      boost::filesystem::path m_inFileName;

      /// The connection pointer (NULL if invalid)
      //Connection* m_connection;

      // The mutex
      boost::mutex* m_mutex;

      /// The server version
      //std::string m_serverVersion;
    };

  }

}


/*
inline void
coral::SQLiteAccess::ConnectionProperties::invalidateConnection()
{
  m_connection = NULL;
}
*/


inline const coral::SQLiteAccess::DomainProperties&
coral::SQLiteAccess::ConnectionProperties::domainProperties() const
{
  return m_domainProperties;
}


inline const std::string&
coral::SQLiteAccess::ConnectionProperties::domainServiceName() const
{
  return m_domainServiceName;
}


inline std::string
coral::SQLiteAccess::ConnectionProperties::connectionString() const
{
  return m_connectionString;
}


inline bool
coral::SQLiteAccess::ConnectionProperties::isReadOnly() const
{
  return m_isReadOnly;
}


inline void
coral::SQLiteAccess::ConnectionProperties::setReadOnly( bool flag )
{
  m_isReadOnly = flag;
}


inline void
coral::SQLiteAccess::ConnectionProperties::setFileName( const boost::filesystem::path& inFileName )
{
  m_inFileName = inFileName;
}


inline boost::filesystem::path
coral::SQLiteAccess::ConnectionProperties::databaseFileName() const
{
  return m_inFileName;
}

#endif
