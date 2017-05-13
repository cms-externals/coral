#ifndef MYSQLACCESS_VIEW_FACTORY_H
#define MYSQLACCESS_VIEW_FACTORY_H 1

#ifdef WIN32
#pragma warning ( disable : 4250 ) // dominance in multiple inheritance
#endif

#include "RelationalAccess/IViewFactory.h"
#include "QueryDefinition.h"

namespace coral {

  namespace MySQLAccess {

    /**
     * Class ViewFactory
     *
     * Implementation of the IViewFactory interface for the MySQLAccess module
     */

    class ViewFactory : public QueryDefinition, virtual public coral::IViewFactory
    {
    public:
      /// Constructor
      explicit ViewFactory( const coral::MySQLAccess::ISessionProperties& properties );

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
