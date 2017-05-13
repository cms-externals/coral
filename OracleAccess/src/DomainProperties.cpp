#include "DomainProperties.h"

coral::OracleAccess::DomainProperties::DomainProperties( coral::Service* service ) :
  m_service( service ),
  m_tableSpaceForTables(""),
  m_tableSpaceForIndices(""),
  m_tableSpaceForLobs(""),
  m_lobChunkSize(0)
{
}


coral::OracleAccess::DomainProperties::~DomainProperties()
{
}


void
coral::OracleAccess::DomainProperties::setTableSpaceForTables( const std::string& tableSpace )
{
  m_tableSpaceForTables = tableSpace;
}


void
coral::OracleAccess::DomainProperties::setTableSpaceForIndices( const std::string& tableSpace )
{
  m_tableSpaceForIndices = tableSpace;
}


void
coral::OracleAccess::DomainProperties::setTableSpaceForLobs( const std::string& tableSpace )
{
  m_tableSpaceForLobs = tableSpace;
}


void
coral::OracleAccess::DomainProperties::setLobChunkSize( int lobChunkSize )
{
  m_lobChunkSize = lobChunkSize;
}


coral::Service*
coral::OracleAccess::DomainProperties::service() const
{
  return m_service;
}


std::string
coral::OracleAccess::DomainProperties::tableSpaceForTables() const
{
  return m_tableSpaceForTables;
}


std::string
coral::OracleAccess::DomainProperties::tableSpaceForIndices() const
{
  return m_tableSpaceForIndices;
}


std::string
coral::OracleAccess::DomainProperties::tableSpaceForLobs() const
{
  return m_tableSpaceForIndices;
}


int
coral::OracleAccess::DomainProperties::lobChunkSize() const
{
  return m_lobChunkSize;
}
