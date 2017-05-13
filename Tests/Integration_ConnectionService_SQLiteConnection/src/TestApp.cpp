#include "TestApp.h"
#include <stdexcept>
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITransaction.h"
#include "CoralBase/Exception.h"
#include "CoralBase/AttributeSpecification.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "CoralCommon/Utilities.h"
#include "CoralBase/AttributeSpecification.h"
#include <iostream>

const std::string TestApp::OVALTAG = "[OVAL]";

TestApp::TestApp(const char * testName) : TestEnv(testName)
{
  addTablePrefix(T1, "T1");
  addTablePrefix(T2, "T2");
}


TestApp::~TestApp()
{
}

void
TestApp::run(){
  coral::IConnectionService& theConnectionService = sw.getConnectionService();
  //theConnectionService->configuration().setConnectionTimeOut(10);
  //theConnectionService->purgeConnectionPool();

  std::string connectionString( getServiceName(0) );
  coral::ISessionProxy* session = 0;

  try {

    session = theConnectionService.connect( connectionString, coral::Update  );
    coral::ITransaction& transaction = session->transaction();
    bool active = transaction.isActive();
    if(!active) transaction.start(true);

    //unused?    coral::ISchema& nominalSchema = session->nominalSchema();

    if(!active) transaction.commit();
    std::cout << OVALTAG << "Connection "<<connectionString<<" was ok."<<std::endl;


    coral::ISessionProxy* tmpSession = theConnectionService.connect( connectionString, coral::ReadOnly  );
    coral::ITransaction& tmptransaction = tmpSession->transaction();
    bool tmpactive = tmptransaction.isActive();
    if(!tmpactive) tmptransaction.start( true );

    if (tmpSession->nominalSchema().existsTable(T1))
      std::cout << "found " << T1 << "." << std::endl;


    if(!tmpactive) tmptransaction.commit();
    std::cout << OVALTAG << "Connection ok "<<connectionString<<" was ok."<<std::endl;
    delete tmpSession;

    if(!active) transaction.start(true);
    //session->transaction().start(true);
    coral::ITypeConverter& typeConverter = session->typeConverter();
    typeConverter.sqlTypeForCppType( coral::AttributeSpecification::typeNameForType<std::string>() );
    coral::TableDescription descr;
    descr.setName(T2);
    descr.insertColumn("N_X",coral::AttributeSpecification::typeNameForType<int>());
    descr.insertColumn("N_S",coral::AttributeSpecification::typeNameForType<std::string>());
    session->nominalSchema().createTable( descr );
    if(!active) transaction.commit();
    //session->transaction().commit();


  }catch ( const coral::Exception& exc) {
    std::cout << "un expected ERROR: "<<exc.what()<<std::endl;
  }


}
