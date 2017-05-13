// $Id: test_ByteBuffer.cpp,v 1.5.2.3 2011/12/10 22:52:28 avalassi Exp $
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Include files
#include <cstring> // For memcpy
#include <sstream>
#include "CoralBase/Exception.h"
#include "CoralServerBase/ByteBuffer.h"
#ifdef _WIN32
#include "CoralServerBase/wintypes.h" // For uint32_t on Windows
#else
#include <stdint.h> // For uint32_t on gcc46 (bug #89762)
#endif

// Namespace
using namespace coral;

namespace coral
{

  class ByteBufferTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( ByteBufferTest );
    CPPUNIT_TEST( test_buffer );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {}

    // ------------------------------------------------------

    void test_buffer()
    {
      {
        ByteBuffer buffer;
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "maxSize0", (size_t)0, buffer.maxSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "usedSize0", (size_t)0, buffer.usedSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "freeSize0", (size_t)0, buffer.freeSize() );
        std::string expOut =
          "ByteBuffer[ maxSize=0, usedSize=0, ascii='', hex=0x ]";
        std::stringstream out;
        out << buffer;
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "print0", expOut, out.str() );
      }
      {
        ByteBuffer buffer( 10 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "maxSize10a", (size_t)10, buffer.maxSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "usedSize10a", (size_t)0, buffer.usedSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "freeSize10a", (size_t)10, buffer.freeSize() );
        {
          std::string expOut =
            "ByteBuffer[ maxSize=10, usedSize=0, ascii='', hex=0x ]";
          std::stringstream out;
          out << buffer;
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( "print10a", expOut, out.str() );
        }
        // Write data and check again
        uint32_t iHex = 0xF1F2F3F4; // Non-printable
        ::memcpy( buffer.data()+buffer.usedSize(), &iHex, 4 );
        buffer.setUsedSize( buffer.usedSize()+4 );
        uint32_t iCiao = 0x6F616943; // "oaiC"... will be streamed as "Ciao"!
        ::memcpy( buffer.data()+buffer.usedSize(), &iCiao, 4 );
        buffer.setUsedSize( buffer.usedSize()+4 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "maxSize10b", (size_t)10, buffer.maxSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "usedSize10b", (size_t)8, buffer.usedSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "freeSize10b", (size_t)2, buffer.freeSize() );
        {
          std::string expOut =
            "ByteBuffer[ maxSize=10, usedSize=8, ascii='....Ciao', "
            "hex=0xF4F3F2F14369616F ]";
          std::stringstream out;
          out << buffer;
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( "print10b", expOut, out.str() );
        }
        // Resize and check again
        buffer.resize( 4 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "maxSize10c", (size_t)4, buffer.maxSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "usedSize10c", (size_t)4, buffer.usedSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "freeSize10c", (size_t)0, buffer.freeSize() );
        {
          std::string expOut =
            "ByteBuffer[ maxSize=4, usedSize=4, ascii='....', "
            "hex=0xF4F3F2F1 ]";
          std::stringstream out;
          out << buffer;
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( "print10c", expOut, out.str() );
        }
        // Resize and check again
        buffer.resize( 10 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "maxSize10d", (size_t)10, buffer.maxSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "usedSize10d", (size_t)4, buffer.usedSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
          ( "freeSize10d", (size_t)6, buffer.freeSize() );
        {
          std::string expOut =
            "ByteBuffer[ maxSize=10, usedSize=4, ascii='....', "
            "hex=0xF4F3F2F1 ]";
          std::stringstream out;
          out << buffer;
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( "print10d", expOut, out.str() );
        }
        // Move the used size beyond the allocated max size
        try
        {
          buffer.setUsedSize( 11 );
          CPPUNIT_FAIL( "Expected an exception" );
        }
        catch( std::exception& e )
        {
          std::stringstream expMsg;
          expMsg << "Attempt to set new usedSize > maxSize";
          CPPUNIT_ASSERT_EQUAL_MESSAGE
            ( "Exception message",
              expMsg.str(),
              std::string( e.what(),
                           expMsg.str().size() ) ); // cut trailing CORAL info
        }
      }
    }

    // ------------------------------------------------------

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( ByteBufferTest );

}

CORALCPPUNITTEST_MAIN( ByteBufferTest )
