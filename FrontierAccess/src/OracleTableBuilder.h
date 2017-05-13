#ifndef FRONTIERACCSS_FRONTIER_TABLE_BUILDER_H
#define FRONTIERACCSS_FRONTIER_TABLE_BUILDER_H

#include <string>

namespace coral {

  class ITableDescription;

  namespace FrontierAccess {

    class ISessionProperties;

    /**
     * Class OracleTableBuilder
     *
     * Utility class to construct the SQL statement for the creation of a table
     */

    class OracleTableBuilder
    {
    public:
      /// Constructor
      OracleTableBuilder( const coral::ITableDescription& description,
                          const ISessionProperties& properties );

      /// Destructor
      ~OracleTableBuilder();

      /// Returns the sql statement
      std::string statement() const;

      /// Returns the proper sql type
      static std::string sqlType( const coral::FrontierAccess::ISessionProperties& properties,
                                  const std::string& cppTypeName,
                                  int size,
                                  bool fixedSize );

    private:
      /// The resulting statement
      std::string m_statement;
    };

  }

}

#endif
