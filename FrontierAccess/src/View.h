#ifndef FRONTIER_ACCESS_VIEW_H
#define FRONTIER_ACCESS_VIEW_H

#include "RelationalAccess/IView.h"

namespace coral
{

  class ITableDescription;
  class TableDescription;

  namespace FrontierAccess
  {
    class ISessionProperties;

    /**
     * Class View
     *
     * Implementation of the IView interface for the FrontierAccess package
     */

    class View : virtual public coral::IView
    {
    public:
      /// Constructor
      View( const ISessionProperties& sessionProperties, const std::string& viewName );

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
      const ISessionProperties& m_sessionProperties;

      /// The definition string
      std::string m_definitionString;

      /// The transient description
      coral::TableDescription*  m_description;

    };

  }

}

#endif
