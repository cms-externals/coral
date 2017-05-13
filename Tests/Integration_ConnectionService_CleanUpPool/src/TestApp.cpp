#include "TestApp.h"
#include <iostream>
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
#include "../../ConnectionService/src/ConnectionService.h"
#include "../../ConnectionService/src/SessionProxy.h"
#include "../../ConnectionService/src/ConnectionPool.h"

const std::string TestApp::OVALTAG = "[OVAL]";

TestApp::TestApp(const char * testName)
  : TestEnv(testName)
{
}


TestApp::~TestApp()
{
}

void
TestApp::run()
{
  addTablePrefix(T1, "T1");

  coral::IConnectionService& theConnectionService = sw.getConnectionService();

  theConnectionService.configuration().setConnectionTimeOut(0);

  std::cout << "Getting the first sessionProxy object "<<std::endl;
  ///*** new *** get the service name from the TestEnv class
  std::string connectionString( getServiceName(0) );

  coral::ISessionProxy* session1 = theConnectionService.connect( connectionString );

  session1->transaction().start( true );
  std::set<std::string> tables = session1->nominalSchema().listTables();
  session1->transaction().commit();

  if(tables.find(T1) == tables.end()) {
    std::cout << OVALTAG << "TEST ERROR with connection " << connectionString << " " << T1 << " is not found." << std::endl;
  } else {
    std::cout << OVALTAG << "Connection " << connectionString << " was ok." << std::endl;
  }

  coral::ConnectionService::ConnectionService& myService = dynamic_cast<coral::ConnectionService::ConnectionService&>(theConnectionService);

  coral::ISessionProxy* session2 = theConnectionService.connect( connectionString );
  coral::ISessionProxy* session3 = theConnectionService.connect( connectionString );
  coral::ISessionProxy* session4 = theConnectionService.connect( connectionString );
  coral::ISessionProxy* session5 = theConnectionService.connect( connectionString );

  std::cout << "--------------------------(1)"<<std::endl;
  std::cout << OVALTAG <<"Number of idle connection:"<< myService.numberOfIdleConnectionsInPool()<<std::endl;
  std::cout << OVALTAG <<"Number of active connection:"<< myService.numberOfActiveConnectionsInPool()<<std::endl;

  delete session1;
  delete session2;
  delete session3;
  delete session4;
  delete session5;

  std::cout << "--------------------------(2)"<<std::endl;
  std::cout << OVALTAG <<"Number of idle connection:"<< myService.numberOfIdleConnectionsInPool()<<std::endl;
  std::cout << OVALTAG <<"Number of active connection:"<< myService.numberOfActiveConnectionsInPool()<<std::endl;
  theConnectionService.purgeConnectionPool();
  std::cout << "--------------------------(3)"<<std::endl;
  std::cout << OVALTAG <<"Number of idle connection:"<< myService.numberOfIdleConnectionsInPool()<<std::endl;
  std::cout << OVALTAG <<"Number of active connection:"<< myService.numberOfActiveConnectionsInPool()<<std::endl;

  //dont need here, exception handling in the main program
  // }catch ( const coral::Exception& exc) {
  //   std::cout << "TEST ERROR: "<<exc.what()<<std::endl;
  //  }catch (...) {
  //    std::cout << "Test Error: unkown exception" << std::end1;
  //  }


}
