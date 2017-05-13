#ifndef RELATIONALACCESS_ICERTIFICATEDATA_H
#define RELATIONALACCESS_ICERTIFICATEDATA_H

#include <string>
#include <vector>

namespace coral
{

  /**
   * Class ICertificateData
   *
   * Class to hold the informations from the certificate of the user.
   * It is assumed that the informations have already been verified.
   *
   */

  class ICertificateData
  {

  public:

    /**
     * Returns the distinguished name of the certificate
     */
    virtual const std::string& distinguishedName( ) const = 0;

    /**
     * Returns the full qualified attribute names of the certificate
     */
    virtual const std::vector< std::string >& FQANs( ) const = 0;

  protected:

    /// Protected empty destructor
    virtual ~ICertificateData() {}

  };

}
#endif
