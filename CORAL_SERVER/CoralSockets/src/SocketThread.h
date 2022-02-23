// $Id: SocketThread.h,v 1.11.2.5 2011/09/16 14:07:16 avalassi Exp $
#ifndef CORALSOCKETS_SOCKETTHREAD_H
#define CORALSOCKETS_SOCKETTHREAD_H 1

// Include files
#include <queue>
#include <vector>
#include "CoralServerBase/IRequestHandler.h"
#include "CoralServerBase/IThreadMonitor.h"
#include "CoralBase/boost_thread_headers.h"

// Local include files
#include "ThreadManager.h"
#include "PacketSocket.h"

namespace coral
{

  namespace CoralSockets
  {

    class SocketThread : public Runnable
                       , virtual public IThreadMonitor
    {

    private:

      // forward definition
      class HandlerThread;

      // copy constructor and assignment operator are private
      SocketThread( const SocketThread& rhs );
      SocketThread& operator=( const SocketThread& );

    public:

      virtual ~SocketThread() {}

      /// one SocketThread instance per socket
      ///
      /// starts one thread which does the receiving and
      /// nHandlerThreadsPerSocket threads for handling requests.
      ///
      SocketThread( coral::IRequestHandler* handler,
                    PacketSocketPtr cSocket,
                    int nHandlerThreadsPerSocket )
        : Runnable()
        , m_handler( handler )
        , m_cSocket( cSocket )
        , m_nHThreads( nHandlerThreadsPerSocket )
        , m_isActive( true )
      {}

      coral::IRequestHandler& getRequestHandler()
      {
        return *(m_handler.get());
      }

      // return a description of the thread
      const std::string desc() const;

      /// main loop of thread
      void operator()();

      /// send packets over the socket
      void send( const PacketSLAC& packet );

      /// get new incomming request
      PacketPtr getNewRequest();

      void endThread()
      {
        m_isActive=false;
      }

      bool isActive()
      {
        return m_isActive;
      }

      void closeSocket()
      {
        m_cSocket->close();
        endThread();
      }

      /// List all existing threads.
      const std::vector<ThreadRecord> listThreads() const;

    private:

      /// Standard constructor is private
      SocketThread();

      /// the request handler for this socket
      std::unique_ptr<coral::IRequestHandler> m_handler;

      PacketSocketPtr m_cSocket;

      const std::string m_cHost;

      // Pool of handler threads
      const int m_nHThreads; // target size of hThread pool
      std::vector<boost::thread*> m_hThreads;
      boost::mutex m_mutexForHThreads;

      std::queue<PacketPtr> m_requests;
      boost::mutex m_mutexForRequests;
      boost::condition_variable m_condForRequests;

      boost::mutex m_mutexForSend;

      bool m_isActive;

      ThreadManager m_thrManager;

    };
  }
}
#endif //CORALSOCKETS_SOCKETTHREAD_H
