#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include <iostream>
#include "Cursor.h"
#include "OracleStatement.h"

coral::OracleAccess::Cursor::Cursor( std::auto_ptr<OracleStatement> statement,
                                     const coral::AttributeList& rowBuffer )
  : m_statement( statement )
  , m_rowBuffer( rowBuffer )
{
  //std::cout << "Create Cursor " << this << std::endl; // debug bug #73334
}


coral::OracleAccess::Cursor::~Cursor()
{
  //std::cout << "Delete Cursor " << this << std::endl; // debug bug #73334
  this->close();
}


bool
coral::OracleAccess::Cursor::next()
{
  //std::cout << "Cursor " << this << ": next" << std::endl; // debug bug #73334
  if ( !m_statement.get() ) return false;
  //std::cout << "Cursor::next: fetchNext" << std::endl; // debug bug #73334
  bool result = m_statement->fetchNext();
  //std::cout << "Cursor::next: fetchNext OK" << std::endl; // debug bug #73334
  if ( ! result )
  {
    this->close();
  }
  return result;
}


const coral::AttributeList&
coral::OracleAccess::Cursor::currentRow() const
{
  return m_rowBuffer;
}


void
coral::OracleAccess::Cursor::close()
{
  m_statement.reset(); // Delete here explicitly like previous ptr (bug #90898)
}
