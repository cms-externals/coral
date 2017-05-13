#ifndef CORAL_CORALSTUBS_ROWITERATORALL_H
#define CORAL_CORALSTUBS_ROWITERATORALL_H

//CoralServer includes
#include "CoralServerBase/CALOpcode.h"
#include "CoralServerBase/IRowIterator.h"
#include "CoralServerBase/IRequestHandler.h"

//Coral includes
#include "CoralBase/AttributeList.h"

//CoralServerStubs includes
#include "SegmentReaderIterator.h"

namespace coral {

  namespace CoralStubs {

    /** @class RowIteratorAll
     *
     *  An implementation of the RowIterator
     *
     *  This class transforms from IByteBufferIterator
     *  to a rowIterator. It uses the SegmentReaderIterator
     *  for handling of segments and decoding/unmarshalling
     *
     *  @author Alexander Kalkhof
     *  @date   2009-04-15
     */

    class RowIteratorAll : public IRowIterator {
    public:

      RowIteratorAll(IByteBufferIterator* reply, AttributeList* rowBuffer, CALOpcode opcode);

      virtual ~RowIteratorAll();

      bool next();

      const AttributeList& currentRow() const;

    private:

      AttributeList* m_obuffer;

      bool m_cbuffer;

      bool m_ibuffer;

      bool m_fbuffer;
      //reference to an IRequestHandler instance
      IByteBufferIterator* m_reply;

      SegmentReaderIterator* m_sri;

      void* m_structure;

    };

  }

}

#endif
