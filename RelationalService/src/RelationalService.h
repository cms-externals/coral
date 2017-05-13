#ifndef RELATIONALSERVICE_H
#define RELATIONALSERVICE_H

#include "RelationalAccess/IRelationalService.h"

#include "CoralKernel/Service.h"

#include "CoralBase/boost_thread_headers.h"

#include <map>
#include <set>
#include <vector>

namespace coral
{

  namespace RelationalService
  {

    /**
     * @class RelationalService RelationalService.h
     *
     * A default component implemenentation of the IRelationalService interface
     */
    class RelationalService : public coral::Service, virtual public coral::IRelationalService
    {
    public:
      /// Standard Constructor
      RelationalService( const std::string& componentName );

      /// Standard Destructor
      virtual ~RelationalService();

      /**
       * Lists the currently available technologies
       */
      std::vector< std::string > availableTechnologies() const;

      /**
       * Lists the available implementations for a given technology
       */
      std::vector< std::string > availableImplementationsForTechnology( const std::string& technologyName ) const;

      /**
       * Forces a particular implementation to be used when picking up a technology domain.
       * Returns true in case of success, false if the specified technology or domain are not valid.
       *
       * @param technology The name of the RDBMS technology
       * @param implementation The name of the implementation of the corresponding plugin. If not specified the native implementation is selected
       */
      void setDefaultImplementationForDomain( const std::string& technology, std::string implementation = "" );

      /**
       * Returns a reference to the underlying IRelationalDomain object given the name of
       * the RDBMS technology.
       * If the corresponding plugin module cannot be found, a NonExistingDomainException is thrown.
       */
      coral::IRelationalDomain& domain( const std::string& flavourName );

      /**
       * Returns a reference to the underlying IRelationalDomain object given the full connection
       * string specifying the working schema in a database.
       * If the corresponding plugin module cannot be found, a NonExistingDomainException is thrown.
       */
      coral::IRelationalDomain& domainForConnection( const std::string& connectionString );

      /// Friend declaration for modifying its properties
      friend class CallbackForRDBMSImplementations;

    private:
      /// Retrieves the plugins of the supported technologies
      void retrievePlugins();

      /// Sets the default implementations
      void setDefaultImplementations();

      /// Prefix for the plugin name
      const std::string m_pluginPrefix;

      /// The supported technologies/implementations
      std::map< std::string, std::set< std::string > > m_technologiesAndImplementations;

      /// Default implementations
      std::map< std::string, std::string > m_defaultImplementations;

      /// The property vector
      std::vector< std::string > m_properties;

      /// The mutex lock for the domain handles.
      boost::mutex m_mutexForDomains;
    };

  }

}

#endif
