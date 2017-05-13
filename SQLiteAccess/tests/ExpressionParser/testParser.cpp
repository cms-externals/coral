#include <iostream>
#include <string>
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "src/SQLiteExpressionParser.h"

// Forward declaration (for easier indentation)
namespace coral
{
  class ExpressionParserTest;
}

// The test class
class coral::ExpressionParserTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( ExpressionParserTest );
  CPPUNIT_TEST( test_parse );
  CPPUNIT_TEST_SUITE_END();

protected:

  //--------------------------------------------------------------------------

  void test_parse(){
    std::cout<<"[OVAL] --> testparse"<<std::endl;
    std::string result;
    std::vector<std::string> outlist;
    outlist.push_back("cola");
    outlist.push_back("colb");
    m_parser->addToTableList("","mytab",outlist);
    result=m_parser->parseExpression("select cola from mytab where cola=4");
    std::cout<<"result "<<result<<std::endl;
    result=m_parser->parseExpression("insert into mytab values(:x, :y, :z)");
    std::cout<<"result "<<result<<std::endl;
    result=m_parser->parseExpression("select colb from mytab where colb=':a' ");
    std::cout<<"result "<<result<<std::endl;
    result=m_parser->parseExpression("select colb from mytab where colb=:a ");
    std::cout<<"result "<<result<<std::endl;
    result=m_parser->parseExpression("select colb from mytab where colb>=6 ");
    std::cout<<"result "<<result<<std::endl;

    result=m_parser->parseExpression("update mytab.colb set cola=:a where colb>=:b ");
    std::cout<<"result "<<result<<std::endl;

  }


  //--------------------------------------------------------------------------

private:

  coral::SQLiteAccess::SQLiteExpressionParser* m_parser;

public:

  void setUp(){
    std::cout<<"[OVAL] --> setUp"<<std::endl;
    m_parser=new coral::SQLiteAccess::SQLiteExpressionParser;
  }

  void tearDown(){
    std::cout << "[OVAL] --> tearDown" << std::endl;
    delete m_parser;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::ExpressionParserTest );

CORALCPPUNITTEST_MAIN( ExpressionParserTest )
