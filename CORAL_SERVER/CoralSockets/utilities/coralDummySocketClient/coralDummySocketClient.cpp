// $Id: coralDummySocketClient.cpp,v 1.1.2.6 2011/09/16 16:45:08 avalassi Exp $

// Include files
#include <iostream>
#include <sstream>
#include "CoralServerBase/IRequestHandler.h"
#include "CoralMonitor/StopTimer.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralSockets/GenericSocketException.h"
#include "CoralSockets/SocketClient.h"

// Local include files
#include "../../src/DummyRequestHandler.h"

// Message output
#define LOG std::cout

//-----------------------------------------------------------------------------

namespace coral
{
  namespace CoralSockets
  {

    //-------------------------------------------------------------------------

    typedef std::unique_ptr<ByteBuffer> ByteBufferPtr;

    //-------------------------------------------------------------------------

    bool compareResult( IByteBufferIteratorPtr it1,
                        IByteBufferIteratorPtr it2 )
    {
      while ( it1->next() ) {
        if ( !it2->next() )
          return false;

        if ( it1->currentBuffer().usedSize() != it2->currentBuffer().usedSize() )
          return false;

        for (unsigned int i=0; i<it1->currentBuffer().usedSize(); i++ )
          if ( *(it1->currentBuffer().data()+i) != *(it2->currentBuffer().data()+i) )
            return false;
      };
      if (it2->next() )
        return false;

      return true;
    }

    //-------------------------------------------------------------------------

    ByteBufferPtr string2ByteBuffer( std::string str )
    {
      ByteBufferPtr result( new ByteBuffer( str.length() ) );
      const char * cstr=str.c_str();
      for (unsigned int i=0; i<str.length(); i++ )
        *(result->data()+i) = *(cstr+i);
      result->setUsedSize( str.length() );
      return result;
    }

    //-------------------------------------------------------------------------

    class TestClientThreadMT
    {
    public:

      virtual ~TestClientThreadMT(){}

      TestClientThreadMT( coral::IRequestHandler* handler,
                          const int iThr,
                          const int bufferSize=10000 )
        : m_handler( handler ), m_iThr( iThr ), m_bufferSize( bufferSize ){}

      void operator()()
      {
        DummyRequestHandler dummyHandler;
        try {
          // Test #1 - make sure the server can handle long strings
          {
            ByteBuffer request( int(m_bufferSize*1.5) );
            for ( unsigned int i= 0; i< request.maxSize(); i++ )
              *(request.data()+i)=(char) (i&0xff);
            request.setUsedSize( request.maxSize() );

            //std::cout << "__TestClientThreadMT Send request (payload size=" << request.size() << ")" << std::endl;
            IByteBufferIteratorPtr reply( m_handler->replyToRequest( request ) );
            //std::cout << "__TestClientThreadMT Received reply (payload size=" << reply.size() << ")" << std::endl;
            IByteBufferIteratorPtr expReply( dummyHandler.replyToRequest( request ) );

            if ( !compareResult( reply, expReply ) )
              throw GenericSocketException
                ( "Wrong reply to request with long string" );
          }
          std::cout << "Thread #" << m_iThr << " test 1 OK" << std::endl;
          // Test #2 - make sure the server replies to the right request
          for ( unsigned int iReq=1; iReq<=5000; iReq++ )
          {
            std::stringstream msg;
            msg << "Request #" << iReq << " from thread #" << m_iThr
                << " of client " << m_handler;
            ByteBufferPtr request = string2ByteBuffer( msg.str() );

            IByteBufferIteratorPtr reply( m_handler->replyToRequest( *request ) );
            IByteBufferIteratorPtr expReply( dummyHandler.replyToRequest( *request ) );

            if ( !compareResult( reply, expReply ) )
              throw GenericSocketException
                ( "Wrong reply to request "+msg.str() );
          }
          std::cout << "Thread #" << m_iThr << " test 2 OK" << std::endl;
          std::cerr << "Thread #" << m_iThr << " test 2 OK" << std::endl;
          // Test #3 - make sure the server can handle all values of char
          {
            std::string requestStr = "";
            for ( int i = -128; i<=127; i++ ) requestStr += char(i);
            for ( int i = -128; i<=127; i++ ) requestStr += char(i);
            if ( requestStr.size() != 512 )
            {
              std::stringstream msg;
              msg << "Wrong input size " << requestStr.size() << " (expected 512)";
              throw GenericSocketException( msg.str() );
            }
            ByteBufferPtr request = string2ByteBuffer( requestStr );

            IByteBufferIteratorPtr reply( m_handler->replyToRequest( *request ) );
            IByteBufferIteratorPtr expReply( dummyHandler.replyToRequest( *request ) );

            if ( !compareResult( reply, expReply ) )
              throw GenericSocketException
                ( "Wrong reply to request '"+requestStr+"'" );
          }
          std::cout << "Thread #" << m_iThr << " test 3 OK" << std::endl;
          // Test #4 - make sure the server can handle multy byte buffer replies
          {
            std::stringstream msg;
            msg << "copy 7 Multi reply request from thread #" << m_iThr
                << " of client " << m_handler;
            ByteBufferPtr request = string2ByteBuffer( msg.str() );

            IByteBufferIteratorPtr reply( m_handler->replyToRequest( *request ) );
            IByteBufferIteratorPtr expReply( dummyHandler.replyToRequest( *request ) );

            if ( !compareResult( reply, expReply ) )
              throw GenericSocketException
                ( "Wrong reply to request '"+msg.str()+"'" );
          }
          std::cout << "Thread #" << m_iThr << " test 4 OK" << std::endl;
#if 0
          // Test #5 - simulate long reply times from the backend database
          {
            std::stringstream msg;
            msg << "sleep 21 sleep reply request from thread #" << m_iThr
                << " of client " << m_handler;
            ByteBufferPtr request = string2ByteBuffer( msg.str() );

            IByteBufferIteratorPtr reply( m_handler->replyToRequest( *request ) );
            // the dummyHandler would sleep also, but I don't want to sleep
            // so we hardcode the result

            std::string thanks = "Thank you for your request '";
            ByteBuffer expReply( thanks.size() + request->usedSize() );
            ::memcpy( expReply.data(), thanks.data(), thanks.size() );
            ::memcpy( expReply.data() + thanks.size(), request->data(),
                      request->usedSize() );
            expReply.setUsedSize( thanks.size() + request->usedSize() );

            if ( !reply->next() ) throw GenericSocketException("no reply to sleep request?");

            if ( reply->currentBuffer().usedSize() != expReply.usedSize() )
              throw GenericSocketException("wrong reply size on sleep request!");


            for (unsigned int i=0; i<expReply.usedSize(); i++ )
              if ( *(reply->currentBuffer().data()+i) != *(expReply.data()+i) )
                throw GenericSocketException("wrong reply on sleep request");

          }
          std::cout << "Thread #" << m_iThr << " test 5 OK" << std::endl;
          // All tests completed
#endif
        }
        catch( std::exception& e ) {
          std::cerr << "ERROR! Standard C++ exception: '" << e.what() << "'" << std::endl;
        }

        catch( ... ) {
          std::cerr << "ERROR! Unknown exception caught" << std::endl;
        }

      }

    private:
      coral::IRequestHandler* m_handler;
      const int m_iThr;
      const int m_bufferSize;

    };

  }
}

//-----------------------------------------------------------------------------

// Namespace
using namespace coral::CoralSockets;

/** @file dummySocketClient.cpp
 *
 *  @author Andrea Valassi and Martin Wache
 *  @date   2007-12-26
 */

int main( int argc, char** argv )
{

  try
  {

    // Get the server parameters
    std::string host;
    int port;
    //bool debug = false;
    if ( argc == 1 )
    {
      host = "localhost";
      port = 50017;
      //debug = true;
    }
    /*
    else if ( argc == 5 || argc == 6 )
    {
      host = argv[1];
      port = atoi( argv[2] );
      bufferSize = atoi( argv[3] );
      if ( atoi( argv[4] ) == 1 ) debug = true;
      if ( argc == 6 && std::string( argv[5] ) == std::string( "SLAC" ) ) SLAC = true;
    }
    else
    {
      LOG << "Usage:   " << argv[0] << " [host port bufferSize debug [SLAC]]" << std::endl;
      LOG << "Example: " << argv[0] << " localhost 50017 256 1" << std::endl;
      LOG << "Example: " << argv[0] << " localhost 60017 256 1 SLAC" << std::endl;
      return 1;
    }
    */
    else
    {
      LOG << "Usage:   " << argv[0] << std::endl;
      return 1;
    }
    // Create a client
    LOG << "Entering main" << std::endl;
    LOG << "Create a client connected to the server on "
        << host << ":" << port << std::endl;
    SocketClient client( host, port );
    coral::IRequestHandler* clientHandler=client.getRequestHandler();

    boost::thread_group threads;
    for ( unsigned int iThr=1; iThr<=20; iThr++ )
    {
      LOG << "Create thread#" << iThr << std::endl;
      TestClientThreadMT thread( clientHandler, iThr );
      threads.create_thread( thread );
    }
    threads.join_all();
    LOG << "Exiting main" << std::endl;
  }

  catch( std::exception& e )
  {
    LOG << "ERROR! Standard C++ exception: '" << e.what() << "'" << std::endl;
    return 1;
  }

  catch( ... )
  {
    LOG << "ERROR! Unknown exception caught" << std::endl;
    return 1;
  }

  //printTimers();
  // Successful program termination
  return 0;

}

//-----------------------------------------------------------------------------
