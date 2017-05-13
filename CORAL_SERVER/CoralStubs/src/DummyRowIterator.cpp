#include "DummyRowIterator.h"

#include "Exceptions.h"

namespace coral { namespace CoralStubs {

  DummyRowIterator::DummyRowIterator()
    : m_pos( 0 )
  {
  }

  DummyRowIterator::~DummyRowIterator()
  {
  }

  bool
  DummyRowIterator::next()
  {
    if(m_pos < m_data.size()) {
      m_pos++;
      return true;
    }
    return false;
  }

  const AttributeList&
  DummyRowIterator::currentRow() const
  {
    if((m_pos == 0))
    {
      throw ReplyIteratorException("Wrong access to RowIterator, call next() first","RowIteratorAll::currentRow");
    }
    return *(m_data[m_pos - 1]);
  }

  AttributeList&
  DummyRowIterator::addRow()
  {
    AttributeList* attrp = new AttributeList;
    m_data.push_back(attrp);
    return *attrp;
  }

} }
