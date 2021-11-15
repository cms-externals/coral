// -*- C++ -*-
// $Id: Domain.h,v 1.4 2011/03/22 10:36:50 avalassi Exp $
#ifndef FRONTIER_ACCESS_FRONTIERDOMAIN_H
#define FRONTIER_ACCESS_FRONTIERDOMAIN_H

#include "RelationalAccess/IRelationalDomain.h"

#include "CoralKernel/Service.h"

namespace coral
{

  class IConnection;
  class IWebCacheControl;
  class IConnectionService;

  namespace FrontierAccess
  {
    class DomainProperties;

    /**
       @class Domain Domain.h

       Implementation of the IRelationalDomain interface for the FrontierAccess module
    */

    class Domain : public coral::Service, public coral::IRelationalDomain
    {
    public:
      /// Standard Constructor
      Domain( const std::string& componentName );

      /// Standard Destructor
      virtual ~Domain();

      /// Returns the name of the RDBMS technology
      virtual std::string flavorName() const;

      /// Returns the name of the software implementation library
      virtual std::string implementationName() const;

      /// Returns the version of the software implementation library
      virtual std::string implementationVersion() const;

      /// Creates a new connection object
      virtual coral::IConnection* newConnection( const std::string& uriString ) const;

      /**
       * Decodes the user connection string into the real connection string
       * that should be passed to the Connection class and the schema name
       * that should be passed to the Session class
       */
      virtual std::pair<std::string, std::string >
      decodeUserConnectionString( const std::string& userConnectionString ) const;

      /**
       * Returns true if credentials have to be provided to start a session on the specified connection
       */
      virtual bool isAuthenticationRequired( const std::string& /*connectionString*/ ) const
      {
        return false;
      }

      /// Sets the table space for the tables
      void setTableSpaceForTables( const std::string& tableSpace );

      /// Sets the table space for the indices
      void setTableSpaceForIndices( const std::string& tableSpace );

      /// Sets the table space for the lobs
      void setTableSpaceForLobs( const std::string& tableSpace );

      /// Sets the lob chunk size
      void setLobChunkSize( int lobChunkSize );

    private:
      void setProperties();

    private:
      /// The flavor name
      std::string m_flavorName;

      /// The name of the software implementation library
      std::string m_implementationName;

      /// The version of the software implementation library
      std::string m_implementationVersion;

      /// The domain properties
      DomainProperties* m_properties;

    };

  }

}

#endif
