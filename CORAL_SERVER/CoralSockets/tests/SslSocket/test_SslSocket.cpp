// $Id: test_SslSocket.cpp,v 1.5.2.5 2011/09/16 15:33:45 avalassi Exp $

// Include files
#include <iostream>
#include <sys/time.h> // for gettimeof day debug
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralSockets/GenericSocketException.h"

// Local include files
#ifdef HAVE_OPENSSL
#include "../../src/SslSocket.h"
#endif
#include "../../src/ThreadManager.h"

// port used for testing
int testPort = 50007;

#ifdef HAVE_OPENSSL
const unsigned char testMessage[] ="0123456789012345789";
const int testMessageLen = sizeof( testMessage ) / sizeof( testMessage[0] );

// set up grid environment (on LxPlus)
// source /afs/cern.ch/project/gd/LCG-share/current/etc/profile.d/grid_env.sh
// base dir to test certs (generated with glite-test-certs --fake --localhost)
// ./glite-test-certs --env --force --wrong --localhost --client=mwache --voms="test_fqdn_voms"
// source home/env_settings.sh
// X509_USER_PROXY=home/user-voms.pem voms-proxy-info -all'
const char testCertDir[]="glite/";
//const char testCertDir[]="/home/wache/Projects/Atlas/myLCG/coral-cvs/CORAL_SERVER/CoralSockets/cmt/glite/";

const char userCert[] = "home/usercert.pem";
const char userKey[]  = "home/userkey.pem";

const char fakeUserCert[] = "home/fakecert.pem";
const char fakeUserKey[]  = "home/fakekey.pem";
const char fakeProxy[]    = "home/fake_grid_proxy.pem";

static unsigned char tcpRequest[]="this is raw tcp";
static unsigned char tcpAnswer[]="this is a raw tcp answer";
#endif

// Namespace
using namespace coral::CoralSockets;

namespace coral
{

  uint64_t getTimeMS() {
    timeval val;
    gettimeofday( &val, 0);
    return (uint64_t) val.tv_usec/1000 + (uint64_t) val.tv_sec*1000;
  }

  class SslSocketTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( SslSocketTest );
#ifdef HAVE_OPENSSL
    CPPUNIT_TEST( test_basic );
    CPPUNIT_TEST( test_read_write );
    CPPUNIT_TEST( test_certificateData );
    CPPUNIT_TEST( test_secureTcpSocket );
    CPPUNIT_TEST( test_usercert );
    CPPUNIT_TEST( test_fakeproxy );
    CPPUNIT_TEST( test_fakecert );
#endif
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {
#ifdef HAVE_OPENSSL
    retry:
      try {
        TcpSocketPtr listenSocket   = TcpSocket::listenOn( "localhost", testPort );
      }
      catch (GenericSocketException &e) {
        std::string expMsg="Error binding socket: Address already in use";
        if ( expMsg == std::string( e.what(), expMsg.size() ) ) {
          testPort+=1;
          //std::cout << "skipping port " << testPort << std::endl;
          if (testPort > 50100)
            throw;
          goto retry;
        }
        else throw;
      };
      //    std::cout << "starting teardown sleep" << std::endl;
      //sleep(40); // to make sure old connnections are properly closed
#endif
    }

#ifdef HAVE_OPENSSL
    class DummySslConnector : public Runnable {
    public:
      std::string exceptionMessage;
      void endThread()
      {};

      const std::string desc() const
      {
        return std::string("DummySslConnector");
      };

      void operator()()
      {
        try {
          SslSocketPtr connectSocket1 = SslSocket::connectTo( "localhost",
                                                              testPort );
          connectSocket1->write( testMessage, testMessageLen );

          {
            unsigned char receiveBuffer[100];
            int nRead=connectSocket1->read( receiveBuffer, testMessageLen,
                                            -1 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 2", nRead,
                                          testMessageLen );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 2", memcmp(
                                                                      receiveBuffer, testMessage, testMessageLen) , 0 );
          }
          connectSocket1->close();
        }
        catch (GenericSocketException &e) {
          std::stringstream str;
          str <<  "GenericSocketException: " << e.what() << std::endl;
          exceptionMessage = str.str();
        }
        catch (SslHandshakeException &e) {
          std::stringstream str;
          str <<  "SslHandshakeException: " << e.what() << std::endl;
          exceptionMessage = str.str();
        }
        catch (...) {
          std::stringstream str;
          str <<  "Unknown exception!" << std::endl;
          exceptionMessage = str.str();
        };
      };
    };

    // ------------------------------------------------------

    void test_basic()
    {
      // some basic connection tests

      ThreadManager thrManager;
      SslSocketPtr listenSocket   = SslSocket::listenOn( "localhost", testPort );

      DummySslConnector *connector1 = new DummySslConnector();
      thrManager.addThread( connector1 );

      // wait until thread wants to connect
      listenSocket->poll( TcpSocket::P_READ, -1 );
      SslSocketPtr acceptSocket1  = listenSocket->accept();

      // write to one end and receive on the other
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket1->read( receiveBuffer, testMessageLen, -1 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 1", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 1", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket1->write( testMessage, testMessageLen );

      // establish another connection
      DummySslConnector *connector2 = new DummySslConnector();
      thrManager.addThread( connector2 );

      // wait until thread wants to connect
      listenSocket->poll( TcpSocket::P_READ, -1 );
      SslSocketPtr acceptSocket2  = listenSocket->accept();

      // recieve
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket2->read( receiveBuffer, testMessageLen, -1 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 3", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 3", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket2->write( testMessage, testMessageLen );
      thrManager.joinAll();

      // no exceptions on the other end
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "connector1 exception string",
                                    std::string(""), connector1->exceptionMessage );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "connector2 exception string",
                                    std::string(""), connector2->exceptionMessage );

#if 0
      // close one end and try to write to other end
      connectSocket1->close();
      sleep(1);
      try {
        int i=0;
        while (i<1000) {
          acceptSocket1->write( testMessage, testMessageLen);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException &e) {
        // ok
      };
      // reading should fail as well
      try {
        int i=0;
        while (i<1000) {
          unsigned char receiveBuffer[100];
          acceptSocket1->read( receiveBuffer, 100);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException &e) {
        // ok
      };


      // check if other connection is still alive
      // write to one end
      connectSocket2->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket2->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 4", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 4", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket2->write( testMessage, testMessageLen );
      sleep(1);
      {
        unsigned char receiveBuffer[100];
        int nRead=connectSocket2->read( receiveBuffer, sizeof( receiveBuffer ) / sizeof( receiveBuffer[0] ), 200 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 5", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 5", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }

      // this time we close the other end
      // close one end and try read or write to other end
      acceptSocket2->close();
      sleep(1);
      // reading should fail as well
      try {
        int i=0;
        while (i<1000) {
          unsigned char receiveBuffer[100];
          connectSocket2->read( receiveBuffer, 100);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException &e) {
        // ok
      };
      // writing should fail
      try {
        int i=0;
        while (i<1000) {
          connectSocket2->write( testMessage, testMessageLen);
          i++;
        };
        CPPUNIT_FAIL("SocketClosedException expected");
      }
      catch (SocketClosedException &e) {
        // ok
      };
#endif
    }

    //------------------------------------------------------------------------

#define  largePacketLen 3000000
    //3M*4 bytes=12 M bytes

    // sender thread for tests
    class senderClass : public Runnable {

    public:
      std::string exceptionMessage;

      void endThread()
      {}

      const std::string desc() const
      {
        return std::string("SenderClass");
      }


      void operator()() {
        int *sendbuf = new int[ largePacketLen ];
        try {
          for ( int i=0; i< largePacketLen; i++)
            sendbuf[i]=i;

          SslSocketPtr connect=SslSocket::connectTo("localhost", testPort );

          connect->writeAll( (unsigned char*) sendbuf,
                             largePacketLen *sizeof( sendbuf[0] ) );

        }
        catch (GenericSocketException &e) {
          std::stringstream str;
          str <<  "GenericSocketException: " << e.what() << std::endl;
          exceptionMessage = str.str();
        }
        catch (SslHandshakeException &e) {
          std::stringstream str;
          str <<  "SslHandshakeException: " << e.what() << std::endl;
          exceptionMessage = str.str();
        }
        catch (Exception &e) {
          std::stringstream str;
          str <<  "Exception: " << e.what() << std::endl;
          exceptionMessage = str.str();
          std::cout << "Caught  Exception " << std::endl;
        }
        catch (...) {
          std::stringstream str;
          str <<  "Unknown exception!" << std::endl;
          exceptionMessage = str.str();
        };
        delete[] sendbuf;
      };
    };

    void test_read_write()
    {
#if 0
      // establish a connection
      TcpSocketPtr listen=TcpSocket::listenOn( "localhost", testPort );
      TcpSocketPtr connect=TcpSocket::connectTo( "localhost", testPort );
      TcpSocketPtr accept=listen->accept();
      uint64_t mTime;
      const int len = 3000;
      unsigned char buf[len];

      // check read timeout
      mTime = getTimeMS();
      int res=accept->read( buf, len, 200);
      mTime = getTimeMS() - mTime;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read timeout 1", res == 0, true);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read timeout 2", mTime > 150, true);

      // FIXME how to test write timeout? Is this necessary?

      // write several small pieces and receive a large chunk
      unsigned char test[ len ];
      for (int i=0; i<len; i++)
        test[i]=(unsigned char) ( i & 0xff);

      for (int i=0; i<10; i++)
        connect->writeAll( &test[ i*len/10 ], len/10 );

      unsigned char rec[ len ];
      int nread = accept->read( rec, len, 100);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read 1", nread, len );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("cmp 1", memcmp( test, rec, len ), 0 );

      // write one big chunk and receive several small ones
      connect->writeAll( &test[0], len );

      nread=0;
      for ( int i = 0; i < 10; i++)
        nread+=accept->read( &rec[ i*len/10 ], len/10, 100 );
      // the buffers should be full, so we assume we can read len/10 every time
      CPPUNIT_ASSERT_EQUAL_MESSAGE("read 2", nread, len );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("cmp 2 ", memcmp( test, rec, len ), 0 );
#endif
      // write/receive very large data sets (this needs threads)
      {
        SslSocketPtr listen=SslSocket::listenOn( "localhost", testPort );
        ThreadManager thrManager;
        thrManager.addThread( new senderClass() );

        listen->poll(TcpSocket::P_READ, -1); // wait until the thread has connected
        SslSocketPtr accept1 = listen->accept();

        int *recbuf= new int[ largePacketLen ];
        memset( (unsigned char*)recbuf, 0, largePacketLen * sizeof( recbuf[0] ) );
        accept1->readAll( (unsigned char*)recbuf, largePacketLen * sizeof( recbuf[ 0 ] ) );

        for ( int i=0; i< largePacketLen ; i++)
          if ( recbuf[i] != i )
            CPPUNIT_FAIL("Large send buffer mismatch");
        delete[] recbuf;
        thrManager.joinAll();
      };


    };

    //---------------------------------------------------------------------

    // raw/secure sender thread for tests
    class secureSenderClass : public Runnable {

    public:
      std::string exceptionMessage;

      void endThread()
      {}

      const std::string desc() const
      {
        return std::string("secureSenderClass");
      }


      void operator()()
      {
        int *sendbuf = new int[ largePacketLen ];
        try {
          for ( int i=0; i< largePacketLen; i++)
            sendbuf[i]=i;

          TcpSocketPtr connect=TcpSocket::connectTo("localhost", testPort );

          connect->writeAll( tcpRequest, sizeof( tcpRequest ) );

          unsigned char dummy[ 1000 ];
          connect->readAll( dummy, sizeof( tcpAnswer ) );

          SslSocketPtr connectS=SslSocket::secureSocketClient( connect );

          // send through secure socket
          connectS->writeAll( (unsigned char*) sendbuf,
                              largePacketLen *sizeof( sendbuf[0] ) );

        }
        catch (GenericSocketException &e) {
          std::stringstream str;
          str <<  "GenericSocketException: " << e.what() << std::endl;
          exceptionMessage = str.str();
        }
        catch (SslHandshakeException &e) {
          std::stringstream str;
          str <<  "SslHandshakeException: " << e.what() << std::endl;
          exceptionMessage = str.str();
        }
        catch (Exception &e) {
          std::stringstream str;
          str <<  "Exception: " << e.what() << std::endl;
          exceptionMessage = str.str();
          std::cout << "Caught  Exception " << std::endl;
        }
        catch (...) {
          std::stringstream str;
          str <<  "Unknown exception!" << std::endl;
          exceptionMessage = str.str();
        };
        delete[] sendbuf;
      }
    };

    void test_secureTcpSocket()
    {
      //  (this needs threads)
      {
        // start with a raw tcp socket
        TcpSocketPtr listen=SslSocket::listenOn( "localhost", testPort );
        ThreadManager thrManager;
        thrManager.addThread( new secureSenderClass() );

        listen->poll(TcpSocket::P_READ, -1); // wait until the thread has connected
        TcpSocketPtr accept = listen->accept();

        // read request
        unsigned char dummy[ 1000 ];
        accept->readAll( dummy, sizeof( tcpRequest ) );

        CPPUNIT_ASSERT_EQUAL( std::string( (char*)dummy ), std::string( (char*)tcpRequest ) );

        accept->writeAll( tcpAnswer, sizeof( tcpAnswer ) );

        SslSocketPtr accept1( SslSocket::secureSocketServer( accept ) );

        int *recbuf= new int[ largePacketLen ];
        memset( (unsigned char*)recbuf, 0, largePacketLen * sizeof( recbuf[0] ) );
        accept1->readAll( (unsigned char*)recbuf, largePacketLen * sizeof( recbuf[ 0 ] ) );

        for ( int i=0; i< largePacketLen ; i++)
          if ( recbuf[i] != i )
            CPPUNIT_FAIL("Large send buffer mismatch");
        delete[] recbuf;
        thrManager.joinAll();
      };


    };

    // ------------------------------------------------------

    void test_usercert()
    {
      // the default is to use the proxy certs, now try with usercert.pem
      // and userkey.pem (which is actually simpler, since it is supported by
      // native openssl)

      // set path
      SslPaths &path=getSslPaths();
      std::stringstream path_str;
      path_str << testCertDir << userKey;
      path.m_clientKey = path_str.str();

      std::stringstream path_str2;
      path_str2 << testCertDir << userCert;
      path.m_clientCert = path_str2.str();

      ThreadManager thrManager;
      SslSocketPtr listenSocket   = SslSocket::listenOn( "localhost", testPort );

      DummySslConnector *connector = new DummySslConnector();
      thrManager.addThread( connector );

      // wait until thread wants to connect
      listenSocket->poll( TcpSocket::P_READ, -1 );
      SslSocketPtr acceptSocket1  = listenSocket->accept();

      // write to one end and receive on the other
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket1->read( receiveBuffer, testMessageLen, -1 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 1", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 1", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket1->write( testMessage, testMessageLen );

      thrManager.joinAll();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("senderClass exception",
                                   std::string(""), connector->exceptionMessage );
    };

    // ------------------------------------------------------

    void test_fakeproxy()
    {

      // check if the connection fails with fake proxy certs

      // set path
      SslPaths &path=getSslPaths();
      std::stringstream path_str;
      path_str << testCertDir << fakeProxy;
      path.m_clientKey = path_str.str();
      path.m_clientCert = path_str.str();

      ThreadManager thrManager;
      SslSocketPtr listenSocket   = SslSocket::listenOn( "localhost", testPort );

      DummySslConnector *connector = new DummySslConnector();
      thrManager.addThread( connector );

      // wait until thread wants to connect
      listenSocket->poll( TcpSocket::P_READ, -1 );
      SslSocketPtr acceptSocket1;
      try {
        acceptSocket1  = listenSocket->accept();
        CPPUNIT_FAIL("Error in accept exception expected");
      }
      catch (SslHandshakeException &e) {
        // the actual error messages vary wildly between versions, so I can't easily check
        // better :-(
      }
      thrManager.joinAll();

      std::string expMsg="SslHandshakeException: Error in SSL_connect: ";
      //"'sslv3 alert bad certificate'";
      //"'tlsv1 alert unknown ca'";
      //"'no certificate returned'";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("senderClass exception",
                                   expMsg, std::string( connector->exceptionMessage.c_str(),
                                                        expMsg.size() ) );
    };

    // ------------------------------------------------------

    void test_fakecert()
    {

      // check if the connection fails with fake  user certs+key

      // set path
      SslPaths &path=getSslPaths();
      std::stringstream path_str;
      path_str << testCertDir << fakeUserKey;
      path.m_clientKey = path_str.str();
      std::stringstream path_str1;
      path_str1 << testCertDir << fakeUserCert;
      path.m_clientCert = path_str1.str();

      ThreadManager thrManager;
      SslSocketPtr listenSocket   = SslSocket::listenOn( "localhost", testPort );

      DummySslConnector *connector = new DummySslConnector();
      thrManager.addThread( connector );

      // wait until thread wants to connect
      listenSocket->poll( TcpSocket::P_READ, -1 );
      SslSocketPtr acceptSocket1;
      try {
        acceptSocket1  = listenSocket->accept();
        CPPUNIT_FAIL("Error in accept exception expected");
      }
      catch (SslHandshakeException &e) {
        // the actual error messages vary wildly between versions, so I can't easily check
        // better :-(
      }
      thrManager.joinAll();

      std::string expMsg="SslHandshakeException: Error in SSL_connect: ";
      //"'tlsv1 alert unknown ca'";
      //"'sslv3 alert bad certificate'";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("senderClass exception",
                                   expMsg, std::string( connector->exceptionMessage.c_str(),
                                                        expMsg.size() ) );
    };

    // ------------------------------------------------------

    void test_certificateData()
    {
      // some tests if we get the correct certificate data

      ThreadManager thrManager;
      SslSocketPtr listenSocket   = SslSocket::listenOn( "localhost", testPort );

      DummySslConnector *connector1 = new DummySslConnector();
      thrManager.addThread( connector1 );

      // wait until thread wants to connect
      listenSocket->poll( TcpSocket::P_READ, -1 );
      SslSocketPtr acceptSocket1  = listenSocket->accept();

      const ICertificateData * cert=acceptSocket1->getCertificateData();
      CPPUNIT_ASSERT_EQUAL_MESSAGE("dn 1", cert->distinguishedName(),
                                   std::string("/C=UG/L=Tropic/O=Utopia/OU=Relaxation/CN=mwache/CN=proxy") );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("FQANs vector length", cert->FQANs().size(),
                                   (size_t)1 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE("FQANs 1 ",  cert->FQANs()[0],
                                   std::string("test_fqdn_voms") );

      // write to one end and receive on the other
      {
        unsigned char receiveBuffer[100];
        int nRead=acceptSocket1->read( receiveBuffer, testMessageLen, -1 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message length 1", nRead,
                                      testMessageLen );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "message content 1", memcmp(
                                                                  receiveBuffer, testMessage, testMessageLen) , 0 );
      }
      // write to the other end
      acceptSocket1->write( testMessage, testMessageLen );
    };

#endif

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( SslSocketTest );

}

CORALCPPUNITTEST_MAIN( SslSocketTest )
