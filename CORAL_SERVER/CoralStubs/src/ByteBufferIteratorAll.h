#ifndef CORAL_CORALSTUBS_BYTEBUFFERITERATORALL_H
#define CORAL_CORALSTUBS_BYTEBUFFERITERATORALL_H

//CoralServer includes
#include "CoralServerBase/CALOpcode.h"
#include "CoralServerBase/IByteBufferIterator.h"
#include "CoralServerBase/IRowIterator.h"

//Coral includes
#include "CoralBase/AttributeList.h"

//CoralServerStubs includes
#include "SegmentWriterIterator.h"

namespace coral {

  namespace CoralStubs {

    class ByteBufferIteratorAll : public IByteBufferIterator {
    public:

      ByteBufferIteratorAll(IRowIterator*, CALOpcode opcode, bool cacheable, bool proxy, bool isempty, AttributeList*);

      ~ByteBufferIteratorAll();

      bool next();

      bool isLast() const;

      const ByteBuffer& currentBuffer() const;

    private:

      void fillBuffer();

      SegmentWriterIterator m_swi;

      IRowIterator* m_rowi;

      AttributeList* m_rowBuffer;

      bool m_isempty;

      void* m_structure;

      bool m_islast;

      bool m_lastbuffer;

    };

  }

}

#endif
