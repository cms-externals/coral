#include <iostream>
#include <string>
#include "CoralKernel/Context.h"
#include "CoralKernel/IHandle.h"

#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IDatabaseServiceSet.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/SchemaException.h"

#include "RelationalAccess/../src/ISequence.h" // temporary
#include "RelationalAccess/../src/SequenceDescription.h"

#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"

#define LOG( msg ){ coral::MessageStream myMsg("SEQUENCE"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

namespace coral
{
  class SequenceTest;
}

//----------------------------------------------------------------------------

class coral::SequenceTest : public coral::CoralCppUnitDBTest
{
  CPPUNIT_TEST_SUITE( SequenceTest );
  CPPUNIT_TEST( test01 );
  CPPUNIT_TEST( test02 );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test01()
  {
#ifdef CORAL240SQ
    std::string T1 = BuildUniqueTableName( "SEQUENCE_T1" );

    std::unique_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );

    LOG(" **** TestSequence: TEST01...start **** ");
    // This part tests all create and drop methods
    // Use this description
    coral::SequenceDescription desc("SEQ_" + T1);
    desc.setStartValue( 42 );

    coral::ISchema& schema = session->nominalSchema();

    session->transaction().start();

    coral::ISequence& sequence01 = schema.createSequence( desc );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Unexpected value #1", sequence01.nextValue(), (unsigned long long)42 );

    try
    {
      schema.createSequence( desc );
      // should have now one sequence with the name, should throw SchemaException
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "Sequence exists #1", schema.existsSequence( "SEQ_" + T1 ), true );
    }
    catch(coral::SchemaException& /*e*/)
    {
      // everything fine
    }

    // Sequence should exists
    schema.dropSequence("SEQ_" + T1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Sequence exists #2", schema.existsSequence( "SEQ_" + T1 ), false );

    schema.dropIfExistsSequence( "SEQ_" + T1 );

    schema.createSequence( desc );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Sequence exists #3", schema.existsSequence( "SEQ_" + T1 ), true );

    schema.dropIfExistsSequence( "SEQ_" + T1 );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Sequence exists #4", schema.existsSequence( "SEQ_" + T1 ), false );

    // Finally create the sequence
    coral::ISequence& sequence02 = schema.createSequence( desc );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Sequence next value", sequence02.nextValue(), (unsigned long long)42 );

    session->transaction().commit();

    LOG(" **** TestSequence: TEST01...done **** ");
#endif
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test02()
  {
#ifdef CORAL240SQ
    std::string T1 = BuildUniqueTableName( "SEQUENCE_T1" );

    std::unique_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );

    LOG(" **** TestSequence: TEST02...start **** ");

    coral::ISchema& schema = session->nominalSchema();

    session->transaction().start();

    coral::SequenceDescription desc("SEQ_" + T1);

    schema.createSequence( desc );

    session->transaction().commit();

    LOG(" **** TestSequence: TEST02...#2 **** ");

    session->transaction().start();

    bool found = false;

    std::set<std::string> list = schema.listSequences();
    for( std::set<std::string>::iterator i = list.begin(); i != list.end(); ++i )
    {
      if( i->compare("SEQ_" + T1) )
        found = true;
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Sequence 'SEQ_" + T1 + "' not found", found, true );

    coral::ISequence& sequence = schema.sequenceHandle( "SEQ_" + T1 );

    unsigned long long value01 = sequence.nextValue();
    unsigned long long value02 = value01;

    value01 = sequence.nextValue();

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Values #1", value01, value02 + 1 );

    sequence.nextValue();
    sequence.nextValue();
    sequence.nextValue();
    sequence.nextValue();

    value01 = sequence.nextValue();

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Values #1", value01, value02 + 6 );

    schema.dropSequence( "SEQ_" + T1 );

    session->transaction().commit();

    LOG(" **** TestSequence: TEST02...done **** ");
#endif
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  SequenceTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  ~SequenceTest(){}

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void setUp()
  {
#ifdef CORAL240SQ
    std::string T1 = BuildUniqueTableName( "SEQUENCE_T1" );

    std::unique_ptr<coral::ISessionProxy> session( connSvc().connect( UrlRW(), coral::Update ) );

    LOG(" **** TestSequence: SETUP...start **** ");
    // This setup garanties there is no sequence registered
    coral::ISchema& schema = session->nominalSchema();

    session->transaction().start();

    schema.dropIfExistsSequence( "SEQ_" + T1 );

    session->transaction().commit();
    LOG(" **** TestSequence: SETUP...done **** ");
#endif
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void tearDown(){}

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::SequenceTest );

//----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( SequenceTest )
