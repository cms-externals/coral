// $Id: SslSocket.h,v 1.5.2.3 2011/09/16 13:40:52 avalassi Exp $
#ifndef CORALSOCKETS_SSLSOCKET_H
#define CORALSOCKETS_SSLSOCKET_H 1


// OpenSSL headers
#if 1
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#if 0
#include <globus_openssl.h>
#include <globus_gsi_callback.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif



#include <boost/shared_ptr.hpp>

#include "CoralBase/boost_thread_headers.h"
#include "CertificateData.h"


#include "TcpSocket.h"
//#include "my_gridsite.h"

namespace coral {

  class SslSocketTest; // forward definition of the test class

  // forward definition
  class ICertificateData;

  namespace CoralSockets {

    // forward declaration
    class SslSocket;

    typedef boost::shared_ptr<SslSocket> SslSocketPtr;

    /// a class which handles the paths to ssl certificates etc.
    class SslPaths {
      friend class coral::SslSocketTest; // has to modify the paths for tests

    public:
      // constructor
      SslPaths();

      const char* caCertPath()
      {
        return m_caCertPath.c_str();
      }

      const char* vomsPath()
      {
        return m_vomsPath.c_str();
      }

      const char* serverCert() const
      {
        return m_serverCert.c_str();
      }

      const char* serverKey() const
      {
        return m_serverKey.c_str();
      }

      const char* clientCert() const
      {
        return m_clientCert.c_str();
      }

      const char* clientKey() const
      {
        return m_clientKey.c_str();
      }

    private:
      // Not defined, to prevent copying
      SslPaths(const SslPaths& );
      SslPaths& operator =(const SslPaths& other);

      std::string m_caCertPath;
      std::string m_vomsPath;
      std::string m_serverCert;
      std::string m_serverKey;
      std::string m_clientKey;
      std::string m_clientCert;
    };

    SslPaths& getSslPaths();

    class SslSocket : virtual public TcpSocket {
    public:

      /// create new socket connected to remote host
      /// throws exceptions on errors
      static SslSocketPtr connectTo( const std::string& host, unsigned int port);

      /// create a listening socket
      /// throws exceptions on errors
      static SslSocketPtr listenOn( const std::string& ip, unsigned int port);

      /// accept new connections on listening sockets
      SslSocketPtr accept();

      static SslSocketPtr secureSocketServer( TcpSocketPtr socket );

      static SslSocketPtr secureSocketClient( TcpSocketPtr socket );

      /// returns the data contained in the certificate
      /// the data has already been validated and can be trusted
      const coral::ICertificateData *getCertificateData() const;

      /// closes the socket
      //virtual void close();

      virtual ~SslSocket();

    private:
      SslSocket( int socket, const std::string& desc);

      /// standard constructor is private
      SslSocket();
      /// copy constructor is private
      SslSocket(const SslSocket& );
      /// assignment operator as well
      SslSocket& operator=(const SslSocket& );

      virtual size_t read_write( PMode mode,
                                 unsigned char* buf,
                                 size_t len,
                                 int timeout );

      static SslSocketPtr connectTo( int fd, const std::string& desc );

      static SslSocketPtr acceptOn( int fd, const std::string& desc );

    protected:
      /// the SSL context
      SSL_CTX * m_sslCtx;

      /// the SSL connection structure
      SSL * m_sslCon;

      /// a mutex for SSL_read() and SSL_write(), they are not thread safe!
      mutable boost::mutex m_sslMutex;

      /// the remote certificate data
      std::unique_ptr<CertificateData> m_certificateData;
    };

    class SslHandshakeException : public Exception
    {
    public:
      /// Constructor
      SslHandshakeException( const std::string& reason = "",
                             const std::string& methodName = "" )
        : Exception( reason , methodName, "coral::CoralSockets" ) {}

      /// Destructor
      virtual ~SslHandshakeException() throw() {}

    };

  }
}

#endif // CORALSOCKETS_TCPSOCKET_H
