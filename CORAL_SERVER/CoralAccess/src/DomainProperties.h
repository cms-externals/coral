// $Id: DomainProperties.h,v 1.4.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_DOMAINPROPERTIES_H
#define CORALACCESS_DOMAINPROPERTIES_H 1

// Include files
#include "CoralKernel/Service.h"

namespace coral
{

  namespace CoralAccess
  {

    /** @class DomainProperties
     *
     *  Stripped-down version of the OracleAccess DomainProperties class.
     *
     *  @author Andrea Valassi
     *  @date   2008-12-17
     */

    class DomainProperties
    {

    public:

      /// Constructor
      explicit DomainProperties( coral::Service* service );

      /// Destructor
      virtual ~DomainProperties();

      /// Returns the pointer to the service
      coral::Service* service() const
      {
        return m_service;
      }

    private:

      /// The pointer to the service
      coral::Service* m_service;

    };

  }

}
#endif // CORALACCESS_DOMAINPROPERTIES_H
