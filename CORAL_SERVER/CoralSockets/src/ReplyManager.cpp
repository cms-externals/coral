// $Id: ReplyManager.cpp,v 1.12.2.2 2011/09/16 14:47:38 avalassi Exp $

// Include files
#include "CoralMonitor/StatsTypeTimer.h"
#include "CoralMonitor/StopTimer.h"
#include "CoralSockets/GenericSocketException.h"

// Local include files
#include "ReplyManager.h"

// Logger
#define LOGGER_NAME "CoralSockets::ReplyManager"
#include "logger.h"

// Namespace
using namespace coral;
using namespace coral::CoralSockets;

// Assign value of static variable
unsigned int ReplySlot::timeout=30*60; // half an hour

//-----------------------------------------------------------------------------

static coral::StatsTypeTimer myStatsTypeTimerRequestSlot("CSocket_RequestSlot", "request slot");
static coral::StatsTypeTimer myStatsTypeTimerFindSlot("CSocket_FindSlot", "find slot");

//-----------------------------------------------------------------------------

void ReplySlot::appendReply( std::auto_ptr<ByteBuffer> reply,
                             int segmentNr,
                             bool lastReply )
{
  boost::shared_ptr<ByteBuffer> buffer(reply.get());
  reply.release();
  boost::mutex::scoped_lock lock( m_mutex);
  if ( !m_used )
    throw GenericSocketException("Slot is not used!",
                                 "ReplySlot::appendReply");
  if ( m_status==ALL_REPLIES )
    throw GenericSocketException("Slot has already got all replies!",
                                 "ReplySlot::appendReply");
  if ( m_segmentNr != segmentNr )
    throw SegmentationErrorException("multi segment reply, but segment no is"
                                     " not continous!",
                                     "ReplySlot::appendReply");
  if ( m_status==ITERATOR_CLOSED )
  {
    // if the iterator is already closed we drop incoming replies after the
    // checks
    if ( lastReply )
      _freeSlot();
    return;
  }
  m_replies.push( buffer );
  m_segmentNr++;
  if ( lastReply ) m_status = ALL_REPLIES;
  m_cond.notify_all();
}

//-----------------------------------------------------------------------------

boost::shared_ptr<ByteBuffer> ReplySlot::nextReply( bool& lastReply )
{
  boost::mutex::scoped_lock lock( m_mutex );
  if ( m_status == ITERATOR_CLOSED )
    throw GenericSocketException("Panic! nextReply called after slot has"
                                 " been released!", "ReplySlot::nextReply");
  // return 0 if the last reply has already been returned
  if ( m_status == ALL_REPLIES && m_replies.empty() )
    return boost::shared_ptr<ByteBuffer>( (ByteBuffer*)0 );
  unsigned int wait_time=0;
  while ( m_replies.empty() && m_status == OK )
  {
    // it looks like on Mac OS signals can get lost?
    // so better use a time out..
    // todo: handle errors (connection lost etc...)
    boost::xtime xt;
    xtime_get(&xt, boost::TIME_UTC);
    xt.sec += 20;
    if ( !m_cond.timed_wait(lock, xt) )
    {
      DEBUG( "wait for reply timed out..."<< m_requestID << std::endl);
      wait_time+=20;
      if (!m_replies.empty() )
        ERROR("(non fatal) wait for reply timed out but reply is there.."
              " Signal lost? " << m_requestID);
    }
    if ( wait_time > timeout )
      setTimedOut();
  }
  if ( m_status == TIME_OUT )
    throw GenericSocketException("reply timed out","ReplySlot::nextReply");
  if ( m_status == CLOSED )
    throw GenericSocketException("connection closed", "ReplySlot::nextReply");
  if ( m_replies.empty() )
    throw GenericSocketException("Panic! replies empty before pop!",
                                 "ReplySlot::nextReply");
  boost::shared_ptr<ByteBuffer> buffer( m_replies.front() );
  m_replies.pop();
  lastReply = m_replies.empty() && (m_status==ALL_REPLIES);
  return buffer;
}

//-----------------------------------------------------------------------------

void ReplySlot::_freeSlot()
{
  DEBUG("freeSlot("<<m_requestID<<") called" << std::endl);
  while ( !m_replies.empty() )
    m_replies.pop();
  m_used=false;
  m_status=OK;
}

//-----------------------------------------------------------------------------

void ReplySlot::freeSlot()
{
  boost::mutex::scoped_lock lock( m_mutex );
  _freeSlot();
}

//-----------------------------------------------------------------------------

void ReplySlot::releaseSlot()
{
  boost::mutex::scoped_lock lock( m_mutex );
  DEBUG("releaseSlot("<<m_requestID<<") called" << std::endl);
  // the iterator has been closed, so we don't need the replies
  // any more...
  while ( !m_replies.empty() )
    m_replies.pop();
  if (m_status==ALL_REPLIES)
  {
    // we have already got all replies, so we can just free the slot
    _freeSlot();
    return;
  }
  // wait until all replies have arrived and then free the slot
  m_status=ITERATOR_CLOSED;
}

//-----------------------------------------------------------------------------

bool ReplySlot::takeSlot( int requestID )
{
  boost::mutex::scoped_lock lock( m_mutex );
  DEBUG("takeSlot( " << requestID << " ) called" << std::endl );
  if ( m_used ) return false;
  m_requestID=requestID;
  m_used=true;
  m_status=OK;
  m_segmentNr=0;
  return true;
}

//-----------------------------------------------------------------------------

void ReplySlot::setTimedOut()
{
  // private method, expects m_mutex to be locked!
  if (m_status != OK)
    throw GenericSocketException("Panic! m_status!=OK",
                                 "ReplySlot::setTimedOut");
  m_status=TIME_OUT;
}

//-----------------------------------------------------------------------------

void ReplySlot::setConnectionClosed()
{
  boost::mutex::scoped_lock lock( m_mutex );
  m_status=CLOSED;
  m_cond.notify_all();
}

//-----------------------------------------------------------------------------

SocketReplyIterator::SocketReplyIterator( ReplySlot& slot )
  : IByteBufferIterator()
  , m_slot(slot)
  , m_slotReleased(false)
  , m_currentBuf((ByteBuffer*) 0)
  , m_lastReply( false )
{
}

//-----------------------------------------------------------------------------

SocketReplyIterator::~SocketReplyIterator()
{
  if ( !m_slotReleased )
    m_slot.releaseSlot();
}

//-----------------------------------------------------------------------------

bool SocketReplyIterator::next()
{
  if (m_slotReleased)
  {
    // slot got already released, we are at the last buffer (or even beyond)
    // release previous buffer
    m_currentBuf = boost::shared_ptr<ByteBuffer>( (ByteBuffer*)0 );
    return false;
  }
  DEBUG( "SocketReplyIterator::next() request #"<<m_slot.requestID() );
  m_currentBuf = m_slot.nextReply( m_lastReply );
  if (m_lastReply)
  {
    m_slot.releaseSlot();
    m_slotReleased=true;
  }
  return m_currentBuf.get() != 0;
}

//-----------------------------------------------------------------------------

bool SocketReplyIterator::isLast() const
{
  if ( m_currentBuf.get() == 0)
    throw GenericSocketException("no current Buffer. Next has not"
                                 " been called?", "SocketReplyIterator::isLast()");
  return m_lastReply;
}

//-----------------------------------------------------------------------------

const ByteBuffer& SocketReplyIterator::currentBuffer() const
{
  if ( m_currentBuf.get() == 0)
    throw GenericSocketException("called currentReply() before the first "
                                 "next() or after the last reply", "SocketReplyIterator::currentReply");
  return *m_currentBuf;
}

//-----------------------------------------------------------------------------

ReplySlot& ReplyManager::requestSlot( int requestID )
{
  ScopedTimerStats timer(myStatsTypeTimerRequestSlot);
  //  ScopedTimer("CoralSockets::ReplyManger::requestSlot");
  int slot = (requestID + m_requestOffset) % m_sleepersSize;
  int tries=0;
  int maxTries=m_sleepersSize;
  while ( m_sleepers[ slot ].used() != 0 && tries< maxTries )
  {
    tries++;
    DEBUG( "skipping slot " << slot << " for id " << requestID);
    nextSlot( slot );
  }
  if ( tries>=maxTries )
    throw AllSlotsTakenException( "requestSlot" );
  if ( !m_sleepers[ slot ].takeSlot( requestID ) )
    throw GenericSocketException("PANIC! internal error could not take slot!",
                                 "ReplyManager::requestSlot");
  m_requestOffset=(m_requestOffset+tries)%m_sleepersSize;
  return m_sleepers[ slot ];
}

//-----------------------------------------------------------------------------

ReplySlot& ReplyManager::findSlot( int requestID )
{
  ScopedTimerStats timer(myStatsTypeTimerFindSlot);
  //  ScopedTimer("CoralSockets::ReplyManger::findSlot");
  int slot = (requestID + m_answerOffset ) % m_sleepersSize;
  int tries=0;
  int maxTries=m_sleepersSize;
  if ( m_sleepers[slot].requestID() == requestID )
    return m_sleepers[ slot ];
  if ( m_sleepers[slot].requestID() > requestID )
  {
    while ( m_sleepers[slot].requestID() != requestID && tries< maxTries )
    {
      tries++;
      DEBUG( "prev answer skipping slot " << slot << " id "
             << m_sleepers[slot].requestID() << " searching " << requestID );
      prevSlot(slot);
    }
  }
  else
  {
    while ( m_sleepers[slot].requestID() != requestID && tries< maxTries )
    {
      tries++;
      DEBUG( "answer skipping slot " << slot <<" id "
             << m_sleepers[slot].requestID() << " searching " << requestID );
      nextSlot(slot);
    }
    m_answerOffset=(m_answerOffset+tries)%m_sleepersSize;
  }
  if ( m_sleepers[ slot ].requestID() != requestID )
    throw GenericSocketException( " didn't find correct slot!",
                                  "ReplyManager::findSlot()");
  if ( tries>=maxTries )
    throw GenericSocketException( "replySlot max tries" );
  return m_sleepers[ slot ];
}

//-----------------------------------------------------------------------------

void ReplyManager::close()
{
  for ( int i=0; i < m_sleepersSize; i++)
    m_sleepers[ i ].setConnectionClosed();
}

//-----------------------------------------------------------------------------
