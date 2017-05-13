#ifndef CORALSTUBS_SIMPLEBYTEBUFFERITERATOR_H
#define CORALSTUBS_SIMPLEBYTEBUFFERITERATOR_H

//CoralServer includes
#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/IByteBufferIterator.h"

//C++ includes
#include <list>

namespace coral
{

  namespace CoralStubs {

    /** @class SimpleByteBufferIterator
     *   *
     *  @author Alexander Kalkhof
     *  @date   2009-04-28
     */

    class SimpleByteBufferIterator : public IByteBufferIterator
    {

    public:

      SimpleByteBufferIterator( const ByteBuffer& );

      virtual ~SimpleByteBufferIterator();

      bool next();

      bool isLast() const;

      const ByteBuffer& currentBuffer() const;

    private:

      const ByteBuffer& m_buffer;

      bool m_active;

      bool m_last;

    };

  }

}

#endif
