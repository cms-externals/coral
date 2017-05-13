#ifndef SQLITEACCESS_VIEW_H
#define SQLITEACCESS_VIEW_H

#include "RelationalAccess/IView.h"
#include <boost/shared_ptr.hpp>

namespace coral {

  class ITableDescription;
  class TableDescription;

  namespace SQLiteAccess {

    class SessionProperties;
    class PrivilegeManager;

    /**
     * Class View
     *
     * Implementation of the IView interface for the SQLiteAccess package
     */

    class View : virtual public coral::IView
    {
    public:
      /// Constructor
      View( boost::shared_ptr<const SessionProperties> properties,
            const std::string& viewName );

      /// Destructor
      virtual ~View();

      /// Returns the corresponding table description
      const coral::ITableDescription& description() const;

      /**
       * Returns the name of the view
       */
      std::string name() const;

      /**
       * Returns the SQL string defining the view.
       * The SQL string is RDBMS-specific.
       */
      std::string definition() const;

      /**
       * Returns the number of columns in the view
       */
      int numberOfColumns() const;

      /**
       * Returns a reference to a column description object for the specified column index.
       * If the specified index is out of range, an InvalidColumnIndexException is thrown.
       */
      const coral::IColumn& column( int index ) const;

      /**
       * Returns a reference to the privilege manager of the view.
       */
      coral::ITablePrivilegeManager& privilegeManager();

    private:
      /// Reads the column description from the database
      void refreshDescription();

      /// Reads the view definition from the database
      void refreshDefinition();

    private:
      /// The session properties
      boost::shared_ptr<const SessionProperties> m_properties;

      /// The definition string
      std::string m_definitionString;

      /// The transient description
      coral::TableDescription*  m_description;

      /// The privilege manager
      PrivilegeManager*         m_privilegeManager;
    };

  }

}

#endif
