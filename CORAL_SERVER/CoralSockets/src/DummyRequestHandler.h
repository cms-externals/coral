// $Id: DummyRequestHandler.h,v 1.4.2.2 2011/09/16 16:06:14 avalassi Exp $
#ifndef CORALSOCKETS_DUMMYREQUESTHANDLER_H
#define CORALSOCKETS_DUMMYREQUESTHANDLER_H 1

// Include files
#include <boost/shared_ptr.hpp>
#include <queue>
#include "CoralServerBase/IRequestHandler.h"

namespace coral
{
  namespace CoralSockets
  {

    /** @class SimpleReplyIterator
     *
     *  @author Martin Wache
     *  @date   2009-02-03
     */

    class SimpleReplyIterator : virtual public IByteBufferIterator
    {

    public:

      /// Destructor.
      virtual ~SimpleReplyIterator() {}

      /// Constructor from two arguments: a buffer (with the reply already
      /// retrieved) and a buffer pool (that must be used to release it).
      /// The new iterator is positioned before the start of the loop.
      SimpleReplyIterator( ByteBuffer& buffer, 
                           bool isLast=true );
      SimpleReplyIterator( boost::shared_ptr<ByteBuffer> buffer, 
                           bool isLast=true );

      /// adds buffers to the iterator. If isLast should be set to true
      /// for the last buffer added.
      void addBuffer( ByteBuffer& buffer, 
                      bool isLast);
      void addBuffer( boost::shared_ptr<ByteBuffer> buffer, 
                      bool isLast);

      /// Get the next reply.
      bool next();

      /// Is the current row the last one?
      /// Throws an exception if next() was never called (<first reply).
      /// Throws an exception if next() failed to get a new reply (>last reply).
      bool isLast() const;

      /// Get a reference to the current reply buffer.
      /// Throws an exception if next() was never called (<first reply).
      /// Throws an exception if next() failed to get a new reply (>last reply).
      const ByteBuffer& currentBuffer() const;

    private:
      /// copy constructor is private
      SimpleReplyIterator( const SimpleReplyIterator& rhs );

      /// Standard constructor is private.
      SimpleReplyIterator();

      /// Assignment operator is private.
      SimpleReplyIterator& operator=( const SimpleReplyIterator& rhs );

    private:

      /// The buffer with the current reply.
      boost::shared_ptr<ByteBuffer> m_currentBuffer;

      /// A vector with the replys
      std::queue< boost::shared_ptr<ByteBuffer> > m_buffers;

      /// If the last reply has already been inserted into the vector
      bool m_gotLastBuffer;
    };


    /** @class DummyRequestHandler
     *
     *  @author Andrea Valassi
     *  @date   2007-12-23
     */

    class DummyRequestHandler : virtual public IRequestHandler
    {

    public:

      // Standard constructor.
      DummyRequestHandler();

      // Destructor.
      virtual ~DummyRequestHandler();

      /// Set the certificate data for this request handler.
      /// The caller has to ensure that the ICertificateData pointer
      /// remains valid over the lifetime of the IRequestHandler.
      void setCertificateData( const coral::ICertificateData* cert );

      // Handle a request message and return an iterator over reply messages.
      // This method cannot be const because we need to lock mutexes in it!
      // The iterator is positioned before the first reply in the loop:
      // the next() method must be called to retrieve the first reply.
      // It is the caller's responsibility to make sure that the reference to
      // the reply buffer pool is still valid while the iterator is being used.
      //
      // the dummy request handler will return the content of the request buffer
      // with "Thank you for your request '" prepended.
      // If the request starts with "copy X", where X is a number it will
      // return X buffers containing "Thank you for your request X'" prepended
      // to the request, where X is the number of the buffer starting with 0
      IByteBufferIteratorPtr replyToRequest( const ByteBuffer& requestBuffer );

    };

  }
}
#endif // CORALSERVERBASE_DUMMYREQUESTHANDLER_H
