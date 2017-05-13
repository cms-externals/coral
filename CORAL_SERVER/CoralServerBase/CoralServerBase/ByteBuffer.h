// $Id: ByteBuffer.h,v 1.7.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_BYTEBUFFER_H
#define CORALSERVERBASE_BYTEBUFFER_H 1

// Include files
#include <iostream>

/** @class ByteBuffer ByteBuffer.h
 *
 *  Simple implementation of a byte buffer.
 *  Looseely inspired by the java.nio.Buffer and coral::Blob classes.
 *
 *  A byte buffer is a finite sequence of bytes.
 *  It can be used to write data into it and read data from it.
 *  Apart from its contents, the properties of a ByteBuffer are
 *  its maximum size (capacity) and its used size (append position).
 *
 *  A ByteBuffer's maximum size is the number of allocated bytes.
 *  All of these bytes are safe to be read from or written into
 *  but not all of these bytes are used: bytes that have been
 *  allocated, but not written to, have an undefined content.
 *  The maximum size is never negative, but can be dynamically changed.
 *  Shrinking a buffer will remove trailing bytes after the new maximum size:
 *  this will generally not result in data copies in memory.
 *  Expanding a buffer will append trailing bytes after the old maximum size:
 *  this may result in expensive data copies in memory.
 *
 *  A ByteBuffer's used size is the number of used bytes.
 *  The used size is never negative or greater than its maximum size.
 *  Users who wish to append data 'at the end' of the ByteBuffer
 *  should write at the position of the used size and then increment it.
 *
 *  Writing or reading data from memory beyond the maximum size of the
 *  ByteBuffer may lead to undefined results or access violations.
 *  Writing or reading data from memory beyond the used size of the
 *  ByteBuffer (but within its maximum size) may lead to undefined results.
 *
 *  @author Andrea Valassi, Alexander Kalkhof and Martin Wache
 *  @date   2009-01-22
 */

namespace coral
{

  //--------------------------------------------------------------------------

  class ByteBuffer
  {

  public:

    /// Standard constructor.
    /// Create a ByteBuffer with maxSize=0 and usedSize=0.
    ByteBuffer();

    /// Create a ByteBuffer with maxSize=newMaxSize and usedSize=0.
    /// The content of the new bytes allocated is undefined.
    ByteBuffer( size_t newMaxSize );

    /// Destructor.
    virtual ~ByteBuffer();

    /// Get the current maximum size.
    size_t maxSize() const
    {
      return m_maxSize;
    }

    /// Get the current used size.
    size_t usedSize() const
    {
      return m_usedSize;
    }

    /// Get the current number of unused bytes.
    size_t freeSize() const
    {
      return maxSize() - usedSize();
    }

    /// Get the address of the buffer data (const - read only).
    const unsigned char* data() const
    {
      return m_data;
    }

    /// Get the address of the buffer data (non const - read or write).
    /// WARNING: the caller is responsible to avoid access violations.
    unsigned char* data()
    {
      return m_data;
    }

    /// Set the new used size.
    /// Throws an exception if newUsedSize>maxSize().
    /// Setting newUsedSize<usedSize() is allowed.
    void setUsedSize( size_t newUsedSize );

    /// Resize the buffer (change the new maximum size).
    /// Expanding the buffer may involve expensive memory copies;
    /// the content of the new bytes allocated is undefined.
    /// Shrinking the buffer erases all bytes in [newMaxSize,maxSize)
    /// and does not generally involve memory copies; it also sets
    /// usedSize=newMaxSize if currently usedSize>newMaxSize.
    void resize( size_t newMaxSize );

    /// Print the ByteBuffer to an output stream.
    std::ostream& print( std::ostream& out ) const;

  private:

    /// Copy constructor is private.
    ByteBuffer( const ByteBuffer& rhs );

    /// Assignment operator is private.
    ByteBuffer& operator=( const ByteBuffer& rhs );

  private:

    /// The current maximum size of the byte buffer.
    size_t m_maxSize;

    /// The current used size of the byte buffer.
    size_t m_usedSize;

    /// The data in the byte buffer.
    unsigned char* m_data;

  };

  //--------------------------------------------------------------------------

  inline std::ostream& operator<<( std::ostream& out, const ByteBuffer& sbuf )
  {
    return sbuf.print( out );
  }

  //--------------------------------------------------------------------------

}
#endif // CORALSERVERBASE_BYTEBUFFER_H
