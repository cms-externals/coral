#include <iostream>
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"

#include "Cursor.h"
#include "DomainProperties.h"
#include "SessionProperties.h"
#include "SQLiteStatement.h"

coral::SQLiteAccess::Cursor::Cursor( SQLiteStatement* statement,
                                     coral::AttributeList& rowBuffer )
  : m_statement( statement )
  , m_rowBuffer( rowBuffer )
{
}


coral::SQLiteAccess::Cursor::~Cursor()
{
  this->close();
}


bool
coral::SQLiteAccess::Cursor::next()
{
  //if(!m_statement->sessionProperties()->session())
  //  throw coral::SessionException( "The cursor is no longer valid",
  //                                 "SQLiteStatement::next",
  //                                 m_statement->sessionProperties()->domainProperties().service()->name());
  if ( m_statement == 0 ) return false;
  bool result = m_statement->fetchNext();
  if ( ! result )
  {
    this->close();
    return false;
  }
  else
  {
    if( !m_statement->defineOutput(m_rowBuffer) ) {
      throw coral::QueryException("sqlite", "Could not define output","ICursor::currentRow" );
    }
  }
  return result;
}


const coral::AttributeList&
coral::SQLiteAccess::Cursor::currentRow() const
{
  //if(!m_statement->sessionProperties()->session())
  //  throw coral::SessionException( "The cursor is no longer valid",
  //                                 "SQLiteStatement::currentRow",
  //                                 m_statement->sessionProperties()->domainProperties().service()->name());
  return m_rowBuffer;
}


void
coral::SQLiteAccess::Cursor::close()
{
  if ( m_statement != 0 )
  {
    delete m_statement;
    m_statement = 0;
  }
}
