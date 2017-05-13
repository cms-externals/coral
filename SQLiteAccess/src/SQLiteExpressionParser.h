#ifndef SQLITEACCESS_SQLITEEXRPESSIONPARSER_H
#define SQLITEACCESS_SQLITEEXRPESSIONPARSER_H

#include <string>
#include <map>
#include <set>
#include <vector>
namespace coral {
  class ITableDescription;
  namespace SQLiteAccess {

    /**
       @class SQLiteExpressionParser SQLiteExpressionParser.h
    */

    class SQLiteExpressionParser
    {
    public:
      /// Constructor
      SQLiteExpressionParser();

      /// Destructor
      ~SQLiteExpressionParser();

      /// Processes an expression and returns the parsed result
      std::string parseExpression( const std::string& expression ) const;
      void addToTableList( const std::string& tablealias ,
                           const coral::ITableDescription& description );
      void addToTableList( const std::string& tablealias, const std::string& table, const std::vector<std::string>& outputVariables );
      std::string decorateWord( const std::string& expression,
                                char previous ) const;
      std::string replaceBindVariables( const std::string& inStmt ) const;
    private:
      /// The tables and the columns involved
      std::map< std::string, std::set< std::string > > m_tablesAndColumns;
      /// table and alias list
      std::set< std::string > m_aliasList;
    };
  }
}
#endif
