// $Id: CoralCppUnitTest.h,v 1.7 2011/05/05 15:01:37 kalkhof Exp $
#ifndef COMMON_CORALCPPUNITTEST_H
#define COMMON_CORALCPPUNITTEST_H 1

// Include files
#include <stdexcept>
#include <iostream>
#include <cstdlib>

// Local include files
#include "CppUnit_headers.h"

namespace coral
{

  //--------------------------------------------------------------------------

  /** @class ProgressListener
   *
   *  Simple TestListener printing one line per test in the standard output.
   *
   *  Based on CppUnit::BriefTestProgressListener (copy and paste)
   *  using std::cout instead of std::cerr.
   *
   *  @author Marco Clemencic
   *  @date   2006-11-13
   */

  class ProgressListener : public CppUnit::TestListener
  {

  public:

    // Default constructor.
    ProgressListener() : m_lastTestFailed(false) {}

    // Destructor.
    virtual ~ProgressListener() {}

    void startTest( CppUnit::Test *test )
    {
      std::cout << test->getName();
      std::cout.flush();
      m_lastTestFailed = false;
    }

    void addFailure( const CppUnit::TestFailure &failure )
    {
      std::cout << " : " << (failure.isError() ? "error" : "assertion");
      m_lastTestFailed  = true;
    }

    void endTest( CppUnit::Test * /*test*/ )
    {
      if ( !m_lastTestFailed )
        std::cout <<  " : OK";
      std::cout << std::endl;
    }

  private:

    bool m_lastTestFailed;

  };


  //---------------------------------------------------------------------------

  /** @class CoralCppUnitTest
   *
   *  Based on CoolUnitTest, the base class for COOL tests that do not use
   *  a database (CoolDBUnitTest is used for tests involving databases).
   *  Copied and reused for CORAL unit and integration tests.
   *
   *  @author Marco Clemencic 2006-05-09 (COOL)
   *  @author Andrea Valassi  2010-09-30 (CORAL)
   */
  class CoralCppUnitTest : public CppUnit::TestFixture
  {

  public:

    // Standard constructor
    CoralCppUnitTest(){}

    // Destructor
    virtual ~CoralCppUnitTest(){}

    // Parse the command-line arguments
    // You may redefine this static method in derived classes if needed
    static bool ParseArguments( int argc, char* argv[] );

    // Program main
    static int Main();

    // Add prefix to table names to make them unique across lots and platforms
    inline static const std::string BuildUniqueTableName( const std::string& name );

  protected:

    // The test path retrieved from the command line arguments (non-const ref).
    static std::string& TestPath()
    {
      static std::string s_testPath = "";
      return s_testPath;
    }

  };

  //---------------------------------------------------------------------------

  inline bool CoralCppUnitTest::ParseArguments( int argc, char** argv )
  {
    if ( argc == 1 )
    {
      return true; // OK (default testPath="")
    }
    else if ( argc > 2 || std::string( argv[1] ) == "-h" )
    {
      std::cout << "Usage: " << argv[0]
                << " [-h | CppUnit subtest path]" << std::endl;
      return false;
    }
    else
    {
      TestPath() = argv[1];
      return true;
    }
  }

  //---------------------------------------------------------------------------

  inline int CoralCppUnitTest::Main()
  {
    // Add a listener that collects test result
    //CppUnit::TestResultCollector result;
    //controller.addListener( &result );

    // Get the top level suite from the registry
    CppUnit::Test* suite =
      CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Add the test to the list of tests to run.
    // Pass a Test pointer and let the CppUnit runner delete it.
    CppUnit::TextTestRunner runner;
    runner.addTest( suite );

    // Change the default outputter to a compiler error format outputter
    // uncomment the following line if you need a compiler outputter.
    // Pass an Outputter pointer and let the CppUnit runner delete it.
    runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(),
                                                         std::cout ) );

    // Change the default outputter to a xml error format outputter
    // uncomment the following line if you need a xml outputter.
    // Pass an Outputter pointer and let the CppUnit runner delete it.
    //runner.setOutputter( new CppUnit::XmlOutputter( &runner.result(),
    //                                                std::cout ) );

    // Add a listener that prints test progress
    coral::ProgressListener listener;
    runner.eventManager().addListener( &listener );

    // Add a listener that collects test result
    //CppUnit::TestResultCollector collector;
    //runner.eventManager().addListener( &collector );

    // Run the tests
    bool wasSuccessful = false;
    try
    {
      wasSuccessful = runner.run( TestPath(), false, true, false );
    }

    // Test path not resolved
    catch ( std::invalid_argument &e )
    {
      std::cout << std::endl << "ERROR: " << e.what() << std::endl;
      return 1;
    }

    // Should never happen?
    catch ( std::exception& e )
    {
      std::cout << std::endl
                << "UNEXPECTED STD EXCEPTION CAUGHT in CoralUnitTest: "
                << e.what() << std::endl;
      return 1;
    }

    // Should never happen?
    catch ( ... )
    {
      std::cout << std::endl
                << "UNKNOWN EXCEPTION CAUGHT in CoralUnitTest"
                << std::endl;
      return 1;
    }

    // Return error code 1 if the one of tests failed.
    // Print a message on standard error if something failed (for QMTest)
    if ( ! wasSuccessful ) std::cerr << "Error: CppUnit Failures" << std::endl;
    int retcode = wasSuccessful ? 0 : 1;
    // Uncomment the next line if you want to integrate CppUnit with Oval
    std::cout << "[OVAL] Cppunit-result =" << retcode << std::endl;
    return retcode;

  }

  //---------------------------------------------------------------------------

  inline const std::string
  CoralCppUnitTest::BuildUniqueTableName( const std::string& name )
  {
    // The unique prefix across nightly slots
    static std::string slotPrefix = "";
    if ( slotPrefix == "" )
    {
      const char* slotname_c = ::getenv( "LCG_NGT_SLT_NAME" );
      const std::string slotname( slotname_c ? slotname_c : "" );
      if( slotname == "" )
        slotPrefix = "N";
      else if( slotname == "dev" )
        slotPrefix = "A";
      else if ( slotname == "dev1" )
        slotPrefix = "B";
      else if ( slotname == "dev2" )
        slotPrefix = "C";
      else if ( slotname == "dev3" )
        slotPrefix = "D";
      else if ( slotname == "dev4" )
        slotPrefix = "E";
      else if ( slotname == "release" )
        slotPrefix = "R";
      else if ( slotname == "test" )
        slotPrefix = "T";
      else if ( slotname == "test1" )
        slotPrefix = "U";
      else if ( slotname == "test2" )
        slotPrefix = "V";
      else
        slotPrefix = "Z";
    }
    // The unique prefix across platforms
    static std::string platformPrefix = "";
    if ( platformPrefix == "" )
    {
      const char* cmtconfig_c = ::getenv( "CMTCONFIG" );
      const std::string cmtconfig( cmtconfig_c ? cmtconfig_c : "" );
      if( cmtconfig == "" )
        platformPrefix = "NIL";
      else if ( cmtconfig == "slc4_ia32_gcc34" )
        platformPrefix = "4TO";
      else if ( cmtconfig == "slc4_ia32_gcc34_dbg" )
        platformPrefix = "4TD";
      else if ( cmtconfig == "slc4_amd64_gcc34" )
        platformPrefix = "4SO";
      else if ( cmtconfig == "slc4_amd64_gcc34_dbg" )
        platformPrefix = "4SD";
      else if ( cmtconfig == "i686-slc5-gcc43-opt" )
        platformPrefix = "5TO";
      else if ( cmtconfig == "i686-slc5-gcc43-dbg" )
        platformPrefix = "5TD";
      else if ( cmtconfig == "i686-slc5-gcc43-cov" )
        platformPrefix = "5TC";
      else if ( cmtconfig == "x86_64-slc5-gcc43-opt" )
        platformPrefix = "5SO";
      else if ( cmtconfig == "x86_64-slc5-gcc43-dbg" )
        platformPrefix = "5SD";
      else if ( cmtconfig == "x86_64-slc5-gcc43-cov" )
        platformPrefix = "5SC";
      else if ( cmtconfig == "x86_64-slc5-llvm27-dbg" )
        platformPrefix = "5SL";
      else if ( cmtconfig == "x86_64-slc5-gccmax-dbg" )
        platformPrefix = "5SM";
      else if ( cmtconfig == "x86_64-slc5-icc11-dbg" )
        platformPrefix = "5SI";
      else if ( cmtconfig == "i386-mac106-gcc42-opt" )
        platformPrefix = "MTO";
      else if ( cmtconfig == "i386-mac106-gcc42-dbg" )
        platformPrefix = "MTD";
      else if ( cmtconfig == "x86_64-mac106-gcc42-opt" )
        platformPrefix = "MSO";
      else if ( cmtconfig == "x86_64-mac106-gcc42-dbg" )
        platformPrefix = "MSD";
      else if ( cmtconfig == "win32_vc71_dbg" )
        platformPrefix = "VC7";
      else if ( cmtconfig == "i686-winxp-vc9-dbg" )
        platformPrefix = "VC9";
      else
        platformPrefix = "000";
    }
    return slotPrefix + platformPrefix + "_" + name;
  }

  //---------------------------------------------------------------------------

}

#define CORALCPPUNITTEST_MAIN(CLASS)                                    \
  int main( int argc, char** argv )                                     \
  {                                                                     \
    if ( coral::CLASS::ParseArguments( argc, argv ) ) coral::CLASS::Main(); \
    else return 1;                                                      \
  }

#endif // COMMON_CORALCPPUNITTEST_H
