#include "TestApp.h"
#ifdef CORAL_SEALED
#include "SealKernel/ComponentLoader.h"
#include "SealKernel/IMessageService.h"
#include "PluginManager/PluginManager.h"
#include <stdexcept>
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITransaction.h"
#include "CoralBase/Exception.h"
#include "CoralBase/AttributeSpecification.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "CoralCommon/Utilities.h"
#include "../../../LFCReplicaService/src/LFCReplicaService.h"
#include "../../../LFCReplicaService/src/ReplicaSet.h"
#include "../../../LFCReplicaService/src/XMLReplicaImporter.h"

const std::string TestApp::OVALTAG = "[OVAL]";
#endif

TestApp::TestApp()
#ifdef CORAL_SEALED
  : m_context( new seal::Context )
#endif
{
  this->loadServices();
}


TestApp::~TestApp()
{
}

void
TestApp::loadServices()
{
#ifdef CORAL_SEALED
  seal::PluginManager* pm = seal::PluginManager::get();
  pm->initialise();
  seal::Handle<seal::ComponentLoader> loader = new seal::ComponentLoader( m_context.get() );

  // Load the SEAL Message service and set the output threshold to Info
  loader->load( "SEAL/Services/MessageService" );
  std::vector< seal::Handle<seal::IMessageService> > v_msgSvc;
  m_context->query( v_msgSvc );
  if ( ! v_msgSvc.empty() ) {
    seal::Handle<seal::IMessageService>& msgSvc = v_msgSvc.front();
    msgSvc->setOutputLevel( seal::Msg::Debug);
  }

  // Load the Connection Service
  std::cout << "Loading Connection Service"<<std::endl;
  loader->load( "CORAL/Services/ConnectionService" );
#endif
}

#ifdef CORAL_SEALED
seal::IHandle<coral::IConnectionService>
TestApp::connectionService()
{
  std::vector< seal::IHandle<coral::IConnectionService> > v_svc;
  m_context->query( v_svc );
  if ( v_svc.empty() ) {
    throw std::runtime_error( "Could not locate the connection service" );
  }
  return v_svc.front();
}

//
void TestApp::setupDataSource( coral::ISessionProxy& session ){
  session.transaction().start();
  // creates a dummy table to be looked up
  session.nominalSchema().dropIfExistsTable("T_CONNECTION_SERVICE_TEST");
  coral::TableDescription descr;
  descr.setName("T_CONNECTION_SERVICE_TEST");
  descr.insertColumn("N_X",coral::AttributeSpecification::typeNameForType<int>());
  descr.insertColumn("N_S",coral::AttributeSpecification::typeNameForType<std::string>());
  session.nominalSchema().createTable( descr );
  session.transaction().commit();
  coral::sleepSeconds(1);
}
#endif

void
TestApp::run()
{
#ifdef CORAL_SEALED
  seal::IHandle<coral::IConnectionService> theConnectionService = connectionService();
  if(! theConnectionService ) {
    std::cout << "Connection Service not loaded."<<std::endl;
    return;
  }
  //theConnectionService->configuration().disablePoolAutomaticCleanUp();
  theConnectionService->configuration().setDefaultLookupService("CORAL/Services/LFCReplicaService");
  theConnectionService->configuration().setDefaultAuthenticationService("CORAL/Services/LFCReplicaService");

  try {
    // here we non existing conection, just to let the LFCreplicaService to be loaded
    try {
      std::string connectionString = "missing_connection_string";
      coral::ISessionProxy* session = theConnectionService->connect( connectionString, coral::ReadOnly  );
      session->transaction().start( true);
      session->transaction().commit();
      delete session;
    } catch (const coral::Exception& ex) {
      std::cout << OVALTAG <<"Exception:"<<ex.what()<<std::endl;
    }
    seal::Handle<coral::LFCReplicaService::LFCReplicaService> replicaService = m_context->component<coral::LFCReplicaService::LFCReplicaService>("CORAL/Services/LFCReplicaService");
    if(!replicaService) throw std::runtime_error("The LFC Replica service has not been loaded.");
    coral::LFCReplicaService::ReplicaFilter filter;
    try{
      replicaService->deleteReplicas("/connection_folder/connection0",
                                     "",
                                     filter);
    } catch (coral::Exception& ex) {
      std::cout << "Exception:"<<ex.what()<<std::endl;
    }
    try{
      replicaService->deleteReplicas("/connection_folder/connection1",
                                     "",
                                     filter);
    } catch (coral::Exception& ex) {
      std::cout << "Exception:"<<ex.what()<<std::endl;
    }
    std::string authenticationFile("/afs/cern.ch/sw/lcg/app/pool/db/LFCReplicaService/authentication.xml");
    std::string lookupFile("/afs/cern.ch/sw/lcg/app/pool/db/LFCReplicaService/dblookup.xml");
    coral::LFCReplicaService::XMLReplicaImporter importer(m_context.get(),"coral_replica_manager",lookupFile,authenticationFile);
    coral::LFCReplicaService::ReplicaSet* replicaSet = importer.getData();
    if( replicaSet) {
      replicaService->importReplicas( *replicaSet );
      delete replicaSet;
    }


    theConnectionService->configuration().disableReadOnlySessionOnUpdateConnections();
    std::string connectionString = "/connection_folder/connection0";
    coral::ISessionProxy* session = theConnectionService->connect( connectionString );
    setupDataSource( *session );
    session->transaction().start( true);
    std::set<std::string> tables = session->nominalSchema().listTables();
    if(tables.find("T_CONNECTION_SERVICE_TEST")==tables.end()) {
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table T_CONNECTION_SERVICE_TEST is not found."<<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;
    session = 0;
    connectionString = "/connection_folder/connection0";
    session = theConnectionService->connect( connectionString, coral::ReadOnly   );
    session->transaction().start( true );
    tables = session->nominalSchema().listTables();
    if(tables.find("T_CONNECTION_SERVICE_TEST")==tables.end()) {
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table T_CONNECTION_SERVICE_TEST is not found."<<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;
    session = 0;
    connectionString = "/connection_folder/connection0";
    std::string role("writer");
    std::cout << "# Connection with role=\""<<role<<"\""<<std::endl;
    session = theConnectionService->connect( connectionString, role );
    session->transaction().start( true );
    tables = session->nominalSchema().listTables();
    if(tables.find("T_CONNECTION_SERVICE_TEST")==tables.end()) {
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table T_CONNECTION_SERVICE_TEST is not found."<<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;
    session = 0;
    role = "reader";
    std::cout << "# Connection with role=\""<<role<<"\""<<std::endl;
    try{
      session = theConnectionService->connect( connectionString, role );
      session->transaction().start( true );
      tables = session->nominalSchema().listTables();
      session->transaction().commit();
      delete session;
      session = 0;
    } catch (coral::Exception& e) {
      std::cout << "Connection not established: "<<e.what()<<std::endl;
    }
    connectionString = "/connection_folder/connection1";
    session = theConnectionService->connect( connectionString, coral::Update  );
    setupDataSource( *session );
    session->transaction().start();
    tables = session->nominalSchema().listTables();
    if(tables.find("T_CONNECTION_SERVICE_TEST")==tables.end()) {
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table T_CONNECTION_SERVICE_TEST is not found."<<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;
    session = 0;
    role = "writer";
    connectionString = "/connection_folder/connection1";
    std::cout << "# Connection with role=\""<<role<<"\""<<std::endl;
    session = theConnectionService->connect( connectionString, role, coral::Update  );
    session->transaction().start();
    tables = session->nominalSchema().listTables();
    if(tables.find("T_CONNECTION_SERVICE_TEST")==tables.end()) {
      std::cout << OVALTAG <<"TEST ERROR with connection "<<connectionString<<" table T_CONNECTION_SERVICE_TEST is not found."<<std::endl;
    } else {
      std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;
    }
    session->transaction().commit();
    delete session;
    session = 0;
    role = "reader";
    connectionString = "/connection_folder/connection1";
    std::cout << "# Connection with role=\""<<role<<"\""<<std::endl;
    try{
      session = theConnectionService->connect( connectionString, role, coral::Update  );
      session->transaction().start();
      tables = session->nominalSchema().listTables();
      session->transaction().commit();
      delete session;
      session = 0;
    } catch (coral::Exception& e) {
      std::cout << "Connection not established: "<<e.what()<<std::endl;
    }
  }catch ( const coral::Exception& exc) {
    std::cout << "TEST ERROR: "<<exc.what()<<std::endl;
  }

#endif
}
