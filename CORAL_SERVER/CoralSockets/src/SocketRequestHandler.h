// $Id: SocketRequestHandler.h,v 1.4.2.2 2010/12/20 09:10:11 avalassi Exp $
#ifndef CORALSOCKETS_SOCKETREQUESTHANDLER_H
#define CORALSOCKETS_SOCKETREQUESTHANDLER_H 1

// Include files

// Local include files
#include "CoralServerBase/IRequestHandler.h"
#include "CoralSockets/GenericSocketException.h"

#include "PacketSocket.h"
#include "ReplyManager.h"

namespace coral {

  namespace CoralSockets {

    class ThreadManager;

    /** @class SocketRequestHandler SocketRequestHandler.h
     *
     *  @author Andrea Valassi and Martin Wache
     *  @date   2007-12-26
     */

    class SocketRequestHandler : public coral::IRequestHandler
    {

    public:

      virtual ~SocketRequestHandler();

      SocketRequestHandler( PacketSocketPtr socket );

      void setCertificateData( const coral::ICertificateData* cert );

      /// reply to the request (thread safe)
      IByteBufferIteratorPtr replyToRequest( const ByteBuffer& requestBuffer );

    protected:

      // sends the request over the sockets, registers a replySlot for the
      // request and returns the replySlot
      ReplySlot& send( const ByteBuffer& requestMsg );

    private:

      /// Standard constructor is private
      SocketRequestHandler();
      /// copy constructor is private
      SocketRequestHandler( const SocketRequestHandler &handler);
      /// assignment operator as well
      SocketRequestHandler& operator=( const SocketRequestHandler& );

      /// packet socket for connection
      PacketSocketPtr m_Socket;

      /// a mutex to protect the sending and m_requestID
      boost::mutex m_mutexForSend;

      /// the current requestID
      int m_requestID;

      /// storage for the waiting requests
      std::auto_ptr<ReplyManager> m_rplMngr;

      // Forward declaration
      class receiveThread; // thread for receiving packets

      /// one thread for receiving packets
      receiveThread *m_receiveThread;

      /// thread manager to handle receive thread
      std::auto_ptr<ThreadManager> m_thrManager;
    };

  }

}
#endif
