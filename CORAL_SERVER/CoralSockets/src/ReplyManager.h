// $Id: ReplyManager.h,v 1.9.2.5 2011/09/16 14:53:20 avalassi Exp $
#ifndef CORALSOCKETS_REPLYMANAGER_H
#define CORALSOCKETS_REPLYMANAGER_H 1

#include <vector>
#include <queue>

#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/IByteBufferIterator.h"
#include "CoralBase/boost_thread_headers.h"

#include "PacketSocket.h"
#include "SegmentationError.h"

namespace coral {

  class ReplyManagerTest; // forward declaration

  namespace CoralSockets {

    /** @struct replySlot
     *
     *  @author Martin Wache
     *  @date   2008-12-10
     *
     *  Holds information about the replies we are waiting for. The
     *  threads are put to sleep and waiting on a signal on cond.
     *
     *  to write to this struct, you have to own a lock on the mutex
     */

    class ReplySlot {
      friend class coral::ReplyManagerTest;

    public:
      ReplySlot()
        : m_requestID( 0 )
        , m_segmentNr( 0 )
        , m_used( false )
        , m_mutex()
        , m_cond()
        , m_replies()
        , m_status( OK )
      {};


      /// appends a byte buffer to the replies
      void appendReply( std::unique_ptr<ByteBuffer> reply, int segmentNr,
                        bool lastReply );

      /// returns next reply and removes it from the list
      /// this method sleeps if necesary
      boost::shared_ptr<ByteBuffer> nextReply( bool &lastReply );

      /// called by the iterator when it is deallocated
      /// if the slot has no pending replies it is freed,
      /// if not it will be freed as soon as the last reply arrieved.
      void releaseSlot();

      /// removes all remaining messages from the list
      /// and marks the slot as free
      void freeSlot();

      /// marks this request slot as used by requestID
      /// return false if this fails
      bool takeSlot( int requestID );

      /// marks this slot as connection closed
      /// and signals the thread
      void setConnectionClosed();

      inline int used()
      { return m_used; };

      inline int requestID()
      { return m_requestID; };

    private:
      // lockless private implementation of freeSlot()
      void _freeSlot();

      /// marks this slot as timed out
      /// expects the mutex to be locked!
      void setTimedOut();

      /// the request id the thread is waiting for
      int m_requestID;

      /// expected segment number of next packet
      int m_segmentNr;

      /// if the slot is currently used
      int m_used;

      /// mutex and conditional variable with which the thread is locked and
      /// sleeping
      boost::mutex m_mutex;
      boost::condition_variable m_cond;

      /// the replies
      std::queue< boost::shared_ptr< ByteBuffer > > m_replies;

      enum Status {
        OK,
        TIME_OUT,
        CLOSED,
        ALL_REPLIES,
        ITERATOR_CLOSED
      };

      /// true if all replies are in the replies queue
      Status m_status;

    protected:
      /// time after which a reply is timed out in seconds
      static unsigned int timeout;

    };


    /** @class SocketReplyIterator
     *
     *  @author Martin Wache
     *  @date   2009-01-27
     *
     */
    class SocketReplyIterator : public IByteBufferIterator
    {
    public:
      SocketReplyIterator( ReplySlot& slot );

      /// Destructor.
      virtual ~SocketReplyIterator();

      /// Get the next reply.
      virtual bool next();

      /// Get a reference to the current reply buffer.
      /// Throws an exception if next() was not called to get the first reply.
      /// Throws an exception if next() failed to get a new reply.
      virtual const ByteBuffer& currentBuffer() const;

      virtual bool isLast() const;

    protected:
      ReplySlot& m_slot;

      bool m_slotReleased;

      boost::shared_ptr<ByteBuffer> m_currentBuf;

      bool m_lastReply;
    };




    /** @class ReplyManager
     *
     *  @author Martin Wache
     *  @date   2008-12-10
     *
     *  A class for managing the sleeper slots.
     *  requestSlot( requestID ) reserves a replySlot struct on which the
     *  requestor can sleep
     *  replySlot( requestID ) finds the replySlot struct corresponding to
     *  the requestID
     */

    class ReplyManager {
    public:
      ReplyManager(int sleepersSize_=100) :
        m_requestOffset(0),
        m_answerOffset(0),
        m_sleepersSize( sleepersSize_)
      {
        m_sleepers=new ReplySlot[ m_sleepersSize ];
      };

      ~ReplyManager()
      {
        delete[] m_sleepers;
      };

      // protected by send mutex
      /// automaticaly takes the slot for requestID
      ReplySlot& requestSlot( int requestID );

      /// only executed by one thread
      ReplySlot& findSlot( int requestID );

      /// mark all slots as closed and wake up waiters
      void close();

    private:

      inline void nextSlot( int& slot ) {
        slot=(slot+1)%m_sleepersSize;
      };
      inline void prevSlot( int& slot ) {
        if (slot==0)
          slot=m_sleepersSize;
        slot=(slot-1);
      };

      int m_requestOffset;
      int m_answerOffset;
      int m_sleepersSize;
      ReplySlot* m_sleepers;
    };

    class AllSlotsTakenException : public Exception
    {

    public:

      /// Constructor
      AllSlotsTakenException( const std::string& methodName = "" )
        : Exception( "All slots in the replyManager are taken!",
                     methodName, "coral::CoralSockets" ) {}

      /// Destructor
      virtual ~AllSlotsTakenException() throw() {}

    };


  }
}

#endif // CORALSOCKETS_REPLYMANAGER_H
