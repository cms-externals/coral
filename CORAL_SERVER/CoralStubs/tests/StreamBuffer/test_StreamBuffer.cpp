// Include files
#include <sstream>
#include "CoralBase/Exception.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/Blob.h"
#include "CoralServerBase/CALPacketHeader.h"

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

#include "CoralServerBase/ByteBuffer.h"
#include "../../src/CppTypes.h"
#include "../../src/AttributeUtils.h"

#include "../../src/SegmentWriterIterator.h"
#include "../../src/SegmentReaderIterator.h"
#include "../../src/DummyByteBufferIterator.h"

#include <vector>
#include <set>
#include <sstream>

#include <limits.h>

namespace coral
{

  template<class T1, class T2>
  void
  compareDataTypes(const std::string& s, const T1& t1, const T2& t2)
  {
    if(t1 != t2) {
      std::ostringstream s2;
      s2 << " [" << t1 << "]![" << t2 << "] size(" << sizeof(T1) << ")";
      CPPUNIT_FAIL("data missmatch (" + s + ")" + s2.str());
    }
  }

  template<>
  void
  compareDataTypes< std::string, std::string >(const std::string& s, const std::string& t1, const std::string& t2)
  {
    if(t1.compare(t2) != 0) {
      CPPUNIT_FAIL("data missmatch " + s);
    }
  }

  template<>
  void
  compareDataTypes< coral::Blob, coral::Blob >(const std::string& s, const coral::Blob& t1, const coral::Blob& t2)
  {
    if(t1 != t2) {
      CPPUNIT_FAIL("data missmatch " + s);
    }
  }

  template<>
  void
  compareDataTypes< coral::Date, coral::Date >(const std::string& s, const coral::Date& t1, const coral::Date& t2)
  {
    if(t1 != t2) {
      CPPUNIT_FAIL("data missmatch " + s);
    }
  }

  template<>
  void
  compareDataTypes< coral::TimeStamp, coral::TimeStamp >(const std::string& s, const coral::TimeStamp& t1, const coral::TimeStamp& t2)
  {
    if(t1 != t2) {
      CPPUNIT_FAIL("data missmatch " + s);
    }
  }

  class StreamBufferTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( StreamBufferTest );
    //   CPPUNIT_TEST( test_MainTypes );
    // CPPUNIT_TEST( test_ComplexTypes );
    //   CPPUNIT_TEST( test_CoralAttributeList );
    //   CPPUNIT_TEST( test_SegmentIterator );
    CPPUNIT_TEST( test_SISimpleTypes );
    CPPUNIT_TEST( test_SIComplexTypes );
    CPPUNIT_TEST( test_SICoralTypes );
    CPPUNIT_TEST( test_Opcodes );
    CPPUNIT_TEST_SUITE_END();

  public:

    void
    test_SISimpleTypesBuffer(size_t buffersize)
    {
      coral::CoralStubs::SegmentWriterIterator swi(coral::CALOpcodes::ConnectRO, true, false, buffersize);
      coral::CoralStubs::DummyByteBufferIterator dbi;
      coral::CoralStubs::SegmentReaderIterator sri(0, dbi);

      ///check unsigned char

      swi.append((unsigned char)0x42);

      unsigned char c1 = 'F';
      swi.append(c1);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      unsigned char c2 = 'H';

      sri.extract(c2);
      sri.extract(c2);

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "compare unsigned char", c1, c2 );

      ///check bool

      swi.append(true);
      swi.append(false);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      bool b01;
      bool b02;

      sri.extract(b01);
      sri.extract(b02);

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "compare bool", true, b01 );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "compare bool", false, b02 );

      ///check integers

      swi.append((uint16_t) 0);
      swi.append((uint16_t) 0xFFFF);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      uint16_t uint16_01 = 2345;
      uint16_t uint16_02 = 2345;

      sri.extract(uint16_01);
      sri.extract(uint16_02);

      if(uint16_01 != 0)
        CPPUNIT_FAIL("uint16 not equal");

      if(uint16_02 != 65535)
        CPPUNIT_FAIL("uint16 not equal");


      swi.append((uint32_t) 0);
      swi.append((uint32_t) 0xFFFFFFFF);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      uint32_t uint32_01 = 2345;
      uint32_t uint32_02 = 2345;

      sri.extract(uint32_01);
      sri.extract(uint32_02);

      if(uint32_01 != 0)
        CPPUNIT_FAIL("uint32 not equal");

      if(uint32_02 != 4294967295U)
        CPPUNIT_FAIL("uint32 not equal");


      swi.append((uint64_t) 0);
#if __SIZEOF_LONG__ == 4
      swi.append((uint64_t) 0xFFFFFFFF);
#elif __SIZEOF_LONG__ == 8
      swi.append((uint64_t) 0xFFFFFFFFFFFFFFFF);
#else
#error unknown long max
#endif
      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      uint64_t uint64_01 = 2345;
      uint64_t uint64_02 = 2345;

      sri.extract(uint64_01);
      sri.extract(uint64_02);

      if(uint64_01 != 0)
        CPPUNIT_FAIL("uint64 not equal");

#if __SIZEOF_LONG__ == 4
      if(uint64_02 != (uint64_t)0xFFFFFFFF)
#elif __SIZEOF_LONG__ == 8
        if(uint64_02 != (uint64_t)0xFFFFFFFFFFFFFFFF)
#endif

          CPPUNIT_FAIL("uint64 not equal");

      //     streamw.write((uint128_t) 0);

      uint128_t uint128_p2;

#if __SIZEOF_LONG__ == 4
      uint128_p2[0] = 0xFFAFF9FF;
      uint128_p2[1] = 0xF1FF3F00;
#elif __SIZEOF_LONG__ == 8
      uint128_p2[0] = 0xFFAFFFF5FFFFF9FF;
      uint128_p2[1] = 0xF1FFFFBFFFFF3F00;
#endif

      uint128_t uint128_p1;
      memcpy(&uint128_p1, &uint128_p2, 16);

      swi.append(uint128_p1);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      uint128_t uint128_01;

      sri.extract(uint128_01);

#if __SIZEOF_LONG__ == 4
      if(uint128_01[0] != 0xFFAFF9FF)
        CPPUNIT_FAIL("uint128 not equal");
      if(uint128_01[1] != 0xF1FF3F00)
        CPPUNIT_FAIL("uint128 not equal");
#elif __SIZEOF_LONG__ == 8
      if(uint128_01[0] != 0xFFAFFFF5FFFFF9FF)
        CPPUNIT_FAIL("uint128 not equal");
      if(uint128_01[1] != 0xF1FFFFBFFFFF3F00)
        CPPUNIT_FAIL("uint128 not equal");
#endif

      ///check string maxlength 16bit

      std::string s01 = "Hello world!";

      swi.append16(s01);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      std::string s02 = "Halo world";

      sri.extract16(s02);

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "compare string", s01, s02 );

      ///check string maxlength 32bit

      std::string s03 = "Hello world !!";

      swi.append32(s03);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      std::string s04 = "Halo world";

      sri.extract32(s04);

      CPPUNIT_ASSERT_EQUAL_MESSAGE( "compare string", s03, s04 );

    }

    void
    test_SIComplexTypesBuffer(size_t buffersize)
    {
      coral::CoralStubs::SegmentWriterIterator swi(coral::CALOpcodes::ConnectRO, true, false, buffersize);
      coral::CoralStubs::DummyByteBufferIterator dbi;
      coral::CoralStubs::SegmentReaderIterator sri(0, dbi);

      ///check vector of strings

      std::vector<std::string> v01;

      v01.push_back("line1");
      v01.push_back("line2");
      v01.push_back("line3");

      swi.append(v01);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      std::vector<std::string> v02;

      v02.push_back("column01");
      v02.push_back("column02");

      sri.extract(v02);

      std::vector<std::string>::iterator vi01 = v01.begin();
      std::vector<std::string>::iterator vi02;

      size_t counter = 0;
      for(vi02 = v02.begin(); vi02 != v02.end(); vi01++, vi02++, counter++ )
      {
        if(vi01->compare(*vi02) != 0)
          CPPUNIT_FAIL("vector of strings is not equal");
      }
      if(counter != v01.size())
        CPPUNIT_FAIL("vector of strings have not the same length");

      ///check set of strings

      std::set<std::string> set01;

      set01.insert("line1");
      set01.insert("line2");
      set01.insert("line3");

      swi.append(set01);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      std::set<std::string> set02;

      set02.insert("l1");
      set02.insert("l2");

      sri.extract(set02);

      std::set<std::string>::iterator si01 = set01.begin();
      std::set<std::string>::iterator si02;

      counter = 0;
      for(si02 = set02.begin(); si02 != set02.end(); si01++, si02++, counter++ )
      {
        if(si01->compare(*si02) != 0)
          CPPUNIT_FAIL("set of strings is not equal");
      }
      if(counter != set01.size())
        CPPUNIT_FAIL("set of strings have not the same length");

      ///check vector of string pairs

      std::vector< std::pair<std::string, std::string> > p01;

      p01.push_back(std::pair<std::string, std::string>("line1", "l1"));
      p01.push_back(std::pair<std::string, std::string>("line2", "l3"));
      p01.push_back(std::pair<std::string, std::string>("line3", "l2"));

      swi.append(p01);

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      std::vector< std::pair<std::string, std::string> > p02;

      p02.push_back(std::pair<std::string, std::string>("column1", "c1"));
      p02.push_back(std::pair<std::string, std::string>("column2", "c2"));

      sri.extract(p02);

      std::vector< std::pair<std::string, std::string> >::iterator pi01 = p01.begin();
      std::vector< std::pair<std::string, std::string> >::iterator pi02;

      counter = 0;
      for(pi02 = p02.begin(); pi02 != p02.end(); pi01++, pi02++, counter++ )
      {
        if((pi01->first.compare(pi02->first) != 0))
          CPPUNIT_FAIL("vector of string pairs is not equal");
        if((pi01->second.compare(pi02->second) != 0))
          CPPUNIT_FAIL("vector of string pairs is not equal");
      }
      if(counter != p01.size())
        CPPUNIT_FAIL("vector of string pairs have not the same length");


    }

    void
    test_SICoralTypesBuffer(size_t buffersize)
    {
      coral::CoralStubs::SegmentWriterIterator swi(coral::CALOpcodes::ConnectRO, true, false, buffersize);
      coral::CoralStubs::SegmentReaderIterator sri(0, swi);

      coral::AttributeList attr01;

      attr01.extend("C01", typeid(bool));
      attr01.extend("C02", typeid(char));
      attr01.extend("C03", typeid(unsigned char));
      attr01.extend("C04", typeid(signed char));
      attr01.extend("C05", typeid(short));
      attr01.extend("C06", typeid(unsigned short));
      attr01.extend("C07", typeid(int));
      attr01.extend("C08", typeid(unsigned int));
      attr01.extend("C09", typeid(long));
      attr01.extend("C10", typeid(unsigned long));
      attr01.extend("C11", typeid(long long));
      attr01.extend("C12", typeid(unsigned long long));
      attr01.extend("C13", typeid(float));
      attr01.extend("C14", typeid(double));
      attr01.extend("C15", typeid(long double));
      attr01.extend("C16", typeid(std::string));
      attr01.extend("C17", typeid(coral::Blob));
      attr01.extend("C18", typeid(coral::Date));
      attr01.extend("C19", typeid(coral::TimeStamp));

      attr01[0].data<bool>() = true;
      attr01[1].data<char>() = 0x42;
      attr01[2].data<unsigned char>() = 42;
      attr01[3].data<char>() = (signed char)-2; //bug #46922
      attr01[4].data<short>() = 42;
      attr01[5].data<unsigned short>() = 42;
      attr01[6].data<int>() = -3223;
      attr01[7].data<unsigned int>() = 0xFFFFFFFF;
      attr01[8].data<long>() = -123123;
      attr01[9].data<unsigned long>() = 123123;
      attr01[10].data<long long>() = -123123;
      attr01[11].data<unsigned long long>() = 123123;
      attr01[12].data<float>() = 12.42;
      attr01[13].data<double>() = 233.4543;
      attr01[14].data<long double>() = 233.4543;
      attr01[15].data<std::string>() = "Hello World!";
      attr01[17].data<coral::Date>() = coral::Date(2001, 1, 1);
      attr01[18].data<coral::TimeStamp>() = coral::TimeStamp(1972, 1, 1, 12, 32, 34, 0, false);

      coral::Blob& myblob = attr01[16].data<coral::Blob>();

      size_t maxc = 1000;
      myblob.resize( maxc + 1 );
      unsigned char* p = static_cast<unsigned char*>( myblob.startingAddress() );
      for ( size_t j = 0; j < maxc; ++j, ++p ) *p = rand() % 256;

      swi.appendV(attr01);

      swi.flush();

      //try first with a preset full attribute list

      coral::AttributeList attr02;

      attr02.extend("C01", typeid(bool));
      attr02.extend("C02", typeid(char));
      attr02.extend("C03", typeid(unsigned char));
      attr02.extend("C04", typeid(signed char));
      attr02.extend("C05", typeid(short));
      attr02.extend("C06", typeid(unsigned short));
      attr02.extend("C07", typeid(int));
      attr02.extend("C08", typeid(unsigned int));
      attr02.extend("C09", typeid(long));
      attr02.extend("C10", typeid(unsigned long));
      attr02.extend("C11", typeid(long long));
      attr02.extend("C12", typeid(unsigned long long));
      attr02.extend("C13", typeid(float));
      attr02.extend("C14", typeid(double));
      attr02.extend("C15", typeid(long double));
      attr02.extend("C16", typeid(std::string));
      attr02.extend("C17", typeid(coral::Blob));
      attr02.extend("C18", typeid(coral::Date));
      attr02.extend("C19", typeid(coral::TimeStamp));

      std::cout << "try reading back" << std::endl;

      sri.extractV(attr02);

      //compare both attribute lists

      size_t atrs01 = attr01.size();

      if(attr02.size() != atrs01)
        CPPUNIT_FAIL("AttributeList lengths are different");

      for(size_t i = 0; i < atrs01; i++ )
      {
        Attribute& attr_01 = attr01[i];
        Attribute& attr_02 = attr02[i];

        //compare the both attributes
        const AttributeSpecification& spec01 = attr_01.specification();
        const AttributeSpecification& spec02 = attr_02.specification();

        if(spec01.name().compare(spec02.name()) != 0)
          CPPUNIT_FAIL("AttributeList name is not equal");

        if(spec01.typeName().compare(spec02.typeName()) != 0)
          CPPUNIT_FAIL("AttributeList typename is not equal");

        if(spec01.type() != spec02.type())
          CPPUNIT_FAIL("AttributeList type is not equal");

        //compare the data

      }
      compareDataTypes(attr01[0].specification().typeName(), attr01[0].data<bool>(), attr02[0].data<bool>());
      compareDataTypes(attr01[1].specification().typeName(), attr01[1].data<char>(), attr02[1].data<char>());
      compareDataTypes(attr01[2].specification().typeName(), attr01[2].data<unsigned char>(), attr02[2].data<unsigned char>());
      compareDataTypes(attr01[3].specification().typeName(), attr01[3].data<char>(), attr02[3].data<char>()); //bug #46922
      compareDataTypes(attr01[4].specification().typeName(), attr01[4].data<short>(), attr02[4].data<short>());
      compareDataTypes(attr01[5].specification().typeName(), attr01[5].data<unsigned short>(), attr02[5].data<unsigned short>());
      compareDataTypes(attr01[6].specification().typeName(), attr01[6].data<int>(), attr02[6].data<int>());
      compareDataTypes(attr01[7].specification().typeName(), attr01[7].data<unsigned int>(), attr02[7].data<unsigned int>());
      compareDataTypes(attr01[8].specification().typeName(), attr01[8].data<long>(), attr02[8].data<long>());
      compareDataTypes(attr01[9].specification().typeName(), attr01[9].data<unsigned long>(), attr02[9].data<unsigned long>());
      compareDataTypes(attr01[10].specification().typeName(), attr01[10].data<long long>(), attr02[10].data<long long>());
      compareDataTypes(attr01[11].specification().typeName(), attr01[11].data<unsigned long long>(), attr02[11].data<unsigned long long>());
      compareDataTypes(attr01[12].specification().typeName(), attr01[12].data<float>(), attr02[12].data<float>());
      compareDataTypes(attr01[13].specification().typeName(), attr01[13].data<double>(), attr02[13].data<double>());
      compareDataTypes(attr01[14].specification().typeName(), attr01[14].data<long double>(), attr02[14].data<long double>());
      compareDataTypes(attr01[15].specification().typeName(), attr01[15].data<std::string>(), attr02[15].data<std::string>());
      compareDataTypes(attr01[16].specification().typeName(), attr01[16].data<coral::Blob>(), attr02[16].data<coral::Blob>());
      compareDataTypes(attr01[17].specification().typeName(), attr01[17].data<coral::Date>(), attr02[17].data<coral::Date>());
      compareDataTypes(attr01[18].specification().typeName(), attr01[18].data<coral::TimeStamp>(), attr02[18].data<coral::TimeStamp>());

      //next test without a preset buffer
      swi.appendV(attr01);

      swi.flush();

      coral::AttributeList attr03;

      sri.extractV(attr03);

      //compare both attribute lists

      size_t atrs02 = attr01.size();

      if(attr03.size() != atrs02)
        CPPUNIT_FAIL("AttributeList lengths are different");

      for(size_t i = 0; i < atrs02; i++ )
      {
        Attribute& attr_01 = attr01[i];
        Attribute& attr_02 = attr03[i];

        //compare the both attributes
        const AttributeSpecification& spec01 = attr_01.specification();
        const AttributeSpecification& spec02 = attr_02.specification();

        if(spec01.name().compare(spec02.name()) != 0)
          CPPUNIT_FAIL("AttributeList name is not equal");

        if(spec01.typeName().compare(spec02.typeName()) != 0)
          CPPUNIT_FAIL("AttributeList typename is not equal");

        if(spec01.type() != spec02.type())
          CPPUNIT_FAIL("AttributeList type is not equal");

        //compare the data


      }

      swi.appendE(attr01);

      swi.flush();

      coral::AttributeList attr04;

      sri.extractE(attr04);

      coral::CoralStubs::checkAttributeLists(attr04, attr01);

    }

    void
    test_SISimpleTypes()
    {
      test_SISimpleTypesBuffer(CALPACKET_HEADER_SIZE + 4 + 8);
      //      test_SISimpleTypesBuffer(99);
      //      test_SISimpleTypesBuffer(245);
    }

    void
    test_SIComplexTypes()
    {
      test_SIComplexTypesBuffer(CALPACKET_HEADER_SIZE + 4 + 8);

    }

    void
    test_SICoralTypes()
    {
      test_SICoralTypesBuffer(CALPACKET_HEADER_SIZE + 4 + 8);
      test_SICoralTypesBuffer(5000);

    }

    void
    test_Opcodes()
    {
      coral::CoralStubs::SegmentWriterIterator swi(coral::CALOpcodes::ConnectRO, true, true, 1024);
      coral::CoralStubs::DummyByteBufferIterator dbi;
      coral::CoralStubs::SegmentReaderIterator sri(coral::CALOpcodes::ConnectRO, dbi);

      swi.append16(std::string("Hello world"));

      swi.flush();
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      std::string s;

      sri.extract16(s);

      swi.exception(0x00, 0x02, "Test", "test_Opcodes()", "Tests");
      while(swi.next()) dbi.addBuffer(swi.currentBuffer());

      try
      {
        sri.extract16(s);
      }
      catch ( coral::Exception& )
      {
      }
      catch (...)
      {
        throw;
      }

    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( StreamBufferTest );

}

CORALCPPUNITTEST_MAIN( StreamBufferTest )
