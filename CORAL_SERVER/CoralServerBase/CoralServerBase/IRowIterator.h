// $Id: IRowIterator.h,v 1.3.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_IROWITERATOR_H
#define CORALSERVERBASE_IROWITERATOR_H 1

// Include files
#include "CoralBase/AttributeList.h"

namespace coral
{

  /** @class IRowIterator
   *
   *  Interface to an iterator over rows represented as AttributeList's.
   *
   *  Concrete classes implementing this interface are required to
   *  position the iterator before the first row in the loop:
   *  the next() method must be called to retrieve the first row.
   *
   *  This is similar to the ICursor interface, with two differences:
   *  first, the destructor is public, because the CORAL facade returns
   *  auto-pointers to iterators rather than references to them;
   *  second, there is no close() method, because any server-side cursors
   *  associated to the iterator should be released in the desctructor.
   *
   *  This is also similar to the IByteBufferIterator interface
   *  (iterating over AttributeList's rather than over ByteBuffer's),
   *  with the important difference that there is no isLast() method
   *  because it is not needed and would lead to unnecessary row copies.
   *
   *  @author Andrea Valassi, Martin Wache and Alexander Kalkhof
   *  @date   2009-01-26
   */

  class IRowIterator
  {

  public:

    /// Destructor.
    virtual ~IRowIterator(){}

    /// Get the next row.
    virtual bool next() = 0;

    /// Get a reference to the current row.
    /// Throws an exception if next() was never called (<first row).
    /// Throws an exception if next() failed to get a new reply (>last row).
    virtual const AttributeList& currentRow() const = 0;

  };

}
#endif // CORALSERVERBASE_IROWITERATOR_H
