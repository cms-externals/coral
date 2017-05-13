#ifndef CONNECTIONSERVICE_SESSIONHANDLE_H
#define CONNECTIONSERVICE_SESSIONHANDLE_H

#include "ConnectionHandle.h"
#include "TransactionProxy.h"
#include "RelationalAccess/AccessMode.h"

namespace coral
{

  class ISession;

  namespace ConnectionService
  {

    /// Placeholder structure for bookkeping of Connection instances allocation
    class SessionHandle
    {

    public:

      /// convenience object to store the info shared on all the handles
      class SessionSharedInfo
      {
      public:
        /// constructor
        SessionSharedInfo(const std::string& connectionServiceName) :
          m_sessionId(""),
          m_open(false),
          m_connectionServiceName(connectionServiceName){}
        /// destructor
        virtual ~SessionSharedInfo()
        {
          showSessionId( true ); // unregister id from the map
        }
        /// the unique identifier of the session
        Guid m_sessionId;
        /// the session open flag
        bool m_open;
        /// the connection service name
        std::string m_connectionServiceName;
        /// show the unique id of the session
        /// (format is user-controlled via environenment variables)
        const std::string showSessionId( bool last=false ) const;
      private:
        SessionSharedInfo();
      };

      SessionHandle( const std::string& connectionServiceName );

      SessionHandle( const std::string& connectionServiceName,
                     const ConnectionHandle& connection );

      SessionHandle( const SessionHandle& rhs );

      /// destructor
      virtual ~SessionHandle();

      SessionHandle& operator=(const SessionHandle& rhs);

      ///
      operator bool() const;

      /// initialize the session
      bool open( const std::string& schemaName,
                 const std::string& userName,
                 const std::string& password,
                 coral::AccessMode accessMode );

      void close();

      bool isOpen() const;

      bool isValid() const;

      ISession* physicalSession() const;

      ConnectionHandle& connection();

      const ConnectionHandle& connection() const;

      const std::string& connectionServiceName() const
      {
        return m_connectionServiceName;
      }

      TransactionProxy& transactionProxy()
      {
        return m_transactionProxy;
      }

    private:

      /// default constructor is private
      SessionHandle();

      /// the connection service name
      std::string m_connectionServiceName;

      /// handle of the ISession object
      boost::shared_ptr<ISession> m_session;

      /// stored session (shared) info
      boost::shared_ptr<SessionSharedInfo> m_info;

      /// the underlying connection
      ConnectionHandle m_connection;

      /// the transaction proxy
      TransactionProxy m_transactionProxy;

    };

  }

}
#endif
