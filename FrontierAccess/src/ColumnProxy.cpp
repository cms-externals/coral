#include "ColumnProxy.h"
#include "IConstraintRegistry.h"

coral::FrontierAccess::ColumnProxy::ColumnProxy( const coral::IColumn& columnReference,
                                                 const coral::FrontierAccess::IConstraintRegistry& constraintRegistry ) :
  m_columnReference( columnReference ),
  m_constraintRegistry( constraintRegistry )
{
}

coral::FrontierAccess::ColumnProxy::~ColumnProxy()
{
}


std::string
coral::FrontierAccess::ColumnProxy::name() const
{
  return m_columnReference.name();
}


std::string
coral::FrontierAccess::ColumnProxy::type() const
{
  return m_columnReference.type();
}


int
coral::FrontierAccess::ColumnProxy::indexInTable() const
{
  return m_columnReference.indexInTable();
}


bool
coral::FrontierAccess::ColumnProxy::isNotNull() const
{
  return m_columnReference.isNotNull();
}


bool
coral::FrontierAccess::ColumnProxy::isUnique() const
{
  m_constraintRegistry.refreshConstraints();
  return m_columnReference.isUnique();
}


long
coral::FrontierAccess::ColumnProxy::size() const
{
  return m_columnReference.size();
}


bool
coral::FrontierAccess::ColumnProxy::isSizeFixed() const
{
  return m_columnReference.isSizeFixed();
}


std::string
coral::FrontierAccess::ColumnProxy::tableSpaceName() const
{
  return m_columnReference.tableSpaceName();
}
