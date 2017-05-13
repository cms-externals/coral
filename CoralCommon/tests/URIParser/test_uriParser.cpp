// Include files
#include "boost_tuple_headers.h"
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralCommon/URIParser.h"

// Forward declaration (for easier indentation)
namespace coral
{
  class URIParserTest;
}

// Test for URIParser
// Author: Zsolt Molnar
class coral::URIParserTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( URIParserTest );
  CPPUNIT_TEST( test_uriParser );
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() {}

  void tearDown() {}

  // ------------------------------------------------------

  typedef boost::tuple<std::string, std::string, std::string, std::string, int, std::string> URIParserTest_Results;

  URIParserTest_Results _testURIParser( const std::string& uri )
  {
    coral::URIParser parser;
    parser.setURI(uri);
    return URIParserTest_Results( parser.getURI(),
                                  parser.technology(),
                                  parser.protocol(),
                                  parser.hostName(),
                                  parser.portNumber(),
                                  parser.databaseOrSchemaName() );
  }

  void test_uriParser()
  {
    CPPUNIT_ASSERT_EQUAL( _testURIParser("oracle://dbhost/D9/user"),
                          URIParserTest_Results("oracle://dbhost/D9/user", "oracle", "", "dbhost", 0, "D9/user") );

    CPPUNIT_ASSERT_EQUAL( _testURIParser("oracle://dbhost/user"),
                          URIParserTest_Results("oracle://dbhost/user", "oracle", "", "dbhost", 0, "user") );

    CPPUNIT_ASSERT_EQUAL( _testURIParser("mysql://myhost/userdb"),
                          URIParserTest_Results("mysql://myhost/userdb", "mysql", "", "myhost", 0, "userdb") );

    CPPUNIT_ASSERT_EQUAL( _testURIParser("mysql://myhost:1105/userdb"),
                          URIParserTest_Results("mysql://myhost:1105/userdb", "mysql", "", "myhost", 1105, "userdb") );

    CPPUNIT_ASSERT_EQUAL( _testURIParser("xml_file:mylocalFile.xml"),
                          URIParserTest_Results("xml_file:mylocalFile.xml", "xml", "file", "", 0, "mylocalFile.xml") );

    CPPUNIT_ASSERT_EQUAL( _testURIParser("xml_file:/absolutePath.xml"),
                          URIParserTest_Results("xml_file:/absolutePath.xml", "xml", "file", "", 0, "/absolutePath.xml") );

    CPPUNIT_ASSERT_EQUAL( _testURIParser("xml_file:///absolutePath.xml"),
                          URIParserTest_Results("xml_file:///absolutePath.xml", "xml", "file", "", 0, "/absolutePath.xml"));

    CPPUNIT_ASSERT_EQUAL( _testURIParser("xml_http://myhost/absolutePath.xml"),
                          URIParserTest_Results("xml_http://myhost/absolutePath.xml", "xml", "http", "myhost", 0, "/absolutePath.xml") );
  }

  // ------------------------------------------------------

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::URIParserTest );

CORALCPPUNITTEST_MAIN( URIParserTest )
