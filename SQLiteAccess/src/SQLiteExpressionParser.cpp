#include <iostream>
#include "SQLiteExpressionParser.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/IColumn.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include <sstream>
//#include <iostream>
coral::SQLiteAccess::SQLiteExpressionParser::SQLiteExpressionParser() : m_tablesAndColumns(){

}

coral::SQLiteAccess::SQLiteExpressionParser::~SQLiteExpressionParser(){

}

void
coral::SQLiteAccess::SQLiteExpressionParser::addToTableList( const std::string& tablealias, const coral::ITableDescription& description )
{

  if(!tablealias.empty()) m_aliasList.insert(tablealias);
  std::set< std::string >& variables = m_tablesAndColumns[ description.name() ];
  int numberOfColumns = description.numberOfColumns();
  for ( int i = 0; i < numberOfColumns; ++i ) {
    variables.insert( description.columnDescription( i ).name() );
  }
}

void
coral::SQLiteAccess::SQLiteExpressionParser::addToTableList( const std::string& tablealias,const std::string& table, const std::vector<std::string>& outputVariables )
{

  if(!tablealias.empty()) m_aliasList.insert(tablealias);
  std::set< std::string >& variables = m_tablesAndColumns[ table ];
  for ( std::vector<std::string>::const_iterator iVariable = outputVariables.begin();
        iVariable != outputVariables.end(); ++iVariable ) variables.insert( *iVariable );
}

std::string
coral::SQLiteAccess::SQLiteExpressionParser::parseExpression( const std::string& expression ) const {

  std::string os; os.reserve(expression.size());
  std::string lastWord;
  bool withinQuotes = false;
  char previous = '\0';
  for ( std::string::size_type i = 0; i < expression.size(); ++i ) {
    char c = expression[i];
    if ( withinQuotes ) { //within quote
      if ( c == '\'' ) withinQuotes = false;  //close quote
      os += c; //copy everything
      continue;
    }

    if ( c == '\'' ) { //start quote
      lastWord = "";
      os += c;
      withinQuotes = true; //open quote
      continue;
    }

    if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
         c == '`' || c == '~' || c == '!' || c == '@' || c == '%' ||
         c == '^' || c == '&' || c == '*' || c == '(' || c == ')' ||
         c == '-' || c == '+' || c == '=' || c == '[' || c == ']' ||
         c == '{' || c == '}' || c == ';' || c == ':' || c == '"' ||
         c == '\\' || c == '|' || c == '<' || c == '>' || c == ',' ||
         c == '.' || c == '?' || c == '/' ) { //special char
      os += this->decorateWord( lastWord, previous );
      lastWord.clear();

      if( !withinQuotes ) {
        os += c;
      }
      if ( c != ' ' && c != '\t' && c != '\n' && c != '\r' ) {
        previous = c;
      }else{
        previous = '\0';
      }
    }else{
      lastWord += c;
    }
  }
  os += this->decorateWord( lastWord, previous );
  //std::cout<<"expression parser output "<<os.str()<<std::endl;
  return os;
}

std::string
coral::SQLiteAccess::SQLiteExpressionParser::decorateWord( const std::string& expression,char previous ) const
{

  static std::string const quote("\"");
  if ( expression.empty() ) return "";
  if ( previous==':' ) {
    return expression;
  }
  if ( previous == '.' ) { // This is a variable in a table
    for ( std::map< std::string, std::set< std::string > >::const_iterator iTable = m_tablesAndColumns.begin(); iTable != m_tablesAndColumns.end(); ++iTable ) {
      if ( iTable->second.find( expression ) != iTable->second.end() )
        return "\"" + expression + "\"";
    }
    return expression;
  }else {
    if ( m_tablesAndColumns.find( expression ) !=  m_tablesAndColumns.end() ) {
      //This is a table name
      return quote + expression + quote;
    }else if( m_aliasList.size()!=0 && m_aliasList.find(expression)!=m_aliasList.end() ) {
      //This is an alias name
      return quote + expression + quote;
    }else if ( m_tablesAndColumns.size() > 0 )  {
      // This is a variable in the table
      std::map< std::string, std::set< std::string > >::const_iterator iTable = m_tablesAndColumns.begin();
      if ( iTable->second.find( expression ) != iTable->second.end() )
        return quote + expression + quote;
    }
    return expression;
  }
}
std::string coral::SQLiteAccess::SQLiteExpressionParser::replaceBindVariables( const std::string& inStmt) const {

  std::string os; os.reserve(inStmt.size());
  bool withinQuotes = false;
  char sep = ' ';
  for ( std::string::size_type i = 0; i < inStmt.size(); ++i ) {
    char c = inStmt[i];
    if ( withinQuotes ) { //within quote
      if ( c == '\'' ) withinQuotes = false;  //close quote
      os += c;
      continue;
    }
    if ( c == '\'' ) { //start quote
      withinQuotes = true; //open quote
      os +=c;
      continue;
    }
    if ( c == ':' &&  !withinQuotes ) {
      sep = c;
      os += '?';
      continue;
    }
    if ( sep ==':' && !withinQuotes ) {
      if( c == ' ' || c== ')' || c=='(' || c == '\t' || c == '\n' ||
          c == '\r'|| c == ',' || c == '<' || c == '!' || c == '>' ||
          c == '%' || c == '^' || c == '&' || c == '*' || c == '-' ||
          c == '+' || c == '=' || c == '[' || c == ']' || c == '{' ||
          c == '}' ) {
        os += c;
        sep = ' ';
        continue;
      }else{
        os +=' ';
        continue;
      }
    }
    os += c;
  }
  //  std::cout<<"result "<<os.str()<<std::endl;
  return os;
}
