#include <cstring> // For memcpy
#include "DummyByteBufferIterator.h"

namespace coral { namespace CoralStubs {

  DummyByteBufferIterator::DummyByteBufferIterator()
    : m_currentbuffer( 0 )
  {

  }

  DummyByteBufferIterator::~DummyByteBufferIterator()
  {
    if(m_currentbuffer)
      delete m_currentbuffer;
  }

  bool
  DummyByteBufferIterator::next()
  {
    //  std::cout << "amount of buffers (dummy) " << m_buffers.size() << std::endl;

    if(m_buffers.size() > 0)
    {
      if(m_currentbuffer) delete m_currentbuffer;

      m_currentbuffer = m_buffers.front();

      m_buffers.pop_front();

      return true;
    }
    if(m_currentbuffer) delete m_currentbuffer;

    m_currentbuffer = 0;

    return false;
  }

  bool
  DummyByteBufferIterator::isLast() const
  {
    //this method is not used from all use cases
    return true;
  }

  const ByteBuffer&
  DummyByteBufferIterator::currentBuffer() const
  {
    if(!m_currentbuffer)
      throw;
    return *(m_currentbuffer);
  }

  void
  DummyByteBufferIterator::addBuffer(const ByteBuffer& bb)
  {
    //need to copy the byte buffer
    size_t size = bb.usedSize();

    ByteBuffer* nbb = new ByteBuffer(size);

    memcpy(nbb->data(), bb.data(), size);

    nbb->setUsedSize(size);

    m_buffers.push_back(nbb);
  }


} }
