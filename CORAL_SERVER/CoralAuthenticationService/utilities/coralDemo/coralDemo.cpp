#include <iostream>
#include "CoralKernel/Context.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProxy.h"

void loadComponent( std::string component)
{
  coral::Context& context( coral::Context::instance() );

  context.loadComponent( component );
}

coral::IConnectionService& getConnectionService()
{
  coral::IHandle<coral::IConnectionService> handle(
                                                   coral::Context::instance().query<coral::IConnectionService>(
                                                                                                               "CORAL/Service/ConnectionService"));
  if ( !handle.isValid() ) {
    loadComponent("CORAL/Service/ConnectionService");
    handle = coral::Context::instance().query<coral::IConnectionService>(
                                                                         "CORAL/Service/ConnectionService");
  };

  if ( !handle.isValid() )
    throw coral::Exception("Could not lead CORAL connectionService","getConnectionService()","demo");

  return *(handle.get());
}

int main() {

  coral::IConnectionService *connService = new coral::ConnectionService();
  bool sharing = true;
  if(!sharing) {
    coral::IConnectionServiceConfiguration& config = connService->configuration();
    config.disableConnectionSharing();
  }

  std::string serviceName = "sqlite_file:COOLTEST.db";
  coral::ISessionProxy *session = connService->connect( serviceName,
                                                        coral::Update );
  if ( !session )
    throw coral::Exception("Could not connect to " + serviceName,"main()","demo" );

  // start an update transaction
  session->transaction().start( false /*readonly*/ );

  // create a table description
  coral::TableDescription tableDesc;
  tableDesc.setName("table1");

  tableDesc.insertColumn( "id", coral::AttributeSpecification::typeNameForId(
                                                                             typeid( unsigned int ) ) );
  tableDesc.setPrimaryKey( "id" );
  tableDesc.insertColumn( "name", "string", 255, false );

  // first drop the table if it exists, then create it
  session->nominalSchema().dropIfExistsTable( tableDesc.name() );
  coral::ITable& table = session->nominalSchema().createTable( tableDesc );

  // fill the table
  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );
  for ( size_t i = 0; i<20; i++)
  {
    std::cout << "Filling row " << i << std::endl;
    rowBuffer["id"].data<unsigned int>() = i;
    rowBuffer["name"].data<std::string>() = "name";
    table.dataEditor().insertRow( rowBuffer );
  };

  session->transaction().commit();
}
