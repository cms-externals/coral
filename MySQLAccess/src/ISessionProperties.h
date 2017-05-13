// $Id: ISessionProperties.h,v 1.19 2010/12/20 16:45:17 avalassi Exp $
#ifndef ISESSION_PROPERTIES_H
#define ISESSION_PROPERTIES_H 1

#include "MySQL_headers.h"

#include "CoralBase/boost_thread_headers.h"

namespace coral
{
  class ITypeConverter;
  class ITransaction;
  class ISchema;

  namespace monitor
  {
    class IMonitoringService;
  }

  namespace MySQLAccess
  {
    class DomainProperties;
    class TypeConverter;

    struct ServerRevision
    {
      unsigned long major; unsigned long minor; unsigned long revision;
    };

    /**
     * Class SessionProperties
     *
     * Interface for classes holding the domain properties, as well as the global
     * data and properties for the current session.
     */

    class ISessionProperties
    {
    public:

      /// Virtual destructor
      virtual ~ISessionProperties() {}

      /// Returns the domain properties
      virtual const DomainProperties& domainProperties() const = 0;

      /// Returns the type converter
      virtual coral::ITypeConverter& typeConverter() = 0;
      virtual const coral::ITypeConverter& typeConverter() const = 0;

      /// Returns the MYSQL connection handle
      virtual MYSQL*& connectionHandle() const = 0;

      /// Returns the monitoring service
      virtual coral::monitor::IMonitoringService* monitoringService() = 0;
      virtual const coral::monitor::IMonitoringService* monitoringService() const = 0;

      /// Returns the transaction state
      virtual bool isTransactionActive() const = 0;

      /// Returns the transaction mode
      virtual bool isTransactionReadOnly() const = 0;

      /// Returns the corresponding schema
      virtual coral::ISchema& schema() const = 0;

      /// Returns the schema name
      virtual std::string schemaName() const = 0;

      /// Returns the connection string
      virtual std::string connectionString() const = 0;

      /// The database server version
      virtual std::string serverVersion() const = 0;

      /// The database server version
      virtual ServerRevision serverRevision() const = 0;

      /// Return the connection lock
      virtual boost::mutex& lock() const = 0;

      /// Returns the readOnly flag
      virtual bool isReadOnly() const = 0;

    };
  }
}

#endif // ISESSION_PROPERTIES_H
