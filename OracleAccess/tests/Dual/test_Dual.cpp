// Include files
#include <iostream>
#include <exception>
#include <memory>
#include <stdexcept>
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Exception.h"
#include "CoralCommon/Utilities.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/TableDescription.h"

//----------------------------------------------------------------------------

namespace coral
{

  class DualTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( DualTest );
    CPPUNIT_TEST( test_Read );
    CPPUNIT_TEST( test_bug87111 );
    CPPUNIT_TEST( test_ora01466 ); // not a DUAL query, but easy to have here!
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    setUp()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_Read()
    {
      coral::ConnectionService connSvc;
      coral::ISessionProxy* session = connSvc.connect( "CORAL-Oracle-lcgnight/admin" );
      session->transaction().start( true );
      coral::ISchema& schema = session->nominalSchema();
      coral::AttributeList data;
      data.extend<float>( "X" );
      coral::IQuery* query = schema.newQuery();
      query->addToOutputList( "3.21", "X" );
      query->defineOutput( data );
      query->addToTableList( "DUAL" );
      coral::ICursor& cursor = query->execute();
      cursor.next();
      std::cout << "[OVAL] X : " << data["X"].data<float>() << std::endl;
      delete query;
      session->transaction().commit();
      delete session;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_bug87111()
    {
      coral::ConnectionService connSvc;
      coral::ISessionProxy* session = connSvc.connect( "CORAL-Oracle-lcgnight/admin" );
      session->transaction().start( true );
      coral::ISchema& schema = session->nominalSchema();
      coral::AttributeList data;
      data.extend<std::string>( "TIMESTAMP" );
      coral::IQuery* query = schema.newQuery();
      query->addToOutputList( "TO_CHAR(SYSTIMESTAMP AT TIME ZONE 'GMT','yyyy-mm-dd_hh24:mi:ss.ff6')||'000 GMT'", "TIMESTAMP" );
      query->defineOutput( data );
      query->addToTableList( "DUAL" );
      coral::ICursor& cursor = query->execute();
      cursor.next();
      std::cout  << std::endl << "TIMESTAMP : " << data["TIMESTAMP"].data<std::string>() << std::endl;
      delete query;
      session->transaction().commit();
      delete session;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    test_ora01466()
    {
      std::string T1 = BuildUniqueTableName( "ORA01466" );
      coral::ConnectionService connSvc;
      std::auto_ptr<coral::ISessionProxy> session( connSvc.connect( "CORAL-Oracle-lcgnight/admin" ) );
      session->transaction().start( false );
      session->nominalSchema().dropIfExistsTable( T1 );
      coral::TableDescription description;
      description.setName( T1 );
      description.insertColumn( "ID", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
      description.setPrimaryKey( "ID" );
      session->nominalSchema().createTable( description );
      session->transaction().commit();
      try
      {
        session->transaction().start( true );
        std::auto_ptr<coral::IQuery> query( session->nominalSchema().tableHandle(T1).newQuery() );
        query->execute(); // This may (probably will) fail with ORA-01466
        session->transaction().commit();
      }
      catch(...)
      {
        session->transaction().commit();
      }
      coral::sleepSeconds(1);
      session->transaction().start( true );
      std::auto_ptr<coral::IQuery> query( session->nominalSchema().tableHandle(T1).newQuery() );
      query->execute(); // This should no longer fail with ORA-01466
      session->transaction().commit();
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( DualTest );

}

CORALCPPUNITTEST_MAIN( DualTest )
