#ifndef CORALSOCKETS_ICERTIFICATEDATA_H
#define CORALSOCKETS_ICERTIFICATEDATA_H

#include "RelationalAccess/../src/ICertificateData.h"

#include <string>

namespace coral {

  namespace CoralSockets {

    /**
     * Class CertificateData
     *
     * Class to hold the informations from the certificate of the user.
     * It is assumed that the informations have already been verified.
     *
     */

    class CertificateData : public coral::ICertificateData
    {
    public:
      CertificateData();

      virtual ~CertificateData();

      /**
       * Returns the distinguished name of the certificate
       */
      virtual const std::string& distinguishedName( ) const;

      /**
       * Returns the full qualified attribute names of the certificate
       */
      virtual const std::vector< std::string >& FQANs( ) const;

      void setDistinguishedName( const std::string& dn );

      void addFQAN( const std::string& fqan );

    protected:
      std::string m_distinguishedName;

      std::vector< std::string > m_FQANs;
    };
  }

}

#endif
