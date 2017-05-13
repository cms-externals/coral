#include "TestApp.h"
#include <stdexcept>
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IReplicaSortingAlgorithm.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
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

class KeepOnlyOneReplica : public coral::IReplicaSortingAlgorithm {
public:
  KeepOnlyOneReplica(unsigned int replicaIndex) : m_replicaIndex(replicaIndex){}

  virtual ~KeepOnlyOneReplica(){}

  void sort(std::vector<const coral::IDatabaseServiceDescription*>& replicaSet){
    std::vector<const coral::IDatabaseServiceDescription*> vec;
    if(m_replicaIndex+1 <= replicaSet.size()) {
      vec.push_back(replicaSet[m_replicaIndex]);
    }
    replicaSet = vec;
  }

  void setReplicaIndex(unsigned int replicaIndex){
    m_replicaIndex = replicaIndex;
  }


private:
  unsigned int m_replicaIndex;
};

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
  KeepOnlyOneReplica* sorter = new KeepOnlyOneReplica(0);

#ifdef CORAL_SEALED
  theConnectionService.configuration().setReplicaSortingAlgorithm(sorter);
#else
  theConnectionService.configuration().setReplicaSortingAlgorithm(*sorter);
#endif

  setupDataSource( getServiceName(0) );

  theConnectionService.purgeConnectionPool();

  try {
    coral::ISessionProxy* session = 0;
    std::string connectionString =  getServiceName(1);
    session = theConnectionService.connect( connectionString, coral::ReadOnly  );
    session->transaction().start( true );
    std::set<std::string> tables = session->nominalSchema().listTables();
    session->transaction().commit();
    delete session;
    session = 0;
    sorter->setReplicaIndex( 1 );
    connectionString = getServiceName(1);
    session = theConnectionService.connect( connectionString, coral::Update  );
    session->transaction().start();
    tables = session->nominalSchema().listTables();
    if(tables.find(T1)==tables.end()) {
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table " << T1 << " is not found."<<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;
    session = 0;
    connectionString = getServiceName(2);
    try {
      sorter->setReplicaIndex( 2 );
      session = theConnectionService.connect( connectionString, coral::Update  );
      session->transaction().start();
      tables = session->nominalSchema().listTables();
      session->transaction().commit();
      delete session;
    } catch (const coral::ConnectionServiceException& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
    delete sorter;
  }catch ( const coral::Exception& exc) {
    std::cout << "TEST ERROR: "<<exc.what()<<std::endl;
  }


}
