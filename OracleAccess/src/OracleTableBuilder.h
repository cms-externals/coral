#ifndef ORACLEACCSS_ORACLETABLEBUILDER_H
#define ORACLEACCSS_ORACLETABLEBUILDER_H 1

#include <string>
#include <boost/shared_ptr.hpp>

namespace coral
{

  class ITableDescription;

  namespace OracleAccess
  {

    class SessionProperties;

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
                          boost::shared_ptr<const SessionProperties> properties,
                          const std::string& schemaName );

      /// Destructor
      ~OracleTableBuilder();

      /// Returns the sql statement
      std::string statement() const;

      /// Returns the proper sql type
      static std::string sqlType( boost::shared_ptr<const SessionProperties> properties,
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
