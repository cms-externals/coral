// $Id: IByteBufferIterator.h,v 1.1.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_IBYTEBUFFERITERATOR_H
#define CORALSERVERBASE_IBYTEBUFFERITERATOR_H 1

// Include files
#include "CoralServerBase/ByteBuffer.h"

namespace coral
{

  /** @class IByteBufferIterator
   *
   *  Interface to an iterator over ByteBuffer's.
   *
   *  An instance of this interface (returned as a pointer) holds the
   *  replies returned by the IRequestHandler replyToRequest method.
   *
   *  Concrete classes implementing this interface are required to
   *  position the iterator before the first buffer in the loop:
   *  the next() method must be called to retrieve the first buffer.
   *
   *  @author Andrea Valassi and Martin Wache
   *  @date   2009-01-23
   */

  class IByteBufferIterator
  {

  public:

    /// Destructor.
    virtual ~IByteBufferIterator(){}

    /// Get the next buffer.
    virtual bool next() = 0;

    /// Is the current buffer the last one?
    /// Throws an exception if next() was never called (<first buffer).
    /// Throws an exception if next() returned false (>last buffer).
    virtual bool isLast() const = 0;

    /// Get a reference to the current buffer.
    /// Throws an exception if next() was never called (<first buffer).
    /// Throws an exception if next() returned false (>last buffer).
    virtual const ByteBuffer& currentBuffer() const = 0;

  };

}
#endif // CORALSERVERBASE_IBYTEBUFFERITERATOR_H
