#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/CALPacketHeader.h"
#include "../../src/TcpSocket.h"

namespace coral
{
  typedef std::unique_ptr<ByteBuffer> ByteBufferPtr;

  namespace CoralSockets
  {

    // port used for testing
    int testPort = 50007;

#if 0
    std::unique_ptr<RingBufferPipes> pipes;
#endif

    // ---------------------------------------------------
    // helpers

    void createSockets( ISocketPtr &src, ISocketPtr &dst )
    {
      TcpSocketPtr listenSocket = TcpSocketPtr( (TcpSocket*)0);
#if 0
      pipes = std::unique_ptr<RingBufferPipes>( new RingBufferPipes( 10000, "test pipes") );
      src=pipes->getSrc();
      dst=pipes->getDst();
#else
      while ( listenSocket.get() == 0 )
      {
        try
        {
          listenSocket = TcpSocket::listenOn( "localhost", testPort );
        }
        catch (GenericSocketException &e) {
          std::string expMsg="Error binding socket";
          if (expMsg == std::string( e.what(), expMsg.size() ) ) {
            std::cout << "Caught exeption: '" << e.what()
                      << "' sleeping."<< std::endl;
            if (testPort > 50100)
              throw;
            testPort++;
            continue;
          }
          throw;
        }
      }
      src = TcpSocket::connectTo( "localhost", testPort );
      dst = listenSocket->accept();
#endif
    }

    ByteBufferPtr createBuffer( int num ) {
      ByteBufferPtr buf( new ByteBuffer( 3*sizeof( int ) ) );
      *((int*)buf->data()) = num;
      buf->setUsedSize( 3*sizeof( int ) );
      return buf;
    }

    int getBufferNum( ByteBufferPtr buf ) {
      return *(int*)buf->data();
    }

    int getBufferNum( const ByteBuffer& buf ) {
      return *(int*)buf.data();
    }

    ByteBufferPtr string2ByteBuffer( std::string str ) {
      ByteBufferPtr result( new ByteBuffer( str.length() ) );
      const char * cstr=str.c_str();

      for (unsigned int i=0; i<str.length(); i++ )
        *(result->data()+i) = (unsigned char)*(cstr+i);

      result->setUsedSize( str.length() );
      return result;
    }

    std::string byteBuffer2String( const ByteBuffer& buf ) {
      return std::string( (char*)buf.data(), buf.usedSize() );
    }

#define  checkByteBuffer( mesg, buf1, buf2 )                            \
    do {                                                                \
      CPPUNIT_ASSERT_EQUAL_MESSAGE( mesg " size",                       \
                                    buf1.usedSize(), buf2.usedSize() ); \
                                                                        \
      for (unsigned int i = 0; i< buf1.usedSize(); i++) {               \
        if ( *(buf1.data()+i) != *(buf2.data()+i) )                     \
          CPPUNIT_FAIL(mesg " content not equal");                      \
      };                                                                \
    } while (0)

  }
}
