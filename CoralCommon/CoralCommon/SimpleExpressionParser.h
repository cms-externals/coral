#ifndef CORALCOMMON_SIMPLEEXPRESSIONPARSER_H
#define CORALCOMMON_SIMPLEEXPRESSIONPARSER_H 1

#include <map>
#include <set>
#include <string>
#include <vector>

namespace coral
{

  class AttributeList;
  class ITableDescription;

  namespace CoralCommon
  {

    /// Simple class to identify table elements in an expression, appending
    /// double quotes around each identified variable
    /// This class only uses the syntax, it doesn't know about table, column
    /// or variable names.

    class SimpleExpressionParser
    {
    public:
      /// Constructor
      SimpleExpressionParser();

      /// Destructor
      virtual ~SimpleExpressionParser();

      /// Processes an expression and returns the parsed result
      std::string parseExpression( const std::string& expression ) const;

    private:

      std::string decorateWord( const std::string& word,
                                char previous,
                                char following = '\0' ) const;

      char nextNonWhitespaceChar( const std::string& expression,
                                  std::string::const_iterator pos ) const;

      bool isNumerical( const std::string& expression ) const;

      bool isReservedWord( const std::string& expression ) const;

    };

  }

}
#endif // CORALCOMMON_SIMPLEEXPRESSIONPARSER_H
