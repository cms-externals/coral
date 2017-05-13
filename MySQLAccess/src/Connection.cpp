// $Id: Connection.cpp,v 1.21 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"
#include "errmsg.h"
#include "mysqld_error.h"

#include <iostream>
#include <string>

#include "CoralBase/MessageStream.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/URIParser.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SessionException.h"

#include "Connection.h"
#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "MonitorController.h"
#include "Schema.h"
#include "Session.h"
#include "SessionProperties.h"
#include "Transaction.h"

namespace coral
{
  namespace MySQLAccess
  {
    Connection::Connection( const DomainProperties& domainProperties, const std::string& connectionString )
      : m_mysql( 0 )
      , m_connectionString( connectionString )
      , m_serverVersion()
      , m_domainProperties( domainProperties )
      , m_connected( false )
    {
      this->initialize();
    }

    Connection::~Connection()
    {
      if( this->m_connected ) this->disconnect();
      if( mysql_thread_safe() ) mysql_thread_end();
    }

    void Connection::connect()
    {
      {
        boost::mutex::scoped_lock lock( m_lock );

        if( this->m_connected )
        {
          coral::MessageStream log( m_domainProperties.service()->name() );
          log << coral::Warning << "A connection is already present for \"" << this->m_connectionString << "\"" << std::endl << coral::MessageStream::endmsg;
          return;
        }
      }

      if( this->m_mysql == 0 )
        this->initialize();

      coral::MessageStream log( m_domainProperties.service()->name() );
      log << coral::Debug << "Connecting to : " << m_connectionString << std::endl;

      //       // Processing the connection string
      //       coral::URIParser parser;
      //       parser.setURI( "mysql://" + (this->m_connectionString) );

      //       const char* hostn = parser.hostName().c_str();
      //       unsigned    hostp = parser.portNumber();

      //       {
      //         boost::mutex::scoped_lock lock( m_lock );

      //         // Make physical connection to a MySQL server without attaching to a particular schema,
      //         // uses OS credentials, might succeed if they match the ones in MySQL server
      //         if ( ! mysql_real_connect( this->m_mysql,
      //                                    hostn,
      //                                    0, 0, 0,
      //                                    hostp,
      //                                    0,
      //                                    CLIENT_FOUND_ROWS ) )  // This connect time setting ensures compatible behavior with OracleAccess for UPDATE
      //         {
      //           // Could not connect to the server
      //           coral::MySQLAccess::ErrorHandler errorHandler;
      //           MYSQL_HANDLE_ERR(errorHandler, this->m_mysql, "Connecting to the server");

      //           coral::MessageStream log( m_domainProperties.service(), m_domainProperties.service()->name(), coral::Debug );

      //           if( errorHandler.isError() && errorHandler.lastErrorCode() ==  ER_ACCESS_DENIED_ERROR )
      //           {
      //             // Looks we simply failed to connect with current login without a password so
      //             // we try to establish a real connection using user supplied credentials
      //             log << coral::Warning << "Establishing connection with login name failed, user supplied credentials required" << std::endl << coral::MessageStream::endmsg;
      //           }
      //           else
      //           {
      //             if( errorHandler.lastErrorCode() == CR_SERVER_LOST ||
      //                 errorHandler.lastErrorCode() == CR_CONN_HOST_ERROR ||
      //                 errorHandler.lastErrorCode() == CR_CONNECTION_ERROR )
      //             {
      //               throw coral::ConnectionException( m_domainProperties.service()->name(), errorHandler.message(), "coral::MySQLAccess::connect" );
      //             }
      //             else
      //             {
      //               log << coral::Error << errorHandler.message() << std::endl << coral::MessageStream::endmsg;
      //               throw coral::DatabaseNotAccessibleException( m_domainProperties.service()->name(), "IConnection::connect" );
      //             }

      //           }

      //           this->m_connected = false;

      //           return;
      //         }

      //         // Retrieve server version only if we managed to connect with OS user credentials
      //         std::string srv_info = mysql_get_server_info( this->m_mysql );
      //         this->m_serverVersion = srv_info;

      //         this->m_connected = true;
      //       } // End of critical section
    }

    coral::ISession* coral::MySQLAccess::Connection::newSession( const std::string& schemaName, coral::AccessMode mode /*= Update*/ ) const
    {
      if( this->size() != 0 )
        throw coral::MaximumNumberOfSessionsException( "coral::MySQLAccess::Connection::newSession" );

      return new coral::MySQLAccess::Session( const_cast<coral::MySQLAccess::Connection&>( *this ),
                                              this->m_domainProperties, this->m_connectionString, schemaName,
                                              const_cast<MYSQL*&>(this->m_mysql), const_cast<bool&>(this->m_connected),
                                              m_lock,
                                              mode);
    }

    bool Connection::isConnected( bool probePhysicalConnection )
    {

      {
        boost::mutex::scoped_lock lock( m_lock );

        // We perform probing only if the conection has been once established
        if( probePhysicalConnection && this->m_connected )
        {
          if( mysql_ping( this->m_mysql ) )
          {
            coral::MySQLAccess::ErrorHandler errorHandler;
            MYSQL_HANDLE_ERR(errorHandler, this->m_mysql, "Probing physical connection");
            coral::MessageStream log( m_domainProperties.service()->name() );
            log << coral::Error << errorHandler.message() << std::endl << coral::MessageStream::endmsg;
            if( this->m_connected )
            {
              this->invalidateAllSessions(); // Clean up all the active sessions !!!
              ::mysql_close( this->m_mysql );
              this->m_mysql         = 0;
              this->m_connected     = false;
              this->m_serverVersion = "";
            }
          }
          else
            this->m_connected = true;
        }

        return this->m_connected;
      }
    }

    void Connection::disconnect()
    {
      boost::mutex::scoped_lock lock( m_lock );
      if( this->m_connected )
      {
        this->invalidateAllSessions(); // Clean up all the active sessions !!!
#ifdef _WIN32
        // Temporary debug printout for bug #64427
        std::cout << "MySQLAccess::Connection::disconnect: call mysql_close..." << std::endl;
        ::mysql_close( this->m_mysql );
        std::cout << "MySQLAccess::Connection::disconnect: call mysql_close... OK" << std::endl;
#else
        ::mysql_close( this->m_mysql );
#endif
        this->m_mysql         = 0;
        this->m_connected     = false;
        this->m_serverVersion = "";
      }
    }

    std::string Connection::serverVersion() const
    {
      boost::mutex::scoped_lock lock( m_lock );

      if ( ! this->m_connected )
        throw coral::ConnectionNotActiveException( m_domainProperties.service()->name(), "coral::MySQLAccess::Connection::serverVersion" );

      if( ! this->m_serverVersion.empty() )
        return this->m_serverVersion;

      // Retrieve server version
      this->m_serverVersion = std::string( mysql_get_server_info( this->m_mysql ) );

      return this->m_serverVersion;
    }

    coral::ITypeConverter& Connection::typeConverter()
    {
      return this->m_domainProperties.typeConverter();
    }

    void Connection::initialize(void)
    {
      boost::mutex::scoped_lock lock( m_lock );

      if( mysql_thread_safe() )
      {
        m_mysql = ::mysql_init( 0 );
        mysql_thread_init();
      }
      else
      {
        m_mysql = ::mysql_init( 0 );
      }

      unsigned int timeout = 15; // seconds
      mysql_options( m_mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)(&timeout) );
    }

  }
}
