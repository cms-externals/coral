// $Id: SslSocket.cpp,v 1.9.2.2 2011/09/16 13:40:52 avalassi Exp $
#ifdef HAVE_OPENSSL

// Include files
#include <iostream>
#include <sstream>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#ifdef HAVE_VOMS19
extern "C" {
#include <voms_apic_nog.h>
  //#include <vomsssl.h>
  extern int proxy_verify_callback_server(X509_STORE_CTX *ctx, void *empty);
  extern int proxy_verify_callback_client(int ok, X509_STORE_CTX *ctx);
  extern void setup_SSL_proxy_handler(SSL *ssl, char *cadir);
  extern void destroy_SSL_proxy_handler(SSL *);
}
#endif

#include "CoralSockets/GenericSocketException.h"
#include "SslSocket.h"

#define LOGGER_NAME "CoralSockets::SslSocket"
#include "logger.h"

//#undef DEBUG
//#define DEBUG( out )

// Namespace
using namespace coral::CoralSockets;

//---------------------------------------------------------------------------

SslPaths::SslPaths()
{
  // default paths currently point to my test certificates
  m_caCertPath ="glite/grid-security/certificates/";

  m_vomsPath = "glite/grid-security/vomsdir/";

  m_serverCert ="glite/grid-security/hostcert.pem";
  m_serverKey  ="glite/grid-security/hostkey.pem";

  // proxy certificates
  //   m_clientKey  ="glite/home/user_grid_proxy.pem";
  //   m_clientCert ="glite/home/user_grid_proxy.pem";

  m_clientKey  ="glite/home/user-voms.pem";
  m_clientCert ="glite/home/user-voms.pem";

  //   m_clientKey  ="glite/home/fake_grid_proxy.pem";
  //   m_clientCert ="glite/home/fake_grid_proxy.pem";

  // user certificates
  //   m_clientCert ="glite/home/usercert.pem";
  //   m_clientKey  ="glite/home/userkey.pem";

  // check environment variables which overide the defaults
  char *env=0;
  env = getenv( "X509_CERT_DIR");
  if ( env != 0 )
    m_caCertPath=std::string(env);

  env = getenv( "X509_VOMS_DIR");
  if ( env != 0 )
    m_vomsPath=std::string(env);

  env = getenv( "X509_USER_CERT");
  if ( env != 0 )
    m_clientCert=std::string(env);

  env = getenv( "X509_USER_KEY");
  if ( env != 0 )
    m_clientKey=std::string(env);

  env = getenv( "X509_USER_PROXY" );
  if ( env != 0 )
  {
    // user proxy overrides client key and cert
    m_clientCert = std::string(env);
    m_clientKey = std::string(env);
  }

  env = getenv( "X509_HOST_CERT");
  if ( env != 0 )
    m_serverCert=std::string(env);

  env = getenv( "X509_HOST_KEY");
  if ( env != 0 )
    m_serverKey=std::string(env);

}

//---------------------------------------------------------------------------

SslPaths& coral::CoralSockets::getSslPaths()
{
  static boost::mutex mutex;
  boost::mutex::scoped_lock lock(mutex);

  static SslPaths *mySslPaths= new SslPaths();
  if (mySslPaths == 0 )
    throw GenericSocketException("PANIC! Could no allocate memory for SslPaths",
                                 "SslPath::getSslPaths");
  return *mySslPaths;
}

//---------------------------------------------------------------------------

SslSocket::SslSocket( int fd, const std::string & desc )
  : TcpSocket( fd, desc )
  , m_sslCtx( 0 )
  , m_sslCon( 0 )
{
  INFO("SslSocket constructor '" << m_desc << "'");
}

//---------------------------------------------------------------------------

SslSocket::~SslSocket()
{
  INFO("SslSocket destructor '" << m_desc << "'");
}

//---------------------------------------------------------------------------

const coral::ICertificateData *SslSocket::getCertificateData() const
{
  DEBUG( m_desc << ": getCertificateData()  " << m_certificateData.get() );
  return m_certificateData.get();
};

//---------------------------------------------------------------------------

void initOpenSSL()
{
  // Initializing OpenSSL
  // FIXME should this only be called once?

  OpenSSL_add_all_algorithms();
  SSLeay_add_all_algorithms();
  SSL_load_error_strings();
  ERR_load_crypto_strings();
  // ERR_load_BIO_strings();

}

//----------------------------------------------------------------------

SslSocketPtr SslSocket::connectTo( const std::string& host, unsigned int port)
{
  initOpenSSL();

  // establish tcp connection
  std::string desc;
  int fd=TcpSocket::connectTo( desc, host, port );

  return connectTo( fd, desc );
}

//----------------------------------------------------------------------

SslSocketPtr SslSocket::secureSocketClient( TcpSocketPtr socket )
{
  std::string desc;
  int fd=socket->releaseFD();
  return connectTo( fd, socket->desc() );
}

//----------------------------------------------------------------------

SslSocketPtr SslSocket::connectTo( int fd, const std::string& desc )
{
  SslSocketPtr sslSocket( new SslSocket( fd, desc ) );

  sslSocket->m_sslCtx = SSL_CTX_new( SSLv3_method() );

  // set purpose to any for proxy certificates
  SSL_CTX_set_purpose(sslSocket->m_sslCtx, X509_PURPOSE_ANY);

  // load client certificate
  DEBUG("connect is using '"<< getSslPaths().clientCert() << "' as certificate");
  //  if ( SSL_CTX_use_certificate_file( sslSocket->m_sslCtx,
  //        getSslPaths().clientCert(), SSL_FILETYPE_PEM ) <= 0 )
  if ( SSL_CTX_use_certificate_chain_file( sslSocket->m_sslCtx,
                                           getSslPaths().clientCert() ) <= 0 )
  {
    ERR_print_errors_fp( stderr );
    throw GenericSocketException( "Error loading the client certificate!",
                                  "SslSocket::ConnectoTo" );
  }

  DEBUG("connect is using '"<< getSslPaths().clientKey() << "' as key");
  // load private key
  if ( SSL_CTX_use_PrivateKey_file( sslSocket->m_sslCtx,
                                    getSslPaths().clientKey(), SSL_FILETYPE_PEM) <= 0 )
  {
    ERR_print_errors_fp( stderr );
    throw GenericSocketException("Error loading client private key.",
                                 "SslSocket::connectTo");
  }

  // load trusted Certificate Authority
  DEBUG("caCert path :"<< getSslPaths().caCertPath() );
  if ( !SSL_CTX_load_verify_locations( sslSocket->m_sslCtx, 0,
                                       getSslPaths().caCertPath() ) )
  {
    ERR_print_errors_fp( stderr );
    throw GenericSocketException("Error CA verify locations.",
                                 "SslSocket::connectTo");
  }

#ifdef HAVE_VOMS19
  // set verify callback for proxy certificates
  SSL_CTX_set_verify( sslSocket->m_sslCtx,
                      SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                      proxy_verify_callback_client );
#endif

  // unset non-blocking mode ( otherwise the handshake is difficult)
  sslSocket->unsetNonBlocking();

  // create new ssl structure and pass the fd to it
  sslSocket->m_sslCon = SSL_new( sslSocket->m_sslCtx );
  if ( SSL_set_fd( sslSocket->m_sslCon, fd ) != 1 )
  {
    ERROR( "could not set file descriptor in connectTo :"
           << ERR_reason_error_string( ERR_get_error() ) );

  };

#ifdef HAVE_VOMS19
  // Eeek! const_cast, FIXME!
  setup_SSL_proxy_handler( sslSocket->m_sslCon,
                           const_cast<char*>( getSslPaths().caCertPath() ) );
#endif

  // initiate the handshake
  int error;
  if ( (error = SSL_connect( sslSocket->m_sslCon )) <= 0 )
  {
    //ERR_print_errors_fp( stderr );
    DEBUG( "got return code " << error << " from SSL_connect" );
    //std::cerr << "Error :" << ERR_reason_error_string(ERR_get_error()) << std::endl;

    std::stringstream str;
    str << "Error in SSL_connect: '"
        << ERR_reason_error_string( ERR_get_error() ) << "'";

    throw SslHandshakeException( str.str(), "SslSocket::connectTo");
  }

  X509 * x509Cert=0;
  if ( (x509Cert=SSL_get_peer_certificate( sslSocket->m_sslCon )) == 0 )
  {
    throw GenericSocketException( "could not get remote certificate!",
                                  "SslSocket::connectTo" );
  };

  if ( SSL_get_verify_result( sslSocket->m_sslCon ) != X509_V_OK )
  {
    std::cerr << "Server verification has failed" << std::endl;
    // free the certificate
    X509_free( x509Cert );

    throw GenericSocketException( "Server verification has failed",
                                  "SslSocket::connectTo()" );
  };

  //get the common name
  char peerCn[256]="";
  X509_NAME_get_text_by_NID( X509_get_subject_name( x509Cert ),
                             NID_commonName,
                             peerCn,
                             256);

  DEBUG( "Remote certificate common name: " << peerCn );
  char peerDn[256]="";
  X509_NAME_oneline( X509_get_subject_name(x509Cert), peerDn, sizeof( peerDn ) );
  DEBUG( "Remote certificate distinguished name: " << peerDn );
  // FIXME check peerCn against host?

  // free the certificate
  X509_free( x509Cert );

  // set non-blocking mode
  sslSocket->setNonBlocking();

  return sslSocket;
}

//----------------------------------------------------------------------

SslSocketPtr SslSocket::listenOn( const std::string& ip, unsigned int port)
{
  initOpenSSL();

  std::string desc;
  int fd = TcpSocket::listenOn( desc, ip, port );
  return SslSocketPtr( new SslSocket( fd, desc ) );
}

//----------------------------------------------------------------------

const std::string getErrorStr( int errorcode) {
  switch ( errorcode ) {
  case SSL_ERROR_NONE:
    return std::string("SSL_ERROR_NONE");
    break;
  case SSL_ERROR_ZERO_RETURN:
    return std::string(" underlying socket closed?");
    break;
  case SSL_ERROR_WANT_READ:
  case SSL_ERROR_WANT_WRITE:
    return std::string("EAGAIN?");
    break;
  case SSL_ERROR_WANT_CONNECT:
  case SSL_ERROR_WANT_ACCEPT:
    return std::string( "Operation did not complete, still wants to CONNECT"
                        " or ACCEPT." );
    break;
  case SSL_ERROR_WANT_X509_LOOKUP:
    return std::string("Operation did not complete call again.");
    break;
  case SSL_ERROR_SYSCALL:
    {
      std::stringstream stream;
      stream << "SSL Error Syscall: ";
      if (ERR_get_error() != 0 )
        stream << " err_get_error(): "
               << ERR_reason_error_string( ERR_get_error() ) << "'";
      else
        stream << " errno: " << errno << " '" << strerror(errno) << "'";
      return std::string( stream.str() );
    }
    break;
  case SSL_ERROR_SSL:
    {
      std::stringstream stream;
      stream << "SSL Error: '"
             << ERR_reason_error_string( ERR_get_error() ) << "'";
      return std::string( stream.str() );
    }
    break;
  default:
    std::stringstream stream;
    stream << "Unknown error: " << errorcode;
    return stream.str();
  };
  return std::string("unknown");
}

std::string getError()
{
  std::stringstream error;
  // print errors
  unsigned long l;
  char buf[256];
#if SSLEAY_VERSION_NUMBER  >= 0x00904100L
  const char *file;
#else
  char *file;
#endif
  char *dat;
  int line;
  while ( ERR_peek_error() != 0 )
  {
    int i;
    ERR_STATE *es;

    es = ERR_get_state();
    i = (es->bottom+1)%ERR_NUM_ERRORS;

    if (es->err_data[i] == NULL)
      dat = (char*)"";
    else
      dat = es->err_data[i];
    if (dat)
    {
      l = ERR_get_error_line(&file, &line);
      //      if (debug)
      error << ERR_error_string(l, buf)
            << ": " << file
            << ", " << line
            << ", " << dat;

      //fprintf(stderr, "%s:%s,%d,%s\n", ERR_error_string(l, buf),
      //        file, line, dat);
      //      error += std::string(ERR_reason_error_string(l)) + ":" + std::string(ERR_func_error_string(l)) + "\n";
    }
  }
  return error.str();
}

//----------------------------------------------------------------------

SslSocketPtr SslSocket::secureSocketServer( TcpSocketPtr socket )
{
  std::string desc;
  int fd=socket->releaseFD();
  return acceptOn( fd, socket->desc() );
}

//----------------------------------------------------------------------

SslSocketPtr SslSocket::accept()
{
  std::string desc;
  int fd=TcpSocket::accept( desc );
  return acceptOn( fd, desc );
}

//----------------------------------------------------------------------

SslSocketPtr SslSocket::acceptOn( int fd, const std::string& desc )
{
  SslSocketPtr sslSocket( new SslSocket( fd, desc ) );

  sslSocket->m_sslCtx = SSL_CTX_new( SSLv3_method() );

  // set certificate purpose to any, needed for proxy certificates
  SSL_CTX_set_purpose(sslSocket->m_sslCtx, X509_PURPOSE_ANY);

  // load server certificate
  if ( SSL_CTX_use_certificate_file( sslSocket->m_sslCtx,
                                     getSslPaths().serverCert(),
                                     SSL_FILETYPE_PEM )
       <= 0 )
  {
    ERR_print_errors_fp( stderr );
    throw GenericSocketException("Error loading the server certificate!",
                                 "SslSocket::accept");
  }

  // load private key
  if ( SSL_CTX_use_PrivateKey_file( sslSocket->m_sslCtx,
                                    getSslPaths().serverKey(),
                                    SSL_FILETYPE_PEM)
       <= 0 )
  {
    ERR_print_errors_fp( stderr );
    throw GenericSocketException("Error loading server private key.",
                                 "SslSocket::accept");
  }

  // load trusted Certificate Authority
  if ( !SSL_CTX_load_verify_locations( sslSocket->m_sslCtx, 0,
                                       getSslPaths().caCertPath() ) )
  {
    ERR_print_errors_fp( stderr );
    throw GenericSocketException( "Error loading CA verify locations.",
                                  "SslSocket::accept");
  }

  // require peer (client) certificate verification
  SSL_CTX_set_verify( sslSocket->m_sslCtx, SSL_VERIFY_PEER, 0 );
#ifdef HAVE_VOMS19
  // Set the verification depth to 100 for proxy certificates
  SSL_CTX_set_verify_depth( sslSocket->m_sslCtx, 100 );
  // set the verification callback for proxy certificates
  SSL_CTX_set_cert_verify_callback( sslSocket->m_sslCtx,
                                    proxy_verify_callback_server, 0);
#endif

  // unset non-blocking mode ( otherwise the handshake is difficult)
  sslSocket->unsetNonBlocking();

  // create new ssl structure and pass the fd to it
  sslSocket->m_sslCon = SSL_new( sslSocket->m_sslCtx );
  if ( SSL_set_fd( sslSocket->m_sslCon, fd ) != 1 )
  {
    ERROR( sslSocket->m_desc << ": could not set file descriptor in accept :"
           << ERR_reason_error_string( ERR_get_error() ) );
    ERR_print_errors_fp( stderr );
  };

  // initiate the handshake
  int error;
  if ( (error = SSL_accept( sslSocket->m_sslCon )) <= 0 )
  {
    DEBUG( sslSocket->m_desc << ": got return code " << error << " from SSL_accept");
    std::stringstream str;
    str << "Error in SSL_accept: '" << ERR_reason_error_string( ERR_get_error() )
        << "'";
    throw SslHandshakeException( str.str(), "SslSocket::accept()" );
  }

  X509 * x509Cert=0;
  if ( ( x509Cert = SSL_get_peer_certificate( sslSocket->m_sslCon  )) == 0 )
  {
    throw GenericSocketException( "could not get remote certificate!",
                                  "SslSocket::accept");
  };
#if 0
  int extCount=X509_get_ext_count(x509Cert);
  std::cerr << "extension count by opensssl: "<< extCount  << std::endl;
  for (int j = 0; j < extCount; j++) {
    X509_EXTENSION *ext = X509_get_ext(x509Cert, j);
    X509V3_EXT_print_fp(stderr, ext, 0, 0 );
    std::cerr << std::endl;
  };
#endif

  if ( SSL_get_verify_result( sslSocket->m_sslCon ) != X509_V_OK )
  {
    ERROR( sslSocket->m_desc << ": Client verification has failed." );
    // free the certificate
    X509_free( x509Cert );

    throw GenericSocketException( "Client verification has failed",
                                  "SslSocket::accept");
  };


  // get the peer cert chain
  STACK_OF(X509) *x509Chain=0;
  if ( (x509Chain = SSL_get_peer_cert_chain( sslSocket->m_sslCon) ) == 0 )
  {
    // free the certificate
    X509_free( x509Cert );

    throw GenericSocketException( "could not get remote certificate chain!",
                                  "SslSocket::accept" );
  };

#ifdef HAVE_VOMS19
  // retrieve vo data
  struct vomsdata *vd  = 0;
  struct voms **volist = 0;

  // Eeek! const_cast :-( FIXME!
  if ( (vd = VOMS_Init( const_cast<char*>(getSslPaths().vomsPath()),
                        const_cast<char*>(getSslPaths().caCertPath()) ) ) == 0)
  {
    throw GenericSocketException( "could not init voms", "SslSocket::accept" );
  }

  error=0;
  if ( ( VOMS_Retrieve( x509Cert, x509Chain, RECURSE_CHAIN,
                        vd, &error) == 0)
       && ( error != VERR_NOEXT ) )
  {
    char buffer[120];
    VOMS_ErrorMessage(vd, error, buffer, sizeof( buffer ) );
    std::stringstream str;
    str << "Error in VOMS_Retrieve: '" << buffer << "'";
    throw GenericSocketException( str.str(), "SslSocket::accept" );
  }

  sslSocket->m_certificateData = std::auto_ptr<CertificateData>( new CertificateData() );
  sslSocket->m_certificateData->setDistinguishedName( "FIXME");

  volist = vd->data;

  if ( volist != 0 )
  {
    int i = 0;
    //int nbfqan;

    if ( (*volist)->voname != 0 )
    {
      DEBUG( sslSocket->m_desc << ": voname :" << (*volist)->voname );
    }

    while( volist[0]->fqan[i] != 0 )
    {
      DEBUG( i << ". fqan: " << volist[0]->fqan[i] );
      std::stringstream fqan;
      fqan <<  volist[0]->fqan[i];
      sslSocket->m_certificateData->addFQAN( fqan.str() );
      i++;
    }

  }
  else DEBUG("volist empty!");

  //std::cerr << VOMS_ListTargets(vd,&error) << " Error " << error << std::endl;


  //get the common name
  char peerCn[256]="";
  X509_NAME_get_text_by_NID( X509_get_subject_name( x509Cert ),
                             NID_commonName,
                             peerCn,
                             sizeof(peerCn) );

  INFO( sslSocket->m_desc << ": Remote certificate common name: " << peerCn );

  char peerDn[256]="";
  X509_NAME_oneline( X509_get_subject_name( x509Cert ), peerDn, sizeof( peerDn ) );
  INFO( sslSocket->m_desc << ": Remote certificate distinguished name: " << peerDn );


  // FIXME check peerCn against host?
  std::stringstream peerDnStream;
  peerDnStream << peerDn;
  sslSocket->m_certificateData->setDistinguishedName( peerDnStream.str() );
  DEBUG( sslSocket->m_desc << ": ssl certificate data "
         << sslSocket->m_certificateData.get() );
#endif

  // free the certificate
  X509_free( x509Cert );

  // set non-blocking mode
  sslSocket->setNonBlocking();
  return sslSocket;
}

//----------------------------------------------------------------------

size_t SslSocket::read_write( PMode mode,
                              unsigned char *buf,
                              size_t len,
                              int timeout )
{
  if ( mode != P_READ && mode != P_WRITE )
    throw GenericSocketException( "Panic! Wrong mode in read_write!" );

  const char *label = mode==P_READ ? "SslSocket::read" : "SslSocket::write";

  DEBUG( label << ":" << m_desc << " enter read write timeout " << timeout);

  if (m_fd < 0 )
    throw SocketClosedException( label );

  unsigned char *p=buf;
  size_t read=0;
  while ( len > read )
  {
    DEBUG( label << ":" << m_desc
           << " len " << len
           << " read " << read
           << " p " << (long)p );

    if (timeout < 0 )
      // no timeout set, we sleep until data is available,
      // but don't return unless we got all data
      poll( mode, 200 );
    else if ( !poll( mode, timeout ) )
    {
      DEBUG( label << ": " << m_desc << " timeout" );
      // timeout occured, return
      break;
    };

    // has the socket been closed?
    if (m_fd < 0 )
      throw SocketClosedException( label );

    ssize_t nResult=0;
    {
      boost::mutex::scoped_lock lock( m_sslMutex);
      if ( mode == P_READ)
        nResult = SSL_read( m_sslCon, p, len-read );
      else
        nResult = SSL_write( m_sslCon, p, len-read );
    };

    if ( nResult < 0 ) {
      DEBUG( label << ":" << m_desc
             << " nResult < 0 " << errno
             << " " << strerror(errno) );

      int SSL_error = SSL_get_error( m_sslCon, nResult );
      if ( SSL_error == SSL_ERROR_WANT_READ ||
           SSL_error == SSL_ERROR_WANT_WRITE )
      {
        // retry, we have to call SSL_read or SSL_write with the same arguments again

        DEBUG( label << ":" << m_desc
               <<" SSL_ERROR_WANT_READ this should not happen?"  );
        // no data available
        // as long as we poll, this shoud not happen...
        //if (timeout == 0)
        //        break;
        continue;
      }
      else if ( SSL_error == SSL_ERROR_SYSCALL && errno == EAGAIN )
      {
        DEBUG( label << ":" << m_desc
               <<" SSL_ERROR_SYSCALL & EAGAIN this should not happen?"  );
        // no data available
        // as long as we poll, this shoud not happen...
        //if (timeout == 0)
        //        break;
        continue;
      };

      // not sure about these:
      if ( errno == EAGAIN || errno ==EINTR )
        continue;

      // hmm, seems to be something serious
      std::stringstream stream;
      stream << "PANIC! unhandled error Result<0: "<< getErrorStr( SSL_error );
      throw GenericSocketException( stream.str(), label);
      //      throw GenericSocketException( ERR_reason_error_string(ERR_get_error())
      //         , label);
    };

    if ( nResult == 0 )
    {
      // connection lost
      // in case we already read something return what we have
      if ( read != 0 )
      {
        DEBUG( label << ":" << m_desc
               << " nrecv return 0, but we already received something, so"
               " I'm returning that." );
        break;
      };
      DEBUG( label << ":" << m_desc << " nrecv return 0 " );
      close();
      throw SocketClosedException( label );
    };

    read+=nResult;
    p+=nResult;
  };

  DEBUG( label << ":" << m_desc << " leave read write return:" << read );
  return (size_t) read;
}

#endif
