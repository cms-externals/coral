// $Id: test_CppUnitExample.cpp,v 1.2 2010/10/05 20:31:55 avalassi Exp $

// Include files
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Forward declaration (for easier indentation)
namespace coral
{
  class CppUnitExampleTest;
}

// The test class
class coral::CppUnitExampleTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( CppUnitExampleTest );
  CPPUNIT_TEST( test_dummy1 );
  CPPUNIT_TEST( test_dummy2 );
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() {}

  void tearDown() {}

  // ------------------------------------------------------

  void test_dummy1()
  {
    int expectedI = 1234;
    int observedI = 1234;
    CPPUNIT_ASSERT_EQUAL_MESSAGE
      ( "I", expectedI, observedI );
  }

  // ------------------------------------------------------

  void test_dummy2()
  {
    int expectedI = 1234;
    int observedI = 1234;
    CPPUNIT_ASSERT_EQUAL_MESSAGE
      ( "I", expectedI, observedI );
  }

  // ------------------------------------------------------

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::CppUnitExampleTest );

CORALCPPUNITTEST_MAIN( CppUnitExampleTest )
