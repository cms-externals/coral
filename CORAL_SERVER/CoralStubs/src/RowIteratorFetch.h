#ifndef CORAL_CORALSTUBS_ROWITERATORFETCH_H
#define CORAL_CORALSTUBS_ROWITERATORFETCH_H

//Coral includes
#include "CoralBase/AttributeList.h"

//CoralServer includes
#include "CoralServerBase/IRequestHandler.h"
#include "CoralServerBase/IRowIterator.h"
#include "CoralServerBase/ICoralFacade.h"

//CoralServerStubs includes
#include "CppTypes.h"

#include <list>

namespace coral {

  namespace CoralStubs {

    /** @class RowIteratorFetch
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

    class RowIteratorFetch : public IRowIterator {
    public:
      //pass by a predefined rowbuffer to replace the internal one
      RowIteratorFetch(IRequestHandler& requestHandler, uint32_t cursorID, size_t maxrows, AttributeList* rowBuffer);

      virtual ~RowIteratorFetch();

      bool next();

      const AttributeList& currentRow() const;

      size_t getNumberOfRequests() { return m_requestno; };

    private:

      void fillBuffer();
      //reference to an IRequestHandler instance
      IRequestHandler& m_requestHandler;

      uint32_t m_cursor;

      size_t m_maxrows;

      AttributeList* m_obuffer;

      AttributeList* m_iterbuffer;

      bool m_waslast;

      std::list< AttributeList* > m_buffers;

      size_t m_requestno;

    };

  }
}

#endif
