// $Id: test_OpenSSLCipher.cpp,v 1.1.2.2 2010/09/30 10:37:12 avalassi Exp $

#include <iostream>

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

// Local include files
#include "../../src/OpenSSLCipher.h"

// Namespace
using namespace coral;

namespace coral
{

  class OpenSSLCipherTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( OpenSSLCipherTest );
#ifdef HAVE_OPENSSL
    CPPUNIT_TEST( test_EncryptDecrypt );
    CPPUNIT_TEST( test_LongString );
    CPPUNIT_TEST( test_Empty );
    CPPUNIT_TEST( test_CharacterRange );
#endif // HAVE_OPENSSL
    CPPUNIT_TEST( test_Base64 );
    CPPUNIT_TEST( test_Base64Empty );
    CPPUNIT_TEST( test_Base64LongString );
    CPPUNIT_TEST( test_Base64CharacterRange );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp() {}

    void tearDown() {
    }


    // ------------------------------------------------------

    void test_EncryptDecrypt()
    {
      OpenSSLCipher cipher;

      // normal encrypting works
      std::string secretData= "very very secret data";
      std::string crypt   = cipher.Encrypt("password", secretData );
      std::string decrypt = cipher.Decrypt("password", crypt );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("crypt->decrypt doesn't return the same value",
                                   decrypt, secretData );

      // crypting two times the same data should return different crypted data
      // (the salt is random)
      std::string crypt1 = cipher.Encrypt("password", secretData );
      if ( crypt1 == crypt )
        CPPUNIT_FAIL("crypt returned the same data twice");

    }

    // ------------------------------------------------------

    void test_LongString()
    {
      OpenSSLCipher cipher;

      // encrypting of long strings works
      std::string secretData;
      for (int i = 0; i<400; i++)
        secretData+="very ";
      secretData+="secret data";
      std::string crypt   = cipher.Encrypt("password", secretData );
      std::string decrypt = cipher.Decrypt("password", crypt );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("crypt->decrypt doesn't return the same value",
                                   decrypt, secretData );
    }

    // ------------------------------------------------------

    void test_Empty()
    {
      OpenSSLCipher cipher;

      // encrypting of empty strings works
      std::string secretData="";
      std::string crypt   = cipher.Encrypt("password", secretData );
      std::string decrypt = cipher.Decrypt("password", crypt );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("crypt->decrypt doesn't return the same value",
                                   decrypt, secretData );
    }

    // ------------------------------------------------------

    void test_CharacterRange()
    {
      OpenSSLCipher cipher;

      //check that all values from 0 to 255 are possible
      std::string secretData;
      for (int i = 0; i<512; i++)
        secretData+=(char) ( i & 0xff);
      std::string crypt   = cipher.Encrypt("password", secretData );
      std::string decrypt = cipher.Decrypt("password", crypt );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("crypt->decrypt doesn't return the same value",
                                   decrypt, secretData );
    }

    // ------------------------------------------------------

    void test_Base64()
    {
      std::string test="1";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="12";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="123";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="1234";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="12345";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="123456";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="1234567";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="12345678";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
      test="123456789";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("test base64 1", test, DecodeBase64( EncodeBase64( test ) ) );
    }

    // ------------------------------------------------------

    void test_Base64LongString()
    {
      // encrypting of long strings works
      std::string secretData;
      for (int i = 0; i<400; i++)
        secretData+="very ";
      secretData+="secret data";
      std::string crypt   = EncodeBase64(secretData );
      std::string decrypt = DecodeBase64(crypt );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("encode->decode doesn't return the same value",
                                   decrypt, secretData );
    }

    // ------------------------------------------------------

    void test_Base64Empty()
    {
      OpenSSLCipher cipher;

      // encrypting of empty strings works
      std::string secretData="";
      std::string crypt   = EncodeBase64(secretData );
      std::string decrypt = DecodeBase64(crypt );

      CPPUNIT_ASSERT_EQUAL_MESSAGE("encode->decode doesn't return the same value",
                                   decrypt, secretData );
    }

    // ------------------------------------------------------

    void test_Base64CharacterRange()
    {
      OpenSSLCipher cipher;

      //check that all values from 0 to 255 are possible
      std::string secretData;
      for (int i = 0; i<512; i++)
        secretData+=(char) ( i & 0xff);
      std::string crypt   = EncodeBase64(secretData );
      std::string decrypt = DecodeBase64(crypt );

      for (int i=0; i<512; i++)
        if ( decrypt.c_str()[i]!=secretData.c_str()[i] ) {
          std::cout << "Diffrence at " << i << ": " << decrypt.c_str()[i]<< "!="<<secretData.c_str()[i]
                    << std::endl;
          std::cout << "Diffrence at " << i << ": " << (int)decrypt.c_str()[i]<< "!="<<(int)secretData.c_str()[i]
                    << std::endl;
        }

      CPPUNIT_ASSERT_EQUAL_MESSAGE("encode->decode doesn't return the same value",
                                   decrypt, secretData );
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( OpenSSLCipherTest );

}

CORALCPPUNITTEST_MAIN( OpenSSLCipherTest )
