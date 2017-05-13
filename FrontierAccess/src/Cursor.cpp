#include "Cursor.h"
#include "Statement.h"

coral::FrontierAccess::Cursor::Cursor( Statement* statement,
                                       const coral::AttributeList& rowBuffer ) :
  m_statement( statement ),
  m_rowBuffer( rowBuffer )
{
}


coral::FrontierAccess::Cursor::~Cursor()
{
  this->close();
}


bool
coral::FrontierAccess::Cursor::next()
{
  if ( m_statement == 0 ) return false;
  bool result = m_statement->fetchNext();
  if ( ! result ) {
    this->close();
  }
  return result;
}


const coral::AttributeList&
coral::FrontierAccess::Cursor::currentRow() const
{
  return m_rowBuffer;
}


void
coral::FrontierAccess::Cursor::close()
{
  if ( m_statement != 0 ) {
    delete m_statement;
    m_statement = 0;
  }
}
