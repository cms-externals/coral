#include "SimpleByteBufferIterator.h"

#include "Exceptions.h"

namespace coral { namespace CoralStubs {

  SimpleByteBufferIterator::SimpleByteBufferIterator( const ByteBuffer& buffer )
    : m_buffer( buffer )
    , m_active( true )
    , m_last( false )
  {
  }

  SimpleByteBufferIterator::~SimpleByteBufferIterator()
  {
  }

  bool
  SimpleByteBufferIterator::next()
  {
    if(m_active)
    {
      m_active = false;
      return true;
    }
    m_last = true;
    return false;
  }

  bool
  SimpleByteBufferIterator::isLast() const
  {
    return !m_active;
  }

  const ByteBuffer&
  SimpleByteBufferIterator::currentBuffer() const
  {
    if(m_last)
      throw StubsException("SimpleByteBufferIterator already called");
    return m_buffer;
  }

} }
