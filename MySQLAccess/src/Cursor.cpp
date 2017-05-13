// $Id: Cursor.cpp,v 1.4 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include "Cursor.h"
#include "Statement.h"

coral::MySQLAccess::Cursor::Cursor( Statement* statement, const coral::AttributeList& rowBuffer ) :
  m_statement( statement ),
  m_rowBuffer( rowBuffer )
{
}


coral::MySQLAccess::Cursor::~Cursor()
{
  this->close();
}


bool
coral::MySQLAccess::Cursor::next()
{
  if ( m_statement == 0 ) return false;
  bool result = m_statement->fetchNext();
  if ( ! result ) {
    this->close();
  }
  return result;
}


const coral::AttributeList& coral::MySQLAccess::Cursor::currentRow() const
{
  return m_rowBuffer;
}


void
coral::MySQLAccess::Cursor::close()
{
  if ( m_statement != 0 )
  {
    delete m_statement;
    m_statement = 0;
  }
}
