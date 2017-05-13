// $Id: test_CoralServerProxyException.cpp,v 1.1.2.1 2010/09/30 10:37:13 avalassi Exp $
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Include files
#include "CoralServerBase/CoralServerProxyException.h"

// Namespace
using namespace coral;

namespace coral 
{

  class CoralServerProxyExceptionTest : public CoralCppUnitTest 
  {

    CPPUNIT_TEST_SUITE( CoralServerProxyExceptionTest );
    CPPUNIT_TEST( test_ctor );
    CPPUNIT_TEST_SUITE_END();
		
  public:

    void setUp() {}

    void tearDown() {}

    // ------------------------------------------------------

    void test_ctor() 
    {
      std::string msg = "Exception";
      CoralServerProxyException cspExc( msg, "test" );
      Exception exc( msg, "test", "coral::CoralServerProxy" );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "CAL exceptionCode0", 
          (unsigned int)0x01, 
          (unsigned int)cspExc.asCALPayload().data()[0] );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "CAL exceptionCode1", 
          (unsigned int)0x00, 
          (unsigned int)cspExc.asCALPayload().data()[1] );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "CAL message", 
          std::string( exc.what() ), 
          std::string( cspExc.asCALPayload(), 2 ) );
      CPPUNIT_ASSERT_EQUAL_MESSAGE
        ( "what", 
          std::string( exc.what() ), 
          std::string( cspExc.what() ) );
    }

    // ------------------------------------------------------
 
  };
  
  CPPUNIT_TEST_SUITE_REGISTRATION( CoralServerProxyExceptionTest );
  
}

CORALCPPUNITTEST_MAIN( CoralServerProxyExceptionTest )
