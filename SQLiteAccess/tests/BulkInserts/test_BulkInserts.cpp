// Include files
#include <float.h>
#include <iostream>
#include <limits.h> // fix bug #58581
#include <stdexcept>
#include <memory>
#include <sstream>

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "CoralCommon/Utilities.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

#ifdef _WIN32
#undef max //solve windows compilation
#undef min
#endif

namespace coral
{

  class BulkInsertsTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( BulkInsertsTest );
    CPPUNIT_TEST( test_Write );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void setUp()
    {
      std::string T1 = BuildUniqueTableName( "SQL_UT_BI_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

      session->transaction().start(false) ;

      coral::ISchema& schema = session->nominalSchema();

      coral::TableDescription desc;
      desc.setName( T1 );
      desc.insertColumn( "myfloat", coral::AttributeSpecification::typeNameForId(typeid(float)) );
      desc.insertColumn( "mydouble", coral::AttributeSpecification::typeNameForId(typeid(double)) );
      desc.insertColumn( "mylongdouble", coral::AttributeSpecification::typeNameForId(typeid(long double)) );
      desc.insertColumn( "myint", coral::AttributeSpecification::typeNameForId(typeid(int)) );
      desc.insertColumn( "myunsignedint", coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) );
      desc.insertColumn( "myshort", coral::AttributeSpecification::typeNameForId(typeid(short)) );
      desc.insertColumn( "myunsignedshort", coral::AttributeSpecification::typeNameForId(typeid(unsigned short)) );
      /* disabled, see bug #82198
      desc.insertColumn( "mylong", coral::AttributeSpecification::typeNameForId(typeid(long)) );
      desc.insertColumn( "myunsignedlong", coral::AttributeSpecification::typeNameForId(typeid(unsigned long)) );
      */
      desc.insertColumn( "mysleast", coral::AttributeSpecification::typeNameForId(typeid(long long int)) );
      desc.insertColumn( "myuleast", coral::AttributeSpecification::typeNameForId(typeid(unsigned long long int)) );
      desc.insertColumn( "mybool", coral::AttributeSpecification::typeNameForId(typeid(bool)) );
      desc.insertColumn( "mychar", coral::AttributeSpecification::typeNameForId(typeid(char)) );
      desc.insertColumn( "myuschar", coral::AttributeSpecification::typeNameForId(typeid(unsigned char)) );
      desc.insertColumn( "mystring", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
      desc.setNotNullConstraint( "mystring" );

      schema.dropIfExistsTable(T1);

      schema.createTable(desc);

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void test_Write()
    {
      std::string T1 = BuildUniqueTableName( "SQL_UT_BI_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

      session->transaction().start(false) ;

      coral::ISchema& workingSchema = session->nominalSchema();

      coral::ITable& table = workingSchema.tableHandle(T1);

      // Costructing a row buffer.
      coral::AttributeList data;

      std::cout<<"about to construct row buf"<<std::endl;

      table.dataEditor().rowBuffer(data);
      // Adding new rows
      std::cout << "Inserting new rows into the table." << std::endl;
      coral::IBulkOperation* rowInserter = table.dataEditor().bulkInsert(data,3);
      for ( int i = 0; i < 5; ++i )
      {
        std::ostringstream os;
        os << i;
        const std::string& stri = os.str();
        ///some harmless dummy values
        data["myfloat"].data<float>()= 37.623F ;
#ifndef __ICC
        data["mydouble"].data<double>()=2.2e-308 ;
#else
        data["mydouble"].data<double>()=2.3e-308 ;
#endif
        data["mylongdouble"].data<long double>()= -1897.23;
        data["myint"].data<int>()= -41928 ;  //s32
        data["myunsignedint"].data<unsigned int>()=4294967295U;  //u32
        data["myshort"].data<short>()=100 ;  //s16
        data["myunsignedshort"].data<unsigned short>()=65535; //u16
        /* disabled, see bug #82198
        data["mylong"].data<long>()=814748647L;  //s32
        data["myunsignedlong"].data<unsigned long>()= 4294967295U ; //u32
        */
        data["mysleast"].data<long long int>()= 9223372036854775807LL;  //s64
        data["myuleast"].data<unsigned long long int>()= 18446744073709551615ULL;  //u64
        data["mybool"].data<bool>()= true ;
        data["mychar"].data<char>()= -1 ;
        data["myuschar"].data<unsigned char>()= 'c' ;
        data["mystring"].data<std::string>()=std::string("c")+stri ;
        ///lower limits
        if(i==1)
        {
          data["myfloat"].data<float>()=std::numeric_limits<float>::min();
          data["mydouble"].data<double>()=std::numeric_limits<double>::min();
          data["mylongdouble"].data<long double>()=std::numeric_limits<long double>::min();
          data["myint"].data<int>()=std::numeric_limits<int>::min();
          data["myunsignedint"].data<unsigned int>()=0;
          data["myshort"].data<short>()= std::numeric_limits<short>::min();
          data["myunsignedshort"].data<unsigned short>()=0;
          /* disabled, see bug #82198
          data["mylong"].data<long>()=std::numeric_limits<long>::min();
          data["myunsignedlong"].data<unsigned long>()=0;
          */
          data["mysleast"].data<long long int>()=std::numeric_limits<long long int>::min();
          data["myuleast"].data<unsigned long long int>()=0;
          data["mybool"].data<bool>()= false;
          data["mychar"].data<char>()=std::numeric_limits<char>::min();
          data["myuschar"].data<unsigned char>()=0;
        }
        ///upper limits
        if(i==2)
        {
          data["myfloat"].data<float>()=std::numeric_limits<float>::max();
          data["mydouble"].data<double>()=std::numeric_limits<double>::max();
          data["mylongdouble"].data<long double>()=std::numeric_limits<long double>::max();
          data["myint"].data<int>()=std::numeric_limits<int>::max();
          data["myunsignedint"].data<unsigned int>()=std::numeric_limits<unsigned>::max();
          data["myshort"].data<short>()= std::numeric_limits<short>::max();
          data["myunsignedshort"].data<unsigned short>()=std::numeric_limits<unsigned short>::max();
          /* disabled, see bug #82198
          data["mylong"].data<long>()=std::numeric_limits<long>::max();
          data["myunsignedlong"].data<unsigned long>()=std::numeric_limits<unsigned long>::max();
          */
          data["mysleast"].data<long long int>()=std::numeric_limits<long long int>::max();
          data["myuleast"].data<unsigned long long int>()=std::numeric_limits<unsigned long long int>::max();
          data["mybool"].data<bool>()= false;
          data["mychar"].data<char>()=std::numeric_limits<char>::max();
          data["myuschar"].data<unsigned char>()=std::numeric_limits<unsigned char>::max();
        }
        rowInserter->processNextIteration();
      }
      rowInserter->flush();
      delete rowInserter;

      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void test_Read()
    {
      std::string T1 = BuildUniqueTableName( "SQL_UT_BI_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

      session->transaction().start(true) ;

      coral::ISchema& workingSchema = session->nominalSchema();

      coral::ITable& table = workingSchema.tableHandle(T1);

      coral::IQuery* query = table.newQuery();
      query->setRowCacheSize( 10 );
      coral::ICursor& cursor = query->execute();
      int counter=0;
      while( cursor.next() )
      {
        std::cout << "counter : " << counter << std::endl;

        const coral::AttributeList& row = cursor.currentRow();
        if( counter==0 )
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myf", row["myfloat"].data<float>(), 37.623f );
#ifndef __ICC
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mydl", row["mydouble"].data<double>(), 2.2e-308 );
#else
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mydl", row["mydouble"].data<double>(), 2.3e-308 );
#endif
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mylongdl", row["mylongdouble"].data<long double>(), (long double)-1897.23 );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myi", row["myint"].data<int>(), -41928 );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myui", row["myunsignedint"].data<unsigned int>(), 4294967295U );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mysll", row["mysleast"].data<long long int>(), 9223372036854775807LL );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myull", row["myuleast"].data<unsigned long long int>(), 18446744073709551615ULL );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mybool", row["mybool"].data<bool>(), true );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mychar", row["mychar"].data<char>(), (char)-1 );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myuchar", row["myuschar"].data<unsigned char>(), (unsigned char)'c' );
        }
        if( counter==1 )
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myf", row["myfloat"].data<float>(), FLT_MIN );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mydl", row["mydouble"].data<double>(), DBL_MIN );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mylongdl", row["mylongdouble"].data<long double>(), (long double)DBL_MIN );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myi", row["myint"].data<int>(), INT_MIN );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myui", row["myunsignedint"].data<unsigned int>(), (unsigned int)0 );

          CPPUNIT_ASSERT_EQUAL_MESSAGE("myshort", row["myshort"].data<short>(), (short)SHRT_MIN );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myushort", row["myunsignedshort"].data<unsigned short>(), (unsigned short)0 );
          /* disabled, see bug #82198
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myl", row["mylong"].data<long>(), (long)LONG_MIN );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myul", row["myunsignedlong"].data<unsigned long>(), (unsigned long)0 );
          */
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mysll", row["mysleast"].data<long long int>(), std::numeric_limits<long long>::min() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myull", row["myuleast"].data<unsigned long long int>(), (unsigned long long int)0 );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mybool", row["mybool"].data<bool>(), false );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mychar", row["mychar"].data<char>(), (char)CHAR_MIN );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myuchar", row["myuschar"].data<unsigned char>(), (unsigned char)0 );
        }
        if( counter==2 )
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myf", row["myfloat"].data<float>(), FLT_MAX );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mydl", row["mydouble"].data<double>(), DBL_MAX );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mylongdl", row["mylongdouble"].data<long double>(), (long double)DBL_MAX );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myi", row["myint"].data<int>(), INT_MAX );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myui", row["myunsignedint"].data<unsigned int>(), (unsigned int)UINT_MAX );

          CPPUNIT_ASSERT_EQUAL_MESSAGE("myshort", row["myshort"].data<short>(), (short)SHRT_MAX );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myushort", row["myunsignedshort"].data<unsigned short>(), (unsigned short)USHRT_MAX );
          /* disabled, see bug #82198
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myl", row["mylong"].data<long>(), (long)LONG_MAX );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myul", row["myunsignedlong"].data<unsigned long>(), (unsigned long)ULONG_MAX );
          */
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mysll", row["mysleast"].data<long long int>(), std::numeric_limits<long long>::max() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myull", row["myuleast"].data<unsigned long long int>(), (unsigned long long int)std::numeric_limits<unsigned long long>::max() );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mybool", row["mybool"].data<bool>(), false );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("mychar", row["mychar"].data<char>(), (char)CHAR_MAX );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("myuchar", row["myuschar"].data<unsigned char>(), (unsigned char)UCHAR_MAX );
        }
        ++counter;
      }
      cursor.close();
      // Committing the transaction
      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void tearDown()
    {
      std::string T1 = BuildUniqueTableName( "SQL_UT_BI_T1" );

      coral::ConnectionService connSvc;

      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-SQLite-lcgnight/admin" ) );

      session->transaction().start(false) ;

      coral::ISchema& schema = session->nominalSchema();

      schema.dropTable(T1);

      session->transaction().commit();
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( BulkInsertsTest );

}

CORALCPPUNITTEST_MAIN( BulkInsertsTest )
