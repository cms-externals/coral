// $Id: SocketClientWrapper.h,v 1.2.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_SOCKETCLIENTWRAPPER_H
#define CORALACCESS_SOCKETCLIENTWRAPPER_H 1

// Include files
#include "CoralSockets/SocketClient.h"

namespace coral
{

  namespace CoralAccess
  {

    /** @class SocketClientWrapper
     *
     *  @author Andrea Valassi
     *  @date   2009-02-05
     */

    class SocketClientWrapper : virtual public IRequestHandler
    {

    public:

      // Constructor from host and port.
      SocketClientWrapper( const std::string& host, int port )
        : m_client( host, port ) {}

      // Destructor.
      virtual ~SocketClientWrapper(){}

      /// Set the certificate data for this request handler.
      /// The caller has to ensure that the ICertificateData pointer
      /// remains valid over the lifetime of the IRequestHandler.
      void setCertificateData( const coral::ICertificateData* cert )
      {
        return m_client.getRequestHandler()->setCertificateData( cert );
      }

      // Handle a request message and return an iterator over reply messages.
      // This method cannot be const because we need to lock mutexes in it!
      // The iterator is positioned before the first reply in the loop:
      // the next() method must be called to retrieve the first reply.
      // The request handler must be kept alive while the iterator is used:
      // the iterator uses a byte buffer pool owned by the request handler.
      IByteBufferIteratorPtr replyToRequest( const ByteBuffer& requestBuffer )
      {
        return m_client.getRequestHandler()->replyToRequest( requestBuffer );
      }

    private:

      // Standard constructor is private
      SocketClientWrapper();

      // Copy constructor is private
      SocketClientWrapper( const SocketClientWrapper& rhs );

      // Assignment operator is private
      SocketClientWrapper& operator=( const SocketClientWrapper& rhs );

    private:

      // The SocketClient
      CoralSockets::SocketClient m_client;

    };

  }

}
#endif // CORALACCESS_SOCKETCLIENTWRAPPER_H
