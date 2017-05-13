#include "ColumnProxy.h"
#include "IConstraintRegistry.h"

coral::OracleAccess::ColumnProxy::ColumnProxy( const coral::IColumn& columnReference,
                                               const coral::OracleAccess::IConstraintRegistry& constraintRegistry ) :
  m_columnReference( columnReference ),
  m_constraintRegistry( constraintRegistry )
{
}

coral::OracleAccess::ColumnProxy::~ColumnProxy()
{
}


std::string
coral::OracleAccess::ColumnProxy::name() const
{
  return m_columnReference.name();
}


std::string
coral::OracleAccess::ColumnProxy::type() const
{
  return m_columnReference.type();
}


int
coral::OracleAccess::ColumnProxy::indexInTable() const
{
  return m_columnReference.indexInTable();
}


bool
coral::OracleAccess::ColumnProxy::isNotNull() const
{
  return m_columnReference.isNotNull();
}


bool
coral::OracleAccess::ColumnProxy::isUnique() const
{
  m_constraintRegistry.refreshConstraints();
  return m_columnReference.isUnique();
}


long
coral::OracleAccess::ColumnProxy::size() const
{
  return m_columnReference.size();
}


bool
coral::OracleAccess::ColumnProxy::isSizeFixed() const
{
  return m_columnReference.isSizeFixed();
}


std::string
coral::OracleAccess::ColumnProxy::tableSpaceName() const
{
  return m_columnReference.tableSpaceName();
}
