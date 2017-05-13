#ifndef SQLITEACCSS_SQLITETABLEBUILDER_H
#define SQLITEACCSS_SQLITETABLEBUILDER_H

#include <string>
#include <boost/shared_ptr.hpp>

namespace coral {

  class ITableDescription;

  namespace SQLiteAccess {

    class SessionProperties;

    /**
     * Class SQLiteTableBuilder
     *
     * Utility class to construct the SQL statement for the creation of a table
     */

    class SQLiteTableBuilder
    {
    public:
      /// Constructor
      SQLiteTableBuilder( const coral::ITableDescription& description,
                          boost::shared_ptr<const SessionProperties> properties, bool temporary=false);

      /// Destructor
      ~SQLiteTableBuilder();

      /// Returns the sql statement
      std::string statement() const;

      /// Returns the table name
      std::string tableName() const;
    private:
      /// The table description
      const coral::ITableDescription& m_description;
      /// The resulting statement
      std::string m_statement;
      /// If to build temporary table
      bool m_temporary;
    };
  }
}

#endif
