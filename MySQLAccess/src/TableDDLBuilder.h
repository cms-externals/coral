// $Id: TableDDLBuilder.h,v 1.4 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_TABLEDDLBUILDER_H
#define CORAL_MYSQLACCESS_TABLEDDLBUILDER_H 1

#include <set>
#include <string>

namespace coral
{
  class ITypeConverter;
  class ITableDescription;

  namespace MySQLAccess
  {

    class TableDDLBuilder
    {
    public:
      TableDDLBuilder( const ITypeConverter& tc, const ITableDescription& td, const std::string& db );

      /// Builds the proper DDL SQL statement according to the given table description
      std::string sql();

      /// Checks environment variable for user supplied CLOB index prefix length
      unsigned int checkEnvClobPrefixLength() const;

    private:
      /// Builds DDL fragment of column definitions
      void defineColumns();

      /// Builds DDL fragment of PK definition
      void definePrimaryKey();

      /// Builds DDL fragment of UC definitions
      void defineUniqueConstraints();

      /// Builds DDL fragment of FK definitions
      void defineForeignKeys();

      /// Builds DDL fragment of index definitions
      void defineIndices();

      const ITypeConverter&    m_tc;
      const ITableDescription& m_td;
      const std::string m_db;
      std::string m_definition;
      std::set<std::string>    m_builtIdx;
    };
  }
}

#endif // CORAL_MYSQLACCESS_TABLEDDLBUILDER_H
