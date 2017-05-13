#include "TestApp.h"
#include <stdexcept>
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITransaction.h"
#include "CoralBase/Exception.h"
#include "CoralBase/AttributeSpecification.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "CoralCommon/Utilities.h"
#include <iostream>

const std::string TestApp::OVALTAG = "[OVAL]";

TestApp::TestApp(const char * testName) : TestEnv(testName)
{
  addTablePrefix(T1, "T1");
}


TestApp::~TestApp()
{
}

//
void TestApp::setupDataSource( const std::string& connectionString, const std::string& schemaName){
  coral::IConnectionService& theConnectionService = sw.getConnectionService();

  coral::ISessionProxy* session = theConnectionService.connect( connectionString );
  session->transaction().start();
  // creates a dummy table to be looked up
  //std::cout << schemaName <<std::endl;
  ///error *** bug
  //here by mysql a segmenatation fault happens when the schema is not found
  session->schema(schemaName).dropIfExistsTable(T1);
  coral::TableDescription descr;
  descr.setName(T1);
  descr.insertColumn("N_X",coral::AttributeSpecification::typeNameForType<int>());
  descr.insertColumn("N_S",coral::AttributeSpecification::typeNameForType<std::string>());
  session->schema(schemaName).createTable( descr ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );;
  session->transaction().commit();
  delete session;

  // Sleep for 1 second
  coral::sleepSeconds( 1 );
}

void
TestApp::run(){
  coral::IConnectionService& theConnectionService = sw.getConnectionService();

  theConnectionService.configuration().setConnectionTimeOut(0);

  std::vector<std::string> connstrings = sw.getAllReplicas(getServiceName(0), coral::Update, 1);
  std::string connString(connstrings[0]);
  //overides the given schema from the service name dblookup
  setupDataSource(getServiceName(0), connString.substr(connString.find_last_of('/') + 1));

  theConnectionService.purgeConnectionPool();

  try {

    std::string connectionString = getServiceName(2);
    coral::ISessionProxy* session = theConnectionService.connect( connectionString, coral::ReadOnly  );
    session->transaction().start( true);
    std::set<std::string> tables = session->nominalSchema().listTables();
    if(tables.find(T1)!=tables.end()) {
      std::cout << "start" <<std::endl;
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table " << T1 << " is found."<<std::endl;
      for(std::set<std::string>::const_iterator iter=tables.begin(); iter!=tables.end(); iter++) {
        std::cout << "Found table "<< *iter << " ."<<std::endl;
      }
      std::cout << "end" <<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;
    session = 0;
    connectionString = getServiceName(3);
    session = theConnectionService.connect( connectionString, coral::ReadOnly  );
    session->transaction().start( true );
    tables = session->nominalSchema().listTables();
    if(tables.find(T1)==tables.end()) {
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table " << T1 << " is not found."<<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;

  }catch ( const coral::Exception& exc) {
    std::cout << "TEST ERROR: "<<exc.what()<<std::endl;
  }


}
