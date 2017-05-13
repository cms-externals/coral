#ifndef RELATIONALACCESS_SESSION_EXCEPTION_H
#define RELATIONALACCESS_SESSION_EXCEPTION_H

#include "CoralBase/Exception.h"

namespace coral {

  /// Base exception class for errors related to a session.
  class SessionException : public Exception
  {
  public:
    /// Constructor
    SessionException( const std::string& message,
                      const std::string& methodName,
                      const std::string& moduleName ):
      Exception( message, methodName, moduleName )
    {}
    
    SessionException() {}

    /// Destructor
    virtual ~SessionException() throw() {}
  };


  /// Exception thrown when the maximum number of sessions is exceeded per connection
  class MaximumNumberOfSessionsException : public SessionException
  {
  public:
    /// Constructor
    MaximumNumberOfSessionsException( const std::string& moduleName ):
      SessionException( "Cannot create another session in the same connection",
                        "IConnection::newSession", moduleName )
    {}
    
    MaximumNumberOfSessionsException() {}
    
    /// Destructor
    virtual ~MaximumNumberOfSessionsException() throw() {}
  };


  /// Exception thrown when a new session is refused by the server. It describes a possibly temporarely condition.
  class StartSessionException : public SessionException
  {
  public:
    /// Constructor
    StartSessionException( const std::string& moduleName,
                           const std::string& methodName ):
      SessionException( "Cannot create a new session.", methodName, moduleName )
    {}
    
    StartSessionException() {}
    
    /// Destructor
    virtual ~StartSessionException() throw() {}
  };
  
  class InvalidOperationInReadOnlyModeException : public SessionException
  {
  public:
    /// Constructor
    InvalidOperationInReadOnlyModeException( const std::string& moduleName,
                                             const std::string& methodName ):
      SessionException( "An unpdate operation during a read-only session has been requested",
                        methodName, moduleName )
    {}
    
    InvalidOperationInReadOnlyModeException() {}
    
    /// Destructor
    virtual ~InvalidOperationInReadOnlyModeException() throw() {}
  };




  /// Exception thrown when an invalid schema name exception is specified
  class InvalidSchemaNameException : public SessionException
  {
  public:
    /// Constructor
    InvalidSchemaNameException( const std::string& moduleName,
                                std::string methodName = "ISession::schema" ):
      SessionException( "An invalid schema name has been specified",
                        methodName, moduleName )
    {}
    
    InvalidSchemaNameException() {}
    
    /// Destructor
    virtual ~InvalidSchemaNameException() throw() {}
  };



  /// Exception thrown when no authentication service has been loaded when a user session is started
  class NoAuthenticationServiceException : public SessionException
  {
  public:
    /// Constructor
    NoAuthenticationServiceException( const std::string& moduleName,
                                      std::string methodName = "ISession::startUserSession" ):
      SessionException( "No Authentication Service has been loaded",
                        methodName, moduleName )
    {}
    
    NoAuthenticationServiceException() {}
    
    /// Destructor
    virtual ~NoAuthenticationServiceException() throw() {}
  };


  /// Exception thrown when authentication fails with a database server.
  class AuthenticationFailureException : public SessionException
  {
  public:
    /// Constructor
    AuthenticationFailureException( const std::string& moduleName,
                                    std::string methodName = "ISession::startUserSession" ):
      SessionException( "Failed to authenticate with the server",
                        methodName, moduleName )
    {}
    
    AuthenticationFailureException() {}
    
    /// Destructor
    virtual ~AuthenticationFailureException() throw() {}
  };



  /// Exception thrown when a Monitoring Service has not been found in the context hierarchy
  class MonitoringServiceNotFoundException : public SessionException
  {
  public:
    /// Constructor
    MonitoringServiceNotFoundException( const std::string& moduleName,
                                        std::string methodName = "IMonitoring::start" ):
      SessionException( "No Monitoring service has been found",
                        methodName, moduleName )
    {}

    MonitoringServiceNotFoundException() {}
    
    /// Destructor
    virtual ~MonitoringServiceNotFoundException() throw() {}
  };
  
  /// Exception thrown when the database server cannot be reached.
  class ServerException : public SessionException
  {
  public:
    /// Constructor
    ServerException( const std::string& moduleName,
                     std::string message = "The database server could not be reached.",
                     std::string methodName = "ISession::connect" ):
      SessionException( message, methodName, moduleName )
    {}

    ServerException() {}
    
    /// Destructor
    virtual ~ServerException() throw() {}
  };

  /// Exception thrown when the requested database server cannot be resolved or contacted.
  class DatabaseNotAccessibleException : public ServerException
  {
  public:
    /// Constructor
    DatabaseNotAccessibleException( const std::string& moduleName,
                                  std::string methodName,
                                  std::string message = "The specified service could not be reached."  ):
      ServerException( moduleName, message, methodName ){}

    DatabaseNotAccessibleException() {}
    
    /// Destructor
    virtual ~DatabaseNotAccessibleException() throw() {}
  };

  /// Exception thrown when the database server cannot be reached.
  class ConnectionException : public ServerException
  {
  public:
    /// Constructor
    ConnectionException( const std::string& moduleName,
                         std::string methodName,
                         std::string message = "Cannot connect to the server."  ):
      ServerException( moduleName, message, methodName ){}

    ConnectionException() {}
    
    /// Destructor
    virtual ~ConnectionException() throw() {}
  };

  
  /// Exception thrown when the database server cannot be reached from an already created connection object.
  class ConnectionNotActiveException : public ConnectionException
  {
  public:
    /// Constructor
    ConnectionNotActiveException( const std::string& moduleName,
                                  std::string methodName = "ISession::userSchema",
                                  std::string message = "No connection to the server is active"):
      ConnectionException( moduleName, message, methodName ){}

    ConnectionNotActiveException() {}
    
    /// Destructor
    virtual ~ConnectionNotActiveException() throw() {}
  };


  /// Base class for transaction-related exceptions
  class TransactionException : public SessionException
  {
  public:
    /// Constructor
    TransactionException( const std::string& moduleName,
                          const std::string& message,
                          const std::string& methodName ):
      SessionException( message, methodName, moduleName )
    {}

    TransactionException() {}
    
    /// Destructor
    virtual ~TransactionException() throw() {}
  };


  /// Exception thrown when a transaction could not be started
  class TransactionNotStartedException : public TransactionException
  {
  public:
    /// Constructor
    TransactionNotStartedException( const std::string& moduleName,
                                    std::string message = "A transaction could not be started" ):
      TransactionException( moduleName, message, "ITransaction::start" )
    {}

    TransactionNotStartedException() {}
    
    /// Destructor
    virtual ~TransactionNotStartedException() throw() {}
  };


  /// Exception thrown when a transaction could not be commited
  class TransactionNotCommittedException : public TransactionException
  {
  public:
    /// Constructor
    TransactionNotCommittedException( const std::string& moduleName,
                                      std::string message = "A transaction could not be committed" ):
      TransactionException( moduleName, message, "ITransaction::commit" )
    {}
    
    TransactionNotCommittedException() {}

    /// Destructor
    virtual ~TransactionNotCommittedException() throw() {}
  };


  /// Exception thrown when a transaction is not active
  class TransactionNotActiveException : public TransactionException
  {
  public:
    /// Constructor
    TransactionNotActiveException( const std::string& moduleName,
                                   const std::string& methodName,
                                   std::string message = "A transaction is not active" ):
      TransactionException( moduleName, message, methodName )
    {}
    
    TransactionNotActiveException() {}

    /// Destructor
    virtual ~TransactionNotActiveException() throw() {}
  };


  class InvalidOperationInReadOnlyTransactionException : public SessionException
  {
  public:
    /// Constructor
    InvalidOperationInReadOnlyTransactionException( const std::string& moduleName,
                                                    const std::string& methodName ):
      SessionException( "An update operation during a read-only transaction has been requested",
                        methodName, moduleName )
    {}
    
    InvalidOperationInReadOnlyTransactionException() {}
    
    /// Destructor
    virtual ~InvalidOperationInReadOnlyTransactionException() throw() {}
  };



  /// Base class for type-converter-related exceptions
  class TypeConverterException : public SessionException
  {
  public:
    /// Constructor
    TypeConverterException( const std::string& moduleName,
                            const std::string& message,
                            const std::string& methodName ):
      SessionException( message, "ITypeConverter::" + methodName, moduleName )
    {}
    
    TypeConverterException() {}

    /// Destructor
    virtual ~TypeConverterException() throw() {}
  };


  /// Exception thrown whenever an SQL type is not supported
  class UnSupportedSqlTypeException : public TypeConverterException
  {
  public:
    /// Constructor
    UnSupportedSqlTypeException( const std::string& moduleName,
                                 const std::string& methodName,
                                 const std::string& sqlType ):
      TypeConverterException( moduleName,
                              "SQL type \"" + sqlType + "\" is not supported",
                              methodName )
    {}
    
    UnSupportedSqlTypeException() {}
    
    /// Destructor
    virtual ~UnSupportedSqlTypeException() throw() {}
  };


  /// Exception thrown whenever a C++ type is not supported
  class UnSupportedCppTypeException : public TypeConverterException
  {
  public:
    /// Constructor
    UnSupportedCppTypeException( const std::string& moduleName,
                                 const std::string& methodName,
                                 const std::string& cppType ):
      TypeConverterException( moduleName,
                              "C++ type \"" + cppType + "\" is not supported",
                              methodName )
    {}
    
    UnSupportedCppTypeException() {}

    /// Destructor
    virtual ~UnSupportedCppTypeException() throw() {}
  };


}

#endif
