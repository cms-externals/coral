// Include files
#include <sstream>
#include <iostream>
#include "CoralBase/Exception.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/Blob.h"


#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/IRequestHandler.h"
#include "CoralServerBase/CALPacketHeader.h"
#include "CoralServerBase/CALOpcode.h"
#include "CoralServerBase/QueryDefinition.h"

#include "CoralMonitor/StatsStorage.h"

#include "CoralStubs/ServerStub.h"
#include "CoralStubs/ClientStub.h"
#include "../../src/DummyFacade.h"
#include "../../src/Exceptions.h"
#include "CoralServerBase/ICoralFacade.h"

#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"


#include "RelationalAccess/TableDescription.h"

#include "../../src/RowIteratorAll.h"
#include "../../src/RowIteratorFetch.h"

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"

#include <stdio.h>
#include <pthread.h>

namespace coral
{

  class AllPartsTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( AllPartsTest );

    CPPUNIT_TEST( test_Basic );
    CPPUNIT_TEST( test_FetchRowsWithoutBuffer );
    CPPUNIT_TEST( test_FetchRowsWithBuffer );
    CPPUNIT_TEST( test_FetchAllRowsWithoutBuffer );
    CPPUNIT_TEST( test_FetchAllRowsWithBuffer );
    CPPUNIT_TEST( test_ExceptionRAL );
    CPPUNIT_TEST( test_MultiThreadCursor );
    CPPUNIT_TEST( test_statistics );
    CPPUNIT_TEST_SUITE_END();

  public:

    void
    test_Basic()
    {
      //create a test dummy facade
      CoralStubs::DummyFacade df;
      //create a request handler
      CoralStubs::ServerStub sstub(df);
      //create a coral facade
      CoralStubs::ClientStub cstub(sstub);

      bool fromProxy;
      Token session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

      cstub.startTransaction(session, true);

      cstub.commitTransaction(session);

      cstub.releaseSession(session);
    }

    void
    test_FetchRowsWithoutBuffer()
    {
      //create a test dummy facade
      CoralStubs::DummyFacade df;
      //create a request handler
      CoralStubs::ServerStub sstub(df);
      //create a coral facade
      CoralStubs::ClientStub cstub(sstub);
      //connect
      bool fromProxy;
      Token session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

      cstub.startTransaction(session, true);

      QueryDefinition qd;

      qd.addToOrderList( "Col01" );

      qd.addToTableList( "Table01", "T1" );

      qd.limitReturnedRows( 5, 32342 );

      std::cout << std::endl << "Test without RowBuffer at 10 rows per request" << std::endl;

      IRowIteratorPtr rowa = cstub.fetchRows( session, qd, NULL, 10, false );
      IRowIterator* ia = rowa.get();
      size_t counter = 0;
      while(ia->next()) {
        counter++;
        const coral::AttributeList& alh = ia->currentRow();

        alh.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #1");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      std::cout << std::endl << "Test without RowBuffer at 2 rows per request" << std::endl;
      rowa = cstub.fetchRows( session, qd, NULL, 2, false );

      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        const coral::AttributeList& alh = ia->currentRow();

        alh.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #2");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      std::cout << std::endl << "Test without RowBuffer at 10MB per request" << std::endl;

      rowa = cstub.fetchRows( session, qd, NULL, 10, true );
      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        const coral::AttributeList& alh = ia->currentRow();

        alh.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #3");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      std::cout << std::endl << "Test without RowBuffer at 2MB per request" << std::endl;
      rowa = cstub.fetchRows( session, qd, NULL, 2, true );

      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        const coral::AttributeList& alh = ia->currentRow();

        alh.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #4");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;


      cstub.commitTransaction(session);

      cstub.releaseSession(session);
    }

    void
    test_FetchRowsWithBuffer()
    {
      //create a test dummy facade
      CoralStubs::DummyFacade df;
      //create a request handler
      CoralStubs::ServerStub sstub(df);
      //create a coral facade
      CoralStubs::ClientStub cstub(sstub);
      //connect
      bool fromProxy;
      Token session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

      cstub.startTransaction(session, true);

      QueryDefinition qd;

      qd.addToOrderList( "Col01" );

      qd.addToTableList( "Table01", "T1" );

      qd.limitReturnedRows( 5, 32342 );


      AttributeList al01;


      al01.extend("A", typeid(coral::TimeStamp));
      al01.extend("B", typeid(double));
      al01.extend("C1", typeid(std::string));
      al01.extend("C2", typeid(std::string));
      al01.extend("D", typeid(int));
      al01.extend("E", typeid(coral::Blob));

      /*
      al01[0].data<std::string>() = "hello world";
      al01[1].data<int>() = -45;
      */

      //use maxsize 10 rows here
      std::cout << std::endl << "Test with RowBuffer at 10 rows per request, output via currentrow" << std::endl;

      IRowIteratorPtr rowa = cstub.fetchRows( session, qd, &al01, 10, false );
      IRowIterator* ia = rowa.get();
      size_t counter = 0;
      while(ia->next()) {
        counter++;
        const coral::AttributeList& alh = ia->currentRow();
        if(alh.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        if(&alh != &al01)
          CPPUNIT_FAIL("AttributeBuffer missmatch");

        alh.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #1");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      std::cout << std::endl << "Test with RowBuffer at 10 rows per request, output via rowbuffer" << std::endl;

      rowa = cstub.fetchRows( session, qd, &al01, 10, false );

      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        if(al01.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        al01.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #2");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      //use 2 rows
      std::cout << "Test with RowBuffer at 2 rows per request, output via currentrow" << std::endl;

      rowa = cstub.fetchRows( session, qd, &al01, 2, false );

      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        const coral::AttributeList& alh = ia->currentRow();
        if(alh.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        if(&alh != &al01)
          CPPUNIT_FAIL("AttributeBuffer missmatch");

        alh.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #3");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      std::cout << "Test with RowBuffer at 2 rows per request, output via rowbuffer" << std::endl;

      rowa = cstub.fetchRows( session, qd, &al01, 2, false );

      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        if(al01.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        al01.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #4");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      std::cout << "Test with RowBuffer at 10MB per request, output via rowbuffer" << std::endl;

      rowa = cstub.fetchRows( session, qd, &al01, 10, true );

      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        if(al01.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        al01.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #5");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;


      std::cout << "Test with RowBuffer at 2MB per request, output via rowbuffer" << std::endl;

      rowa = cstub.fetchRows( session, qd, &al01, 2, true );

      ia = rowa.get();
      counter = 0;
      while(ia->next()) {
        counter++;
        if(al01.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        al01.toOutputStream(std::cout) << std::endl;
      }
      if(counter != 5)
        CPPUNIT_FAIL("Fetched wrong amount of rows #6");

      std::cout << "Number of requests " << dynamic_cast<coral::CoralStubs::RowIteratorFetch*>(ia)->getNumberOfRequests() << std::endl;

      cstub.commitTransaction(session);

      cstub.releaseSession(session);
    }

    void
    test_FetchAllRowsWithoutBuffer()
    {
      //create a test dummy facade
      CoralStubs::DummyFacade df;
      //create a request handler
      CoralStubs::ServerStub sstub(df);
      //create a coral facade
      CoralStubs::ClientStub cstub(sstub);
      //connect
      bool fromProxy;
      Token session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

      cstub.startTransaction(session, true);

      QueryDefinition qd;

      qd.addToOrderList( "Col01" );

      qd.addToTableList( "Table01", "T1" );

      qd.limitReturnedRows( 5, 32342 );


      IRowIteratorPtr rowa = cstub.fetchAllRows( session, qd, NULL );

      size_t counter = 0;

      IRowIterator* ia = rowa.get();
      while(ia->next()) {
        const coral::AttributeList& alh = ia->currentRow();
        if(alh.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        alh.toOutputStream(std::cout) << std::endl;

        counter++;
      }
      if(counter != 30)
        CPPUNIT_FAIL("Fetched wrong amount of rows #0");

      cstub.commitTransaction(session);

      cstub.releaseSession(session);

    }

    void
    test_FetchAllRowsWithBuffer()
    {
      //create a test dummy facade
      CoralStubs::DummyFacade df;
      //create a request handler
      CoralStubs::ServerStub sstub(df);
      //create a coral facade
      CoralStubs::ClientStub cstub(sstub);
      //connect
      bool fromProxy;
      Token session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

      cstub.startTransaction(session, true);

      QueryDefinition qd;

      qd.addToOrderList( "Col01" );

      qd.addToTableList( "Table01", "T1" );

      qd.limitReturnedRows( 5, 32342 );

      AttributeList al01;


      IRowIteratorPtr rowa = cstub.fetchAllRows( session, qd, &al01 );

      IRowIterator* ia = rowa.get();
      while(ia->next()) {
        const coral::AttributeList& alh = ia->currentRow();
        if(alh.size() == 0)
          CPPUNIT_FAIL("AttributeList size is zero");

        if(&alh != &al01)
          CPPUNIT_FAIL("AttributeBuffer missmatch");

        alh.toOutputStream(std::cout) << std::endl;
      }

      cstub.commitTransaction(session);

      cstub.releaseSession(session);
    }

    void
    test_ExceptionRAL()
    {
      //create a test dummy facade
      CoralStubs::DummyFacade df;
      //create a request handler
      CoralStubs::ServerStub sstub(df);
      //create a coral facade
      CoralStubs::ClientStub cstub(sstub);
      //connect
      bool fromProxy;
      Token session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

      try
      {
        cstub.startTransaction(session, false);
        CPPUNIT_FAIL("Exception was not catched");
      }
      catch ( coral::Exception& )
      {
      }
      cstub.releaseSession(session);

    }

    static void*
    test_MT_01(void* f)
    {
      CoralStubs::ClientStub& cstub = *((CoralStubs::ClientStub*)f);

      bool fromProxy;
      Token session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

      cstub.startTransaction(session, true);

      QueryDefinition qd;

      qd.addToOrderList( "Col01" );

      qd.addToTableList( "Table01", "T1" );

      qd.limitReturnedRows( 5, 32342 );

      AttributeList al01;


      IRowIteratorPtr rowa = cstub.fetchAllRows( session, qd, &al01 );

      //     IRowIteratorPtr rowa = cstub.fetchRows( session, qd, NULL, 10, false );

      IRowIterator* ia = rowa.get();
      while(ia->next()) {
        const coral::AttributeList& alh = ia->currentRow();

        alh.toOutputStream(std::cout) << std::endl;
      }


      cstub.commitTransaction(session);

      cstub.releaseSession(session);

      return NULL;
    }

    void
    test_MultiThreadCursor()
    {
      //create a test dummy facade
      CoralStubs::DummyFacade df;
      //create a request handler
      CoralStubs::ServerStub sstub(df);
      //create a coral facade
      CoralStubs::ClientStub cstub(sstub);
      //connect

      //create 10 threads doing something

      pthread_t id[11];
      pthread_attr_t attr;

      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

      for( size_t i = 1; i < 10; i++ ) {
        pthread_create(&id[i], &attr, test_MT_01, (void*)&cstub);
      }

      void *status;
      for( size_t i = 1; i < 10; i++ ) {
        pthread_join(id[i], &status);
      }

    }

    void
    test_statistics()
    {
      std::vector<std::string> list;
      coral::StatsStorageInstance().report(list);

      std::cout << std::endl;

      for( std::vector<std::string>::iterator i = list.begin(); i != list.end(); ++i )
        std::cout << *i << std::endl;
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( AllPartsTest );

}

CORALCPPUNITTEST_MAIN( AllPartsTest )

/**

int main( int argc, char* argv[] ){
coral::CoralStubs::DummyFacade df;
//create a request handler
coral::CoralStubs::ServerStub sstub(df);
//create a coral facade
coral::CoralStubs::ClientStub cstub(sstub);


coral::Token session = 0;
//   try{

bool fromProxy;

session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);
//  }
//   catch(...)
//   {
//     return 1;
//  }
cstub.startTransaction(session, true);

coral::QueryDefinition qd;

qd.addToOrderList( "Col01" );

qd.addToTableList( "Table01", "T1" );

qd.limitReturnedRows( 5, 32342 );


coral::AttributeList al01;
coral::AttributeList al02;

al01.extend("Column01", typeid(std::string) );
al01.extend("Column02", typeid(int) );

al01[0].data<std::string>() = "hello world";
al01[1].data<int>() = -45;

coral::IRowIteratorPtr rowa = cstub.fetchRows( session, qd, &al01, 10, true );

coral::IRowIterator* ia = rowa.get();
while(ia->next()){
const coral::AttributeList& alh = ia->currentRow();

if(&alh != &al01){
std::cout << "AttributeList missmatch !!" << std::endl;
}

alh.toOutputStream(std::cout) << std::endl;
}


al02.extend("Column03", typeid(float) );

al02[0].data<float>() = 0.3423;

coral::IRowIteratorPtr rowi = cstub.fetchAllRows( session, qd, &al02 );

coral::IRowIterator* ii = rowi.get();

size_t i = 0;
while(ii->next()){
i++;
const coral::AttributeList& alh = ii->currentRow();

if(&alh != &al02){
std::cout << "AttributeList missmatch !!" << std::endl;
}else{
al02.toOutputStream(std::cout) << std::endl;
}

alh.toOutputStream(std::cout) << std::endl;
}

rowi = cstub.fetchAllRows( session, qd, NULL );

ii = rowi.get();
while(ii->next()){
i++;
const coral::AttributeList& alh = ii->currentRow();

if(&alh != &al02){
std::cout << "AttributeList missmatch !!" << std::endl;
}else{
al02.toOutputStream(std::cout) << std::endl;
}

alh.toOutputStream(std::cout) << std::endl;
}


const std::set<std::string> myset = cstub.listTables(session, "MySchema");
std::set<std::string>::const_iterator j;
for(j = myset.begin(); j != myset.end(); j++ )
{
std::cout << "listTables : " << *j << std::endl;
}

const std::vector<std::string> sp = cstub.sessionProperties( session );

const coral::TableDescription tdesc = cstub.tableDescription(session, "MySchema", "MyGreatTable" );

const coral::TableDescriptionMap tdescm = cstub.tableDescriptionMap(session, "MySchema" );

cstub.commitTransaction(session);

cstub.releaseSession(session);

//    coral::ByteBuffer requestBuffer(30000);



//    coral::CALPacketHeader header_request(coral::CALOpcodes::ConnectRO, false, 0);
//    memcpy(requestBuffer.data(), header_request.data(), coral::CALPACKET_HEADER_SIZE);

//    coral::ByteBuffer& replyBuffer = replyBufferPool.allocateBuffer();

//    coral::IReplyIteratorPtr reply = rh.replyToRequest( requestBuffer, replyBufferPool );

session = cstub.connect("http://hello_world", coral::ReadOnly, fromProxy);

cstub.startTransaction(session, false);

cstub.commitTransaction(session);

cstub.releaseSession(session);

}
*/
