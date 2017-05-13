#include "CoralBase/Exception.h"

#include "TestEnv/TestEnv.h"
#include "TestEnv/TestingConfig.h"
#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"

#include "CoralKernel/IHandle.h"
#include "CoralKernel/Context.h"


#include <stdio.h>
#include <string>
#include <iostream>

#define LOG( msg ){ coral::MessageStream myMsg("UTESTS"); myMsg << coral::Debug << msg << coral::MessageStream::endmsg; }

int main(int, char **)
{
  coral::Context& context = coral::Context::instance();

  coral::IHandle<coral::IConnectionService> handle;

  handle = context.query<coral::IConnectionService>( "CORAL/Services/ConnectionService" );
  if( !handle.isValid() )
    context.loadComponent( "CORAL/Services/ConnectionService" );

  handle = context.query<coral::IConnectionService>( "CORAL/Services/ConnectionService" );

  if( !handle.isValid() ) throw;

  coral::ISessionProxy* session = handle.get()->connect( "oracle://intr/atlas_pvss_reader", coral::ReadOnly );


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

  session->transaction().start(true);

  LOG("session started");

  coral::IQuery* query = session->schema(dbSchema).newQuery();

  coral::IQueryDefinition& subquery = query->defineSubQuery( "HIST" );

  LOG("define subquery #01");

  subquery.addToTableList( "EVENTHISTORY_00000008", "t0" );
  subquery.setDistinct();
  subquery.addToOutputList( "t0.ELEMENT_ID" );
  subquery.addToOutputList( "(FIRST_VALUE (t0.TS) OVER ( PARTITION BY t0.ELEMENT_ID ORDER BY t0.TS DESC )) LAST_TS" );
  subquery.addToOutputList( "(FIRST_VALUE (t0.VALUE_NUMBER) OVER ( PARTITION BY t0.ELEMENT_ID ORDER BY t0.TS DESC )) LAST_VAL" );
  subquery.addToOutputList( "(FIRST_VALUE (t0.STATUS) OVER ( PARTITION BY t0.ELEMENT_ID ORDER BY t0.TS DESC )) LAST_STATUS" );

  LOG("set data subquery #01");

  a_list.extend("fromTime", typeid(std::string));
  a_list["fromTime"].data<std::string>() = time0;
  a_list.extend("toTime", typeid(std::string));
  a_list["toTime"].data<std::string>() = time1;

  LOG("done subquery #01");

  std::string cond;

  cond = "t0.TS between to_date(:fromTime, 'YYYY-MM-DD HH24:MI:SS') and to_date(:toTime, 'YYYY-MM-DD HH24:MI:SS')";

  subquery.setCondition( cond, a_list);
  query->addToTableList("SYN_DBE_PVSSDATASETS", "CONF" );
  query->addToTableList("HIST" );
  query->addToOutputList("CONF.ORDER_NUM" );
  query->addToOutputList("HIST.LAST_TS" );
  query->addToOutputList("HIST.LAST_VAL" );
  query->addToOutputList("HIST.LAST_STATUS" );
  query->addToOutputList("CONF.SET_NAME" );

  a_list1.extend("schema", typeid(std::string));
  a_list1["schema"].data<std::string>() = dbSchema;
  a_list1.extend("setname", typeid(std::string));
  a_list1["setname"].data<std::string>() = SetName;

  cond = "HIST.ELEMENT_ID = CONF.DPE_ID and CONF.PVSS_SCHEMA = :schema and CONF.SET_NAME = :setname";

  query->setCondition( cond, a_list1);
  query->addToOrderList("CONF.SET_NAME");
  query->addToOrderList("CONF.ORDER_NUM");
  query->setRowCacheSize( 1000 );

  LOG("execute query");


  coral::ICursor& cursor = query->execute();

  size_t counter = 0;

  while( cursor.next() )
  {
    counter++;
    LOG("data id:" << counter);
  }

  cursor.close();

  session->transaction().commit();
  delete query;

}
