// $Id: ConnectionProperties.h,v 1.3.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_CONNECTIONPROPERTIES_H
#define CORALACCESS_CONNECTIONPROPERTIES_H 1

// Include files
#include <string>
#include "CoralServerBase/ICoralFacade.h"

namespace coral
{

  namespace CoralAccess
  {

    class DomainProperties;

    /** @class ConnectionProperties
     *
     *  A class holding the parameters of a CORAL server connection.
     *  Based on the OracleAccess class with the same name.
     *
     *  @author Andrea Valassi
     *  @date   2008-12-18
     */

    class ConnectionProperties
    {

    public:

      /// Constructor.
      ConnectionProperties( const DomainProperties& domainProperties,
                            const std::string& coralServerUrl );

      /// Destructor.
      virtual ~ConnectionProperties();

      /// Sets the facade for this connection.
      void setFacade( coral::ICoralFacade* facade );

      /// Returns the domain properties.
      const DomainProperties& domainProperties() const;

      /// Returns the coral server (or coral server proxy) connection string.
      std::string coralServerUrl() const;

      /// Is there a valid facade for this connection?
      bool isConnected() const;

      /// Returns the facade for this connection.
      const coral::ICoralFacade& facade() const;

    private:

      /// The domain properties.
      const DomainProperties& m_domainProperties;

      /// The CORAL server (or CORAL server proxy) connection string.
      std::string m_coralServerUrl;

      /// The facade for this connection (owned by the parent Connection).
      coral::ICoralFacade* m_facade;

    };

  }

}
#endif // CORALACCESS_CONNECTIONPROPERTIES_H
