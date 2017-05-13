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
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ConnectionServiceException.h"
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
void TestApp::setupDataSource( const std::string& connectionString){
  coral::IConnectionService& theConnectionService = sw.getConnectionService();

  coral::ISessionProxy* session = theConnectionService.connect( connectionString );
  session->transaction().start();
  // creates a dummy table to be looked up
  session->nominalSchema().dropIfExistsTable(T1);
  coral::TableDescription descr;
  descr.setName(T1);
  descr.insertColumn("N_X",coral::AttributeSpecification::typeNameForType<int>());
  descr.insertColumn("N_S",coral::AttributeSpecification::typeNameForType<std::string>());
  session->nominalSchema().createTable( descr );
  session->transaction().commit();
  delete session;
  coral::sleepSeconds(1);
}

void
TestApp::run(){
  coral::IConnectionService& theConnectionService = sw.getConnectionService();

  theConnectionService.configuration().setConnectionTimeOut(0);

  setupDataSource( getServiceName(0) );


  try {
    try {
      std::cout << "### Testing case=Non existing service ------>"<<std::endl;
      std::string connectionString = getServiceName(1);
      coral::ISessionProxy* session = theConnectionService.connect( connectionString, coral::ReadOnly  );
      session->transaction().start( true);
      session->transaction().commit();
      delete session;
    } catch (const coral::ConnectionServiceException& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
    theConnectionService.purgeConnectionPool();
    std::cout << "    Try again... ------>"<<std::endl;
    try {
      std::string connectionString = getServiceName(1);
      coral::ISessionProxy* session = theConnectionService.connect( connectionString, coral::ReadOnly  );
      session->transaction().start( true);
      session->transaction().commit();
      delete session;
    } catch (const coral::ConnectionServiceException& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
    theConnectionService.purgeConnectionPool();
    std::cout << "### Testing case=Non existing sqlite file ------>"<<std::endl;
    try {
      std::string connectionString = getServiceName(2);
      coral::ISessionProxy* session = theConnectionService.connect( connectionString, coral::ReadOnly  );
      session->transaction().start( true);
      session->transaction().commit();
      delete session;
    } catch (const coral::ConnectionServiceException& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
    theConnectionService.purgeConnectionPool();
    std::cout << "    Try again... ------>"<<std::endl;
    try {
      std::string connectionString = getServiceName(2);
      coral::ISessionProxy* session = theConnectionService.connect( connectionString, coral::ReadOnly  );
      session->transaction().start( true);
      session->transaction().commit();
      delete session;
    } catch (const coral::ConnectionServiceException& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
    theConnectionService.purgeConnectionPool();
    std::cout << "### Testing case=No authentication parameters ------>"<<std::endl;
    try {
      std::string connectionString = getServiceName(3);
      coral::ISessionProxy* session = theConnectionService.connect( connectionString, coral::ReadOnly  );
      session->transaction().start( true);
      session->nominalSchema().listTables();
      session->transaction().commit();
      delete session;
    } catch (const coral::ConnectionServiceException& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
    theConnectionService.purgeConnectionPool();
    std::cout << "    Try again... ------>"<<std::endl;
    try {
      std::string connectionString = getServiceName(3);
      coral::ISessionProxy* session = theConnectionService.connect( connectionString, coral::ReadOnly  );
      session->transaction().start( true);
      session->nominalSchema().listTables();
      session->transaction().commit();
      delete session;
    } catch (const coral::ConnectionServiceException& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
  }catch ( const coral::Exception& exc) {
    std::cout << "TEST ERROR: "<<exc.what()<<std::endl;
  }


}
