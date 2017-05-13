// $Id: DomainProperties.cpp,v 1.5 2010/03/19 23:35:03 avalassi Exp $
#include "MySQL_headers.h"

#include "Domain.h"
#include "DomainProperties.h"
#include "TypeConverter.h"

coral::MySQLAccess::DomainProperties::DomainProperties( coral::MySQLAccess::Domain* service )
  : m_service( service )
  , m_typeConverter( 0 )
{
  m_typeConverter = new coral::MySQLAccess::TypeConverter();
}

coral::MySQLAccess::DomainProperties::~DomainProperties()
{
  if( m_typeConverter != 0 )
  {
    delete m_typeConverter;
    m_typeConverter = 0;
  }
}

coral::Service* coral::MySQLAccess::DomainProperties::service() const
{
  return dynamic_cast<coral::Service*>(m_service);
}

coral::ITypeConverter& coral::MySQLAccess::DomainProperties::typeConverter() const
{
  return *m_typeConverter;
}
