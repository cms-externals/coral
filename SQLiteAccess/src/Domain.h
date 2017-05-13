#ifndef SQLITEACCESS_DOMAIN_H
#define SQLITEACCESS_DOMAIN_H

#include "RelationalAccess/IRelationalDomain.h"

#include "CoralKernel/Service.h"

namespace coral
{
  namespace SQLiteAccess
  {
    class DomainProperties;
    class TypeConverter;
    /**
       @class Domain Domain.h

       Implementation of the IRelationalDomain interface for the SQLiteAccess module
       @author Zhen Xie
    */

    class Domain : public coral::Service, virtual public coral::IRelationalDomain
    {
    public:
      /// Standard Constructor
      Domain( const std::string& componentName );

      /// Standard Destructor
      virtual ~Domain();

      /// Returns the name of the RDBMS technology
      std::string flavorName() const;

      /// Returns the name of the software implementation library
      std::string implementationName() const;

      /// Returns the version of the software implementation library
      std::string implementationVersion() const;

      /// Creates a new session object. The user acquires ownership of this object.
      coral::IConnection* newConnection( const std::string& uriString ) const;

      /**
       * Decodes the user connection string into the real connection string that should be passed to the Connection class
       * and the schema name that should be passed to the Session class
       */
      std::pair<std::string, std::string > decodeUserConnectionString( const std::string& userConnectionString ) const;

      /**
       * Returns true if credentials have to be provided to start a session on the specified connection
       */
      bool isAuthenticationRequired( const std::string& connectionString ) const ;
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
