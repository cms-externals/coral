#ifndef ORACLEACCESS_VIEWFACTORY_H
#define ORACLEACCESS_VIEWFACTORY_H 1

#ifdef WIN32
#pragma warning ( disable : 4250 )
#endif

#include <boost/shared_ptr.hpp>
#include "RelationalAccess/IViewFactory.h"
#include "QueryDefinition.h"

namespace coral
{

  namespace OracleAccess
  {

    /**
     * Class ViewFactory
     *
     * Implementation of the IViewFactory interface for the OracleAccess module
     */

    class ViewFactory : public QueryDefinition,
                        virtual public coral::IViewFactory
    {
    public:
      /// Constructor
      explicit ViewFactory( boost::shared_ptr<const SessionProperties> properties,
                            const std::string& schemaName );

      /// Destructor
      virtual ~ViewFactory();

      /**
       * Creates a new view with the specified
       * name and the current query definition.
       * In case the view already exists a ViewAlreadyExistingException is thrown.
       */
      coral::IView& create( const std::string& viewName );

      /**
       * Creates or replaces in case it exists a view with the specified
       * name and the current query definition.
       */
      coral::IView& createOrReplace( const std::string& viewName );
    };
  }
}

#endif
