#ifndef CORALCOMMON_EXPRESSIONPARSER_H
#define CORALCOMMON_EXPRESSIONPARSER_H

#include <string>
#include <map>
#include <set>
#include <vector>

namespace coral
{
  class ITableDescription;
  class AttributeList;

  namespace CoralCommon {

    /// Simple class to identify table elements in an expression, appending
    /// double quotes arround each identified variable

    class ExpressionParser
    {
    public:
      /// Constructor
      ExpressionParser();

      /// Destructor
      virtual ~ExpressionParser();

      /// Adds to the table list
      void addToTableList( const coral::ITableDescription& description );

      /// Adds to the table list
      void addToTableList( const std::string& table, const std::vector<std::string>& outputVariables );

      /// Adds to the table aliases list
      void addToAliases( const std::string& alias );

      /// Appends to the bind variables
      void appendToBindVariables( const coral::AttributeList& bindData );

      /// Processes an expression and returns the parsed result
      std::string parseExpression( const std::string& expression ) const;

    private:
      /// Identifies a word with a known identifier and puts the double quotes if necessary
      std::string decorateWord( const std::string& expression, char previous ) const;

    private:
      /// The tables and the columns involved
      std::map< std::string, std::set< std::string> > m_tablesAndColumns;
      /// The table aliases
      std::set< std::string > m_tableAliases;
      /// The bind variables
      std::set< std::string> m_bindVariables;
    };

  }

}
#endif // CORALCOMMON_EXPRESSIONPARSER_H
