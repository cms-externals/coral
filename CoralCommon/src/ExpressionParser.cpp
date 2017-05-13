#include <sstream>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"

#include "CoralCommon/ExpressionParser.h"

#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ITableDescription.h"

using namespace coral::CoralCommon;


ExpressionParser::ExpressionParser()
  : m_tablesAndColumns()
  , m_tableAliases()
  , m_bindVariables()
{
  // EMPTY
}


ExpressionParser::~ExpressionParser()
{
  // EMPTY
}


void 
ExpressionParser::addToTableList( const coral::ITableDescription& description )
{
  std::set< std::string>& variables = m_tablesAndColumns[description.name()];
  int numberOfColumns = description.numberOfColumns();
  for ( int i = 0; i < numberOfColumns; ++i )
  {
    variables.insert(description.columnDescription(i).name());
  }
}


void 
ExpressionParser::addToTableList( const std::string& table, 
                                  const std::vector<std::string>& outputVariables )
{
  std::set< std::string>& variables = m_tablesAndColumns[table];
  for ( std::vector<std::string>::const_iterator iVar = outputVariables.begin(); iVar != outputVariables.end(); ++iVar )
    variables.insert(*iVar);
}


void 
ExpressionParser::addToAliases( const std::string& alias )
{
  if ( m_tableAliases.find(alias) == m_tableAliases.end() )
    m_tableAliases.insert(alias);
}


void 
ExpressionParser::appendToBindVariables( const coral::AttributeList& bindData )
{
  for ( coral::AttributeList::const_iterator iBindVariable = bindData.begin(); iBindVariable != bindData.end(); ++iBindVariable )
  {
    m_bindVariables.insert(iBindVariable->specification().name());
  }
}


std::string 
ExpressionParser::parseExpression( const std::string& expression ) const
{
  std::ostringstream os;
  std::string lastWord = "";
  bool withinQuotes = false;
  std::string::size_type length = expression.size();
  char previous = '\0';
  for ( std::string::size_type iCharacter = 0; iCharacter < length; ++iCharacter )
  {
    char c = expression[iCharacter];
    if (withinQuotes)
    {
      if (c == '\'')
        withinQuotes = false;
      os << c;
      continue;
    }
    if (c == '\'')
    {
      os << this->decorateWord(lastWord, previous);
      lastWord = "";
      os << c;
      withinQuotes = true;
      continue;
    }
    if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '`' ||
         c == '~' || c == '!' || c == '@' || c == '%' || c == '^' ||
         c == '&' || c == '*' || c == '(' || c == ')' || c == '-' ||
         c == '+' || c == '=' || c == '[' || c == ']' || c == '{' ||
         c == '}' || c == ';' || c == ':' || c == '"' || c == '\\' ||
         c == '|' || c == '<' || c == '>' || c == ',' || c == '.' ||
         c == '?' || c == '/' )
    {
      std::string finalWord = this->decorateWord(lastWord, previous);
      os << finalWord;
      lastWord = "";
      os << c;
      if ( finalWord.size() > 0 && finalWord[0] == '"' && previous == ':' )
        previous = '\0';
      else if ( finalWord.size() > 0 && finalWord[0] == '"' && previous == '.' )
        previous = '\0';
      else if ( c != ' ' && c != '\t' && c != '\n' && c != '\r' )
        previous = c;
    }
    else
    {
      lastWord += c;
    }
  }
  os << this->decorateWord(lastWord, previous);
  return os.str();
}


std::string 
ExpressionParser::decorateWord( const std::string& expression, 
                                char previous ) const
{
  if ( expression.empty() ) return "";
  if ( previous == ':' ) // This is a bind variable
  {
    if ( m_bindVariables.find(expression) != m_bindVariables.end() )
      return "\"" + expression + "\"";
    else
      return expression;
  }
  else if (previous == '.') // This is a variable in a table
  {
    // AV 05.10.10 why escape it ONLY if the table is known? (bug #73530...)
    for ( std::map<std::string, std::set< std::string> >::const_iterator iTable = m_tablesAndColumns.begin(); 
          iTable != m_tablesAndColumns.end();
          ++iTable )
    {
      if ( iTable->second.find(expression) != iTable->second.end() )
        return "\"" + expression + "\"";
    }
    return expression;
  }
  else
  {
    if ( m_tablesAndColumns.find(expression) != m_tablesAndColumns.end() ||
         m_tableAliases.find( expression ) != m_tableAliases.end() )
      return "\"" + expression + "\"";
    for ( std::map<std::string, std::set< std::string> >::const_iterator iTable = m_tablesAndColumns.begin();
          iTable != m_tablesAndColumns.end();
          ++iTable )
    {
      if (iTable->second.find(expression) != iTable->second.end())
        return "\"" + expression + "\"";
    }
    return expression;
  }
}
