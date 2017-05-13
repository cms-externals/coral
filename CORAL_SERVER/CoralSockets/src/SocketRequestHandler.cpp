// $Id: SocketRequestHandler.cpp,v 1.13.2.4 2011/09/16 14:00:29 avalassi Exp $

// Include files
#include <iostream>

// Local include files
#include "PacketSocket.h"
#include "ReplyManager.h"
#include "SocketRequestHandler.h"
#include "ThreadManager.h"

// Logger
#define LOGGER_NAME "CoralSockets::SocketRequestHandler"
#include "logger.h"

//-----------------------------------------------------------------------------

namespace coral
{
  namespace CoralSockets
  {
    /// a class which does the receiving of packets in an own thread
    class SocketRequestHandler::receiveThread : public Runnable
    {
    public:

      receiveThread( const PacketSocketPtr& socket,
                     ReplyManager& rplMngr )
        : m_Socket( socket )
        , m_rplMngr( rplMngr )
        , m_socketOpen( true )
      {
        DEBUG("socketRequestHandler::receiveThread constructor");
      }

      virtual ~receiveThread()
      {
        DEBUG("socketRequestHandler::receiveThread destructor");
      }

      /// the main loop of the thread
      void operator()();

      /// tell the recieve thread that the socket is going to be closed
      void endThread()
      {
        m_socketOpen = false;
      }

      /// thread description
      const std::string desc() const
      {
        return std::string("Receiver Thread ")+m_Socket->desc();
      }

    private:

      /// the socket which is used to receive data
      const PacketSocketPtr& m_Socket;

      /// storage container for the waiting requests
      ReplyManager& m_rplMngr;

      /// true while the socket is open
      /// if false, the recieve thread will end
      bool m_socketOpen;
    };

  }
}

//-----------------------------------------------------------------------------

// Namespace
using namespace coral::CoralSockets;

//-----------------------------------------------------------------------------

// the main loop of the receiver thread
void SocketRequestHandler::receiveThread::operator()()
{
  INFO( "SocketRequestHandler::receiveThread main loop started" );

  while ( m_socketOpen )
  {
    try
    {
      PacketPtr packet( m_Socket->receivePacket() );

      const int requestID = packet->getHeader().requestID();
      DEBUG("Got reply to request #" << requestID );
      static int csdebug = -1; //AV
      if ( csdebug == -1 )
        csdebug = ( getenv ( "CORALSOCKETS_DEBUG" ) ? 1 : 0 );  //AV
      if ( csdebug > 0 )
        std::cout << "Got reply to request #" << requestID << std::endl;  //AV

      // find sleeper slot
      ReplySlot &slot = m_rplMngr.findSlot( requestID );

      // put reply and wake up the sleeper!
      slot.appendReply( packet->getPayloadPointer(),
                        packet->getHeader().segmentNumber(),
                        !packet->getHeader().moreSegments() /* isLast */ );

    }
    catch( SocketClosedException& )
    {
      INFO("The socket has been closed. Stopping threads.");
      m_socketOpen = false;
      break;
    }
    catch( std::exception& e )
    {
      ERROR("ERROR! Standard C++ exception: '" << e.what() << "'");
      break;
    }
    catch( ... )
    {
      ERROR("ERROR! Unknown exception caught");
      break;
    }
  }

  // notify waiting threads that the socket has been closed
  m_rplMngr.close();

  INFO("SocketRequestHandler::receiveThread main loop ended");
}

//-----------------------------------------------------------------------------

void
SocketRequestHandler::setCertificateData( const coral::ICertificateData* /*cert*/ )
{
  // DUMMY ???
}

//-----------------------------------------------------------------------------

coral::IByteBufferIteratorPtr
SocketRequestHandler::replyToRequest( const coral::ByteBuffer& requestBuffer )
{
  ReplySlot& slot = send( requestBuffer );
  return IByteBufferIteratorPtr( new SocketReplyIterator( slot ) );
}

//-----------------------------------------------------------------------------

SocketRequestHandler::SocketRequestHandler( PacketSocketPtr socket )
  : IRequestHandler()
  , m_Socket( socket )
  , m_requestID( 0 )
  , m_rplMngr( new ReplyManager() )
  , m_receiveThread( 0 )
  , m_thrManager( new ThreadManager() )
{
  INFO("SocketRequestHandler constructor connection:"<< m_Socket->desc());

  m_receiveThread =  new receiveThread( m_Socket, *m_rplMngr );
  // thrManager take the owner ship of m_receiveThread
  m_thrManager->addThread( m_receiveThread );
}

//-----------------------------------------------------------------------------

SocketRequestHandler::~SocketRequestHandler()
{
  INFO("Destructor SocketRequestHandler started");
  m_receiveThread->endThread();
  m_Socket->close();
  // wait until the receive thread has terminated
  m_thrManager->joinAll();
  DEBUG("Destructor SocketRequestHandler finished");

}

//-----------------------------------------------------------------------------

ReplySlot& SocketRequestHandler::send( const coral::ByteBuffer& requestMsg )
{
 retry:
  try
  {
    boost::mutex::scoped_lock lock( m_mutexForSend );
    m_requestID++;
    ReplySlot& slot=m_rplMngr->requestSlot( m_requestID );
    PacketSLAC packet( m_requestID,
                       0 /*segment number*/,
                       0 /*clientID*/,
                       false /* more segments*/,
                       requestMsg );
    DEBUG("sending request #"<< m_requestID);
    m_Socket->sendPacket( packet );
    return slot;
  }
  catch ( AllSlotsTakenException& )
  {
    ERROR("(non fatal) all slots for requests are taken. Sleeping.");
    sleep( 20 );
    goto retry;
  }
  catch( std::exception& e )
  {
    ERROR("ERROR! Standard C++ exception: '" << e.what() << "'");
    throw;
  }
  catch( ... )
  {
    ERROR("ERROR! Unknown exception caught");
    throw;
  }
}

//-----------------------------------------------------------------------------
