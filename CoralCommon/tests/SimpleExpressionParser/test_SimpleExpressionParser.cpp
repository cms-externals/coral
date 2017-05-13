// Include files
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralCommon/SimpleExpressionParser.h"

using namespace coral::CoralCommon;
namespace coral
{

  class SimpleExpressionParserTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( SimpleExpressionParserTest );
    CPPUNIT_TEST( test_columns );
    CPPUNIT_TEST( test_orderBy );
    CPPUNIT_TEST( test_condition );
    CPPUNIT_TEST( test_selectItems );
    CPPUNIT_TEST_SUITE_END();

  private:

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    SimpleExpressionParserTest():CoralCppUnitTest()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_columns()
    {
      SimpleExpressionParser parser;
      // escape mixedcase column names (ALWAYS)
      CPPUNIT_ASSERT_EQUAL( std::string("\"columnName\""),
                            parser.parseExpression("columnName") );
      // escape lowercase column names (Oracle ONLY?)
      CPPUNIT_ASSERT_EQUAL( std::string("\"columnname\""),
                            parser.parseExpression("columnname") );
      // do not escape uppercase column names (Oracle ONLY?)
      CPPUNIT_ASSERT_EQUAL( std::string("COLUMNNAME"),
                            parser.parseExpression("COLUMNNAME") );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_orderBy()
    {
      SimpleExpressionParser parser;
      // do not escape reserved words (uppercase)
      CPPUNIT_ASSERT_EQUAL( std::string("\"columnName\" ASC"),
                            parser.parseExpression("columnName ASC") );
      CPPUNIT_ASSERT_EQUAL( std::string("\"columnName\" DESC"),
                            parser.parseExpression("columnName DESC") );
      // do not escape reserved words (even if lowercase)
      CPPUNIT_ASSERT_EQUAL( std::string("\"columnName\" asc"),
                            parser.parseExpression("columnName asc") );
      CPPUNIT_ASSERT_EQUAL( std::string("\"columnName\" desc"),
                            parser.parseExpression("columnName desc") );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_condition()
    {
      SimpleExpressionParser parser;
      // do not escape uppercase bind variables
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\"= :BINDVARIABLE"),
          parser.parseExpression("columnName= :BINDVARIABLE") );
      // do escape mixedcase bind variables
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\"= :\"bindVariable\""),
          parser.parseExpression("columnName= :bindVariable") );
      // do not re-escape already escaped bind variables
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\"= :\"bindVariable\""),
          parser.parseExpression("columnName= :\"bindVariable\"") );
      // do not escape numbers
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\"=023"),
          parser.parseExpression("columnName=023") );
      // do not escape "IS NULL"
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\" IS NULL"),
          parser.parseExpression("columnName IS NULL") );
      // do not escape "is null"
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\" is null"),
          parser.parseExpression("columnName is null") );
      // do not escape "IS NOT NULL"
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\" IS NOT NULL"),
          parser.parseExpression("columnName IS NOT NULL") );
      // do not escape "is not null"
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\" is not null"),
          parser.parseExpression("columnName is not null") );
      // do not escape "like" (bug #89370)
      CPPUNIT_ASSERT_EQUAL
        ( std::string("\"columnName\" like '%aaa%'"),
          parser.parseExpression("columnName like '%aaa%'") );

      // more complex conditions from COOL
      CPPUNIT_ASSERT_EQUAL
        ( std::string("COOL_I3.I IS NULL"),
          parser.parseExpression("COOL_I3.I IS NULL") );
      CPPUNIT_ASSERT_EQUAL
        ( std::string("COOL_I3.I IS NOT NULL"),
          parser.parseExpression("COOL_I3.I IS NOT NULL") );
      CPPUNIT_ASSERT_EQUAL
        ( std::string("PARENT_NODEID= :\"parentNodeId\" OR CHILD_NODEID= :\"childNodeId\""),
          parser.parseExpression("PARENT_NODEID= :parentNodeId OR CHILD_NODEID= :childNodeId") );
      CPPUNIT_ASSERT_EQUAL
        ( std::string("C.HAS_NEW_DATA = :\"hasNewData\" AND C.LAST_OBJECT_ID = O.OBJECT_ID"),
          parser.parseExpression("C.HAS_NEW_DATA = :hasNewData AND C.LAST_OBJECT_ID = O.OBJECT_ID") );
      CPPUNIT_ASSERT_EQUAL // see https://savannah.cern.ch/bugs/?87111#comment9
        ( std::string("C.HAS_NEW_DATA = :\"hasNewData\" and C.LAST_OBJECT_ID = O.OBJECT_ID"),
          parser.parseExpression("C.HAS_NEW_DATA = :hasNewData and C.LAST_OBJECT_ID = O.OBJECT_ID") );
      CPPUNIT_ASSERT_EQUAL
        ( std::string("COOL_C2.CHANNEL_ID=:\"chid\" AND COOL_I3.USER_TAG_ID=:\"utagid3\" AND COOL_I3.NEW_HEAD_ID=0 AND COOL_I3.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I3.IOV_SINCE>=COALESCE(( SELECT /*+ QB_NAME(MAX1)  */ MAX(COOL_I1.IOV_SINCE) FROM WACHE.COOLTEST_F0002_IOVS COOL_I1 WHERE COOL_I1.USER_TAG_ID=:\"utagid1\" AND COOL_I1.NEW_HEAD_ID=0 AND COOL_I1.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I1.IOV_SINCE<=:\"since1\" ),:\"sinc3s\") AND COOL_I3.IOV_SINCE<=:\"until3\" AND COOL_I3.IOV_UNTIL>:\"sinc3u\" AND ( COOL_I3.I IS NULL )'"),
          parser.parseExpression("COOL_C2.CHANNEL_ID=:\"chid\" AND COOL_I3.USER_TAG_ID=:\"utagid3\" AND COOL_I3.NEW_HEAD_ID=0 AND COOL_I3.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I3.IOV_SINCE>=COALESCE(( SELECT /*+ QB_NAME(MAX1)  */ MAX(COOL_I1.IOV_SINCE) FROM WACHE.COOLTEST_F0002_IOVS COOL_I1 WHERE COOL_I1.USER_TAG_ID=:\"utagid1\" AND COOL_I1.NEW_HEAD_ID=0 AND COOL_I1.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I1.IOV_SINCE<=:\"since1\" ),:\"sinc3s\") AND COOL_I3.IOV_SINCE<=:\"until3\" AND COOL_I3.IOV_UNTIL>:\"sinc3u\" AND ( COOL_I3.I IS NULL )'") );

      CPPUNIT_ASSERT_EQUAL
        ( std::string("COOL_C2.CHANNEL_ID=:\"chid\" AND COOL_I3.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I3.IOV_SINCE>=COALESCE(( SELECT /*+ QB_NAME(MAX1)  */ MAX(COOL_I1.IOV_SINCE) FROM WACHE.COOLTEST_F0001_IOVS COOL_I1 WHERE COOL_I1.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I1.IOV_SINCE<=:\"since1\" ),:\"sinc3s\") AND COOL_I3.IOV_SINCE<=:\"until3\" AND COOL_I3.IOV_UNTIL>:\"sinc3u\""),
          parser.parseExpression("COOL_C2.CHANNEL_ID=:\"chid\" AND COOL_I3.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I3.IOV_SINCE>=COALESCE(( SELECT /*+ QB_NAME(MAX1)  */ MAX(COOL_I1.IOV_SINCE) FROM WACHE.COOLTEST_F0001_IOVS COOL_I1 WHERE COOL_I1.CHANNEL_ID=COOL_C2.CHANNEL_ID AND COOL_I1.IOV_SINCE<=:\"since1\" ),:\"sinc3s\") AND COOL_I3.IOV_SINCE<=:\"until3\" AND COOL_I3.IOV_UNTIL>:\"sinc3u\"") );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_selectItems()
    {
      SimpleExpressionParser parser;
      CPPUNIT_ASSERT_EQUAL
        ( std::string("COUNT(*)"),
          parser.parseExpression("COUNT(*)") );
      CPPUNIT_ASSERT_EQUAL
        ( std::string("/*+ QB_NAME(MAIN) INDEX(@MAIN COOL_I3@MAIN (CHANNEL_ID IOV_SINCE IOV_UNTIL)) LEADING(@MAIN COOL_C2@MAIN COOL_I3@MAIN) USE_NL(@MAIN COOL_I3@MAIN) INDEX(@MAX1 COOL_I1@MAX1 (CHANNEL_ID IOV_SINCE IOV_UNTIL))  */ COOL_I3.OBJECT_ID"),
          parser.parseExpression("/*+ QB_NAME(MAIN) INDEX(@MAIN COOL_I3@MAIN (CHANNEL_ID IOV_SINCE IOV_UNTIL)) LEADING(@MAIN COOL_C2@MAIN COOL_I3@MAIN) USE_NL(@MAIN COOL_I3@MAIN) INDEX(@MAX1 COOL_I1@MAX1 (CHANNEL_ID IOV_SINCE IOV_UNTIL))  */ COOL_I3.OBJECT_ID") );
      CPPUNIT_ASSERT_EQUAL
        ( std::string("/*+ INDEX(C) INDEX_RS_ASC(O) LEADING(C O) USE_NL(C O) */ O.OBJECT_ID"),
          parser.parseExpression("/*+ INDEX(C) INDEX_RS_ASC(O) LEADING(C O) USE_NL(C O) */ O.OBJECT_ID") );

    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( SimpleExpressionParserTest );

}

CORALCPPUNITTEST_MAIN( SimpleExpressionParserTest )
