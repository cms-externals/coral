#ifndef CORALSTUBS_DUMMYBYTEBUFFERITERATOR_H
#define CORALSTUBS_DUMMYBYTEBUFFERITERATOR_H

//CoralServer includes
#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/IByteBufferIterator.h"

//C++ includes
#include <list>

namespace coral
{

  namespace CoralStubs {

    /** @class DummyByteBufferIterator
     *   *
     *  @author Alexander Kalkhof
     *  @date   2009-04-08
     */

    class DummyByteBufferIterator : public IByteBufferIterator
    {

    public:

      DummyByteBufferIterator();

      virtual ~DummyByteBufferIterator();

      bool next();

      bool isLast() const;

      const ByteBuffer& currentBuffer() const;

      void addBuffer(const ByteBuffer&);

    private:

      std::list< const ByteBuffer* > m_buffers;

      const ByteBuffer* m_currentbuffer;

    };

  }

}

#endif
