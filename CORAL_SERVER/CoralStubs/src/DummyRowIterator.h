#ifndef CORAL_CORALSTUBS_ROWITERATORALL_H
#define CORAL_CORALSTUBS_ROWITERATORALL_H

// Include files
#include "CoralBase/AttributeList.h"

#include "CoralServerBase/IRowIterator.h"
#include "CoralServerBase/IRequestHandler.h"


namespace coral {

  namespace CoralStubs {

    class DummyRowIterator : public IRowIterator {
    public:

      DummyRowIterator();

      virtual ~DummyRowIterator();

      bool next();

      const AttributeList& currentRow() const;

      AttributeList& addRow();

    private:

      std::vector<AttributeList*> m_data;

      size_t m_pos;

    };


  }

}

#endif
