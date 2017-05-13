// $Id: SocketServer.h,v 1.6.2.2 2010/12/20 14:04:40 avalassi Exp $
#ifndef CORALSOCKETS_GENERICSOCKETSERVERMT_SLAC_H
#define CORALSOCKETS_GENERICSOCKETSERVERMT_SLAC_H 1

// Include files
#include <map>

#include "CoralServerBase/IRequestHandlerFactory.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralSockets/GenericSocketException.h"

namespace coral {

  // Forward declaration
  class IThreadMonitor;

  namespace CoralSockets {

    // Forward declaration
    class ThreadManager;

    /** @class SocketServer SocketServer.h
     *
     *  @author Andrea Valassi and Martin Wache
     *  @date   2007-12-23
     */

    class SocketServer
    {
    public:

      virtual ~SocketServer();

      /// you have to make sure, that the handler is valid over the lifetime
      /// of the SocketServer
      SocketServer( coral::IRequestHandlerFactory& handlerFactory,
                    const std::string& host,
                    int port,
                    int nHandlerThreadsPerSocket=5 );

      /// main loop of socket server
      /// if timeout > 0 the server will shutdown after timeout seconds
      /// with no new connection
      void run( int timeout=-1 );

      /// signal the server to stop
      void stopServer();

      /// get the thread monitor for this socket server
      const IThreadMonitor* threadMonitor() const;

    private:

      /// Standard constructor is private
      SocketServer();

      /// if false, the server will shut down
      bool m_isActive;

      /// the request handler factory which is used to produce
      /// request handler for a new connection
      coral::IRequestHandlerFactory& m_handlerFactory;

      /// connection details
      std::string m_host;
      int m_port;
      int m_nHandlerThreadsPerSocket;

      /// a thread manager
      std::auto_ptr<ThreadManager> m_thrManager;

    };

  }

}
#endif
