#ifndef SQLITEACCESS_VIEW_FACTORY_H
#define SQLITEACCESS_VIEW_FACTORY_H

#ifdef WIN32
//IViewFactory and QueryDefinition are inherits of IQueryDefinition
//QueryDefinition dominates the others but setting them to pure virtual
#pragma warning ( disable : 4250 )
#endif

#include "RelationalAccess/IViewFactory.h"
#include "QueryDefinition.h"
#include <boost/shared_ptr.hpp>

namespace coral {

  namespace SQLiteAccess {

    /**
     * Class ViewFactory
     *
     * Implementation of the IViewFactory interface for the SQLiteAccess module
     */

    class ViewFactory : public QueryDefinition,
                        virtual public coral::IViewFactory
    {
    public:
      /// Constructor
      explicit ViewFactory( boost::shared_ptr<const SessionProperties> properties );

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
