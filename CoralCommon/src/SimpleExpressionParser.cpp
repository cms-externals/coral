#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include "CoralCommon/SimpleExpressionParser.h"

using namespace coral::CoralCommon;


inline const std::string uppercase( const std::string& _expression ) 
{
  std::string expression = _expression;
  std::transform( expression.begin(), expression.end(), 
                  expression.begin(), toupper );
  return expression;
}


SimpleExpressionParser::SimpleExpressionParser()
{
  // EMPTY
}


SimpleExpressionParser::~SimpleExpressionParser()
{
  // EMPTY
}


bool
SimpleExpressionParser::isReservedWord( const std::string& expression ) const
{
  static char reservedWords[][20] = { 
    // 'order by' and 'group by' clause
    "DESC",
    "ASC",
    "ORDER",
    "GROUP",
    "BY",
    // conditions
    "OR",
    "AND",
    "IS",
    "NOT",
    "NULL",
    "IN",
    "LIKE", // bug #89370
    // subquery
    "SELECT",
    "FROM",
    "AS",
    "WHERE",
    // NB: Do not include SQL functions here, eg SYSTIMESTAMP (bug #87111)
    // You may rather create a separate isSQLFunction() method...
  };
  static size_t 
    maxReservedWords = sizeof( reservedWords ) / sizeof( reservedWords[0] );
  for ( size_t i = 0; i<maxReservedWords; i++ )
  {
    //std::cout << "isReservedWord " << reservedWords[i] << std::endl;
    // Reserved words can be lower, upper or mixed case (bug #87111 "part1")
    if ( uppercase(expression) == std::string(reservedWords[i]) ) return true;
  }
  return false;
}


char
SimpleExpressionParser::nextNonWhitespaceChar( const std::string& expression,
                                               std::string::const_iterator pos ) const
{
  std::string::const_iterator it = pos;
  while ( it!= expression.end() )
  {
    if ( *it != ' ' && *it != '\t' && *it != '\n' && *it != '\r' ) return *it;
    ++it;
  }
  return '\0';
}


bool
SimpleExpressionParser::isNumerical( const std::string& expression ) const
{
  std::string::const_iterator it = expression.begin();
  while ( it!= expression.end() )
  {
    if ( *it < '0' || *it > '9' ) return false;
    ++it;
  }
  return true;
}


std::string
SimpleExpressionParser::parseExpression( const std::string& expression ) const
{
  //std::cout << "parseExpression '"<< expression<< "'"<< std::endl;
  if ( isReservedWord( expression ) )
  {
    // Check first if this is a reserved word used as column name...
    return "\"" + expression + "\""; // fix bug #91075
  }
  std::ostringstream os;
  std::string lastWord = "";
  bool withinQuotes = false;
  bool withinComment = false;
  char previous = '\0';
  for ( std::string::const_iterator it = expression.begin();
        it != expression.end() ; ++it )
  {
    char c = *it;
    if ( withinQuotes )
    {
      if ( c == '\'' ) withinQuotes = false;
      os << c;
      continue;
    }
    if ( withinComment )
    {
      std::string::const_iterator it2=it;
      if ( c == '*' && it!=expression.end() && *(++it2) == '/' )
      {
        withinComment = false;
        os << c << *(++it);
        continue;
      }
      os << c;
      continue;
    }
    if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '`' ||
         c == '~' || c == '!' || c == '@' || c == '%' || c == '^' ||
         c == '&' || c == '*' || c == '(' || c == ')' || c == '-' ||
         c == '+' || c == '=' || c == '[' || c == ']' || c == '{' ||
         c == '}' || c == ';' || c == ':' || c == '"' || c == '\\' ||
         c == '|' || c == '<' || c == '>' || c == ',' || c == '.' ||
         c == '?' || c == '/' || c == '\'' )
    {
      // a "word" ends here
      char following = this->nextNonWhitespaceChar( expression, it );
      std::string finalWord = this->decorateWord( lastWord,
                                                  previous,
                                                  following );
      os << finalWord;
      lastWord = "";
      os << c;
      std::string::const_iterator it2 = it;
      if ( c == '/' && it!=expression.end() && *(++it2) == '*' )
      {
        withinComment = true;
        os << *(++it);
        continue;
      }
      if ( c == '\'' )
      {
        withinQuotes = true;
        continue;
      }
      // remember character before new word, ignoring white spaces etc
      //std::cout << "previous: '" << previous << "'" << std::endl;
      if ( c != ' ' && c != '\t' && c != '\n' && c != '\r' ) 
        previous = c;
      else 
        previous = '\0'; // fix https://savannah.cern.ch/bugs/?87111#comment9
      //std::cout << "previous: '" << previous << "'" << std::endl;
    }
    else
    {
      // a "word" character
      lastWord += c;
    }
  }
  os << this->decorateWord( lastWord, previous );
  //std::cout << "parseExpression returns '" << os.str() << "'" << std::endl;
  return os.str();
}


std::string
SimpleExpressionParser::decorateWord( const std::string& word,
                                      char previous,
                                      char following ) const
{
  //std::cout << "decorateWord: '" << word << "', prev: '" << previous << "', foll: '" << following << "'" << std::endl;
  if ( word.empty() )
  {
    // empty string
    return "";
  }
  else if ( previous == '"' && following == '"' )
  {
    // already in quotes
    return word;
  }
  else if ( isNumerical( word ) )
  {
    // don't quote numbers
    return word;
  }
  else if ( previous == ':' || // this is a bind variable
            previous == '.' || // this is a variable in a table
            following == '.' ) // this is a table/view name
  {
    if ( isReservedWord( word ) )
      return "\"" + word + "\""; // escape ALL reserved words used as names
    else if ( word == uppercase( word ) )
      return word; // do not escape uppercase names (for simplicity)
    else
      return "\"" + word + "\""; // quote lowercase and mixed case names
  }
  else if ( following == '(' )
  {
    // either a function or a boolean function (and, or)
    return word;
  }
  else if ( isReservedWord( word ) )
  {
    // don't quote reserved words
    return word;
  }
  else if ( word == uppercase( word ) )
  {
    // No need to quote uppercase words, only lowercase and mixedcase words 
    // may need quoting (is this Oracle-specific? is this different in MySQL?).
    // This essentially represents an internal convention to allow CORAL users
    // (such as COOL code) to freely use additional SQL reserved expressions
    // (such as 'SYSTIMESTAMP WITH TIME ZONE'), bypassing bug #87111 "part2".
    return word;
  }
  /*
  // This is no longer needed after reordering the checks
  // See https://savannah.cern.ch/bugs/?87111#comment7
  else if ( previous == '\0' && following == '\0' )
  {
    // a single word is a column
    return "\"" + word + "\"";
  }
  */
  else
  {
    return "\"" + word + "\"";
  }
}
