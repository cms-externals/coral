#include <iostream>
#include "ColumnProxy.h"
#include "IConstraintRegistry.h"
//#include <iostream>
coral::SQLiteAccess::ColumnProxy::ColumnProxy( const coral::IColumn& columnReference,
                                               const coral::SQLiteAccess::IConstraintRegistry& constraintRegistry ) :
  m_columnReference( columnReference ),
  m_constraintRegistry( constraintRegistry )
{

}

coral::SQLiteAccess::ColumnProxy::~ColumnProxy()
{

}


std::string
coral::SQLiteAccess::ColumnProxy::name() const
{

  return m_columnReference.name();
}


std::string
coral::SQLiteAccess::ColumnProxy::type() const
{

  return m_columnReference.type();
}


int
coral::SQLiteAccess::ColumnProxy::indexInTable() const
{

  return m_columnReference.indexInTable();
}


bool
coral::SQLiteAccess::ColumnProxy::isNotNull() const
{

  return m_columnReference.isNotNull();
}


bool
coral::SQLiteAccess::ColumnProxy::isUnique() const
{

  m_constraintRegistry.refreshConstraints();
  return m_columnReference.isUnique();
}


long
coral::SQLiteAccess::ColumnProxy::size() const
{

  return m_columnReference.size();
}


bool
coral::SQLiteAccess::ColumnProxy::isSizeFixed() const
{

  return m_columnReference.isSizeFixed();
}


std::string
coral::SQLiteAccess::ColumnProxy::tableSpaceName() const
{

  return m_columnReference.tableSpaceName();
}
