#include "CertificateData.h"

using namespace coral;
using namespace coral::CoralSockets;

//-----------------------------------------------------------------------------

CertificateData::CertificateData()
  : m_distinguishedName()
  , m_FQANs()
{
}

//-----------------------------------------------------------------------------

CertificateData::~CertificateData()
{
}

//-----------------------------------------------------------------------------

const std::string& CertificateData::distinguishedName() const
{
  return m_distinguishedName;
}

//-----------------------------------------------------------------------------

const std::vector< std::string >& CertificateData::FQANs() const
{
  return m_FQANs;
}

//-----------------------------------------------------------------------------

void CertificateData::addFQAN( const std::string& fqan)
{
  m_FQANs.insert( m_FQANs.begin(), fqan );
}

//-----------------------------------------------------------------------------

void CertificateData::setDistinguishedName( const std::string& dn)
{
  m_distinguishedName = dn;
}
