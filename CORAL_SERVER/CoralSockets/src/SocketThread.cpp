// $Id: SocketThread.cpp,v 1.39.2.4 2011/09/16 14:21:56 avalassi Exp $

// Include files
#include <sstream>
#include "CoralMonitor/StatsTypeTimer.h"
#include "CoralMonitor/StopTimer.h"
#include "CoralServerBase/IRequestHandler.h"
#include "CoralServerBase/CTLMagicWordException.h"
#include "CoralSockets/GenericSocketException.h"

// Local include files
#include "SocketThread.h"
#include "StatsTypeThreads.h"

// Logger
#define LOGGER_NAME "CoralSockets::SocketThread"
#include "logger.h"

// Namespace
using namespace coral::CoralSockets;

//-----------------------------------------------------------------------------

static coral::StatsTypeTimer myStatsTypeTimerReply( "CSocket_ReplyTime",
                                                    "reply" );

//-----------------------------------------------------------------------------

/// class for handling requests
class SocketThread::HandlerThread : public Runnable
{

public:

  virtual ~HandlerThread() {}

  HandlerThread( SocketThread* sThread, const std::string& desc )
    : m_sThread( sThread ), m_desc( desc ) {}

  // return a description of the thread
  const std::string desc() const
  {
    std::stringstream msg;
    msg << "HandlerThread @" << this
        << " (for SocketThread @" << m_sThread << ")";
    return msg.str();
  }

  /// main loop of the handler
  void operator()();

  ///
  void endThread(){}

private:

  /// the "owner" of this handler thread
  SocketThread* m_sThread;

  /// description of the connection, for error messages
  const std::string m_desc;

};

//-----------------------------------------------------------------------------

const std::string SocketThread::desc() const
{
  std::stringstream msg;
  msg << "SocketThread @" << this;
  return msg.str();
}

//-----------------------------------------------------------------------------

void SocketThread::HandlerThread::operator()()
{
  INFO( "( " << m_desc << " ) Enter handler thread " << this );
  coral::IRequestHandler *handler( &m_sThread->getRequestHandler() );
  while ( m_sThread->isActive() )
  {
    try
    {
      PacketPtr packet1( m_sThread->getNewRequest() );
      if ( packet1.get() == 0 )
      {
        // no new packet -> the thread should end
        continue;
      }
      const int requestID = packet1->getHeader().requestID();
      const int clientID = packet1->getHeader().clientID();
      const ByteBuffer& request = packet1->getPayload();
      DEBUG( "( " << m_desc << " ) thread " << this
             << " handling Request #" << requestID
             << " clientID " << clientID );
      StopTimer replyTimer;
      replyTimer.start();
      std::unique_ptr<IByteBufferIterator>
        replyIt( handler->replyToRequest( request ) );
      uint32_t segmentID=0;
      while ( replyIt->next() )
      {
        DEBUG( "( " << m_desc << " ) sending Reply #" << requestID
               << " SegmentID " << segmentID << " from thread " << this );
        static int csdebug = -1; //AV
        if ( csdebug == -1 )
          csdebug = ( getenv ( "CORALSOCKETS_DEBUG" ) ? 1 : 0 );  //AV
        if ( csdebug > 0 )
          std::cout << "Sending reply to request #" << requestID
                    << " (segment #" << segmentID
                    << ") on socket thread " << m_sThread << std::endl;  //AV
        PacketSLAC packet( requestID,
                           clientID,
                           segmentID,
                           !replyIt->isLast() /* moreSegments */,
                           replyIt->currentBuffer() );
        m_sThread->send( packet );
        segmentID++;
      }
      DEBUG( "( " << m_desc << " ) Reply #" << requestID << " finished" );
      replyTimer.stop();
      myStatsTypeTimerReply.add( replyTimer.getUserTime(), replyTimer.getSystemTime(), replyTimer.getRealTime() );
    }
    catch ( std::exception& e )
    {
      ERROR( "( " << m_desc << " ) EXCEPTION CAUGHT: " << e.what() << " - CLOSE THE SOCKET" );
      m_sThread->closeSocket();
    }
    catch ( ... )
    {
      ERROR( "( " << m_desc << " ) UNKNOWN EXCEPTION CAUGHT - CLOSE THE SOCKET" );
      m_sThread->closeSocket();
    }
  }
  INFO( "( " << m_desc << " ) Exit handler thread " << this );
}

//-----------------------------------------------------------------------------

void SocketThread::send( const PacketSLAC& packet )
{
  boost::mutex::scoped_lock lock( m_mutexForSend );
  m_cSocket->sendPacket( packet );
}

//-----------------------------------------------------------------------------

PacketPtr SocketThread::getNewRequest()
{
#if 0
  boost::mutex::scoped_lock lock( m_mutexForRequests );
  while ( m_requests.empty() && m_isActive )
    m_condForRequests.wait( lock );
  if ( !m_isActive )
    return PacketPtr( (PacketSLAC*)0 );
  PacketPtr packet = m_requests.front();
  m_requests.pop();
  return packet;
#else
  boost::mutex::scoped_lock lock( m_mutexForRequests );
  try
  {
    if ( !m_isActive )
    {
      return PacketPtr( (PacketSLAC*) 0 );
    }
    PacketPtr packet( m_cSocket->receivePacket() );
    return packet;
  }
  catch( SocketClosedException& )
  {
    INFO( "The connection '" << m_cSocket->desc()
          <<"' has been closed. Stopping threads." );
  }
  catch( CTLMagicWordException& e )
  {
    ERROR( "Received packet with errors in the header '" << e.what() << "'" );
    // send error msg packet back
    PacketSLAC errPacket( CTLWrongMagicWord );
    send( errPacket );
  }
  catch( ErrorInHeaderException& e )
  {
    ERROR("Received packet with errors in the header '"<< e.what() << "'");
    // send error msg packet back
    if ( e.getErrorCode() == ErrorInHeaderException::ErrWrongVersion )
    {
      PacketSLAC errPacket( CTLWrongVersion );
      send( errPacket );
    }
    else if ( e.getErrorCode() == ErrorInHeaderException::ErrWrongChecksum )
    {
      PacketSLAC errPacket( CTLWrongChecksum );
      send( errPacket );
    }
    else
    {
      // send back a general checksum error
      PacketSLAC errPacket( CTLWrongChecksum );
      send( errPacket );
    }
  }
  m_isActive = false;
  return PacketPtr( (PacketSLAC*) 0 );
#endif
}

//-----------------------------------------------------------------------------

void SocketThread::operator()()
{
  INFO( "( " << m_cSocket->desc() <<" ) Enter recieve thread");
  getRequestHandler().setCertificateData( m_cSocket->getCertificateData() );
  for ( int i=0; i<m_nHThreads; i++ )
    m_thrManager.addThread( new HandlerThread( this, m_cSocket->desc() ) );
#if 0
  try
  {
    while ( m_isActive )
    {
      PacketPtr packet( m_cSocket->receivePacket() );
      if ( packet->getHeader().moreSegments() )
        throw GenericSocketException( "received multi-buffer request, but "
                                      "they are not yet supported!",
                                      "SocketThread::operator()()");
      if ( packet->getHeader().segmentNumber() != 0 )
        throw GenericSocketException( "received packet with segment number!=0",
                                      "SocketThread::operator()()");
      DEBUG( "( " << m_cSocket->desc() << " ) got packet request #"
             << packet->getHeader().requestID() );
      // queue packet for processing
      {
        boost::mutex::scoped_lock lock( m_mutexForRequests );
        m_requests.push( packet );
        m_condForRequests.notify_one( );
      }
    }
  }
  catch( SocketClosedException& e )
  {
    INFO("The connection '" << m_cSocket->desc()
         <<"' has been closed. Stopping threads." );
  }
  catch( CTLMagicWordException& e )
  {
    ERROR( "Received packet with errors in the header '" << e.what() << "'" );
    // send error msg packet back
    PacketSLAC errPacket( CTLWrongMagicWord );
    send( errPacket );
  }
  catch( ErrorInHeaderException& e )
  {
    ERROR("Received packet with errors in the header '"<< e.what() << "'");
    // send error msg packet back
    if ( e.getErrorCode() == ErrorInHeaderException::ErrWrongVersion )
    {
      PacketSLAC errPacket( CTLWrongVersion );
      send( errPacket );
    }
    else if ( e.getErrorCode() == ErrorInHeaderException::ErrWrongChecksum )
    {
      PacketSLAC errPacket( CTLWrongChecksum );
      send( errPacket );
    }
    else
    {
      // send back a general checksum error
      PacketSLAC errPacket( CTLWrongChecksum );
      send( errPacket );
    }
  }
  catch( std::exception& e )
  {
    ERROR("( " << m_cSocket->desc() << " ) EXCEPTION CAUGHT: " << e.what() );
  }
  catch( ... )
  {
    ERROR( "( " << m_cSocket->desc() << " ) UNKNOWN EXCEPTION CAUGHT" );
  }
  DEBUG( "( " << m_cSocket->desc() << " ) Waiting for the threads to end" );
  m_isActive = false;
  m_condForRequests.notify_all();
#endif
  m_thrManager.joinAll();
  DEBUG( "( " << m_cSocket->desc() << " ) Close the client socket");
  m_cSocket->close();
  INFO( "( " << m_cSocket->desc() << " ) Exit receive thread." );
  //printTimers();
}

//-----------------------------------------------------------------------------

const std::vector<coral::IThreadMonitor::ThreadRecord>
SocketThread::listThreads() const
{
  return m_thrManager.listThreads();
}

//-----------------------------------------------------------------------------
