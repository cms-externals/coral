#include "UserTests.h"
#include "TestEnv/TestingConfig.h"

#include <iostream>
#include <stdexcept>
#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("UTESTS"); myMsg << coral::Debug << msg << coral::MessageStream::endmsg; }

TestUserTests::TestUserTests( const TestEnv& env )
  : Testing(env)
{
}

//-----------------------------------------------------------------------------

TestUserTests::~TestUserTests()
{
}

//-----------------------------------------------------------------------------

void
TestUserTests::test01()
{
  coral::ISessionProxy& session = getSession();

  std::string time0 = "2009-08-26 05:00:00";
  std::string time1 = "2009-08-26 07:00:00";

  std::string dbSchema = "ATLAS_PVSSSCT";
  std::string SetName = "ATLSCT_Air_Temperature_Barrel_3";
  // get the nominal schema
  /*  coral::ISchema& schema = session->nominalSchema();
  // get the name
  std::string dbSchema = schema.schemaName();
  */
  coral::AttributeList a_list;
  coral::AttributeList a_list1;

  session.transaction().start();

  LOG("session started");

  coral::IQuery* query = session.schema(dbSchema).newQuery();

  coral::IQueryDefinition& subquery = query->defineSubQuery( "hist" );

  LOG("define subquery #01");

  subquery.addToTableList( "EVENTHISTORY_00000008", "t0" );
  subquery.addToOutputList( "t0.ELEMENT_ID" );
  subquery.addToOutputList( "(FIRST_VALUE (t0.TS) OVER ( PARTITION BY t0.ELEMENT_ID ORDER BY t0.TS DESC )) last_ts" );
  subquery.addToOutputList( "(FIRST_VALUE (t0.VALUE_NUMBER) OVER ( PARTITION BY t0.ELEMENT_ID ORDER BY t0.TS DESC )) last_val" );
  subquery.addToOutputList( "(FIRST_VALUE (t0.STATUS) OVER ( PARTITION BY t0.ELEMENT_ID ORDER BY t0.TS DESC )) last_status" );

  LOG("set data subquery #01");

  a_list.extend("fromTime", typeid(std::string));
  a_list["fromTime"].data<std::string>() = time0;
  a_list.extend("toTime", typeid(std::string));
  a_list["toTime"].data<std::string>() = time1;

  LOG("done subquery #01");

  std::string cond;

  cond = "t0.TS between to_date(:fromTime, 'YYYY-MM-DD HH24:MI:SS') and to_date(:toTime, 'YYYY-MM-DD HH24:MI:SS')";

  subquery.setCondition( cond, a_list);
  query->addToTableList("DBE_PVSSDATASETS", "conf" );
  query->addToTableList("hist" );
  query->addToOutputList("conf.ORDER_NUM" );
  query->addToOutputList("hist.last_ts" );
  query->addToOutputList("hist.last_val" );
  query->addToOutputList("hist.last_status" );
  query->addToOutputList("conf.SET_NAME" );

  a_list1.extend("schema", typeid(std::string));
  a_list1["schema"].data<std::string>() = dbSchema;
  a_list1.extend("setname", typeid(std::string));
  a_list1["setname"].data<std::string>() = SetName;

  cond = "hist.ELEMENT_ID = conf.DPE_ID and conf.PVSS_SCHEMA = :schema and conf.SET_NAME = :setname";

  query->setCondition( cond, a_list1);
  query->addToOrderList("conf.SET_NAME");
  query->addToOrderList("conf.ORDER_NUM");
  query->setRowCacheSize( 1000 );

  coral::ICursor& cursor = query->execute();
  cursor.close();

  session.transaction().commit();
  delete query;
}

//-----------------------------------------------------------------------------
