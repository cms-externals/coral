// $Id: RowVectorIterator.h,v 1.2 2009/02/05 11:34:23 avalassi Exp $
#ifndef CORALSERVER_ROWVECTORITERATOR_H
#define CORALSERVER_ROWVECTORITERATOR_H 1

// Include files
#include <vector>
#include "CoralServerBase/IRowIterator.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class RowVectorIterator
     *
     *  Iterator over a vector of rows represented as AttributeList's.
     *
     *  @author Andrea Valassi
     *  @date   2009-01-26
     */

    class RowVectorIterator : virtual public IRowIterator
    {

    public:

      /// Constructor from a row buffer pointer and a vector of rows.
      /// The new iterator is positioned before the start of the loop.
      RowVectorIterator( AttributeList* pBuffer,
                         const std::vector<AttributeList> rows );

      /// Destructor.
      virtual ~RowVectorIterator();

      /// Get the next row.
      bool next();

      /// Is the current row the last one?
      /// Throws an exception if next() was never called (<first row).
      /// Throws an exception if next() failed to get a new reply (>last row).
      bool isLast() const;

      /// Get a reference to the current row.
      /// Throws an exception if next() was never called (<first row).
      /// Throws an exception if next() failed to get a new reply (>last row).
      const AttributeList& currentRow() const;

    private:

      /// Standard constructor is private.
      RowVectorIterator();

      /// Copy constructor is private.
      RowVectorIterator( const RowVectorIterator& rhs );

      /// Assignment operator is private.
      RowVectorIterator& operator=( const RowVectorIterator& rhs );

    private:

      /// The next call count.
      unsigned int m_next;

      /// Was the buffer pointer in the constructor not null?
      bool m_hasBuffer;

      /// The current row pointer (NOT owned by this instance).
      /// If the buffer pointer in the constructor was not null,
      /// the rows in the vector are copied into the pointed buffer.
      /// If the buffer pointer in the constructor was null,
      /// the pointer is updated to point to each row in the vector.
      AttributeList* m_pBuffer;

      /// The row vector.
      const std::vector<AttributeList> m_rows;

    };
  }
}
#endif // CORALSERVERBASE_ROWVECTORITERATOR_H
