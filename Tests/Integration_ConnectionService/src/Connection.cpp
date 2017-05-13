#include "Connection.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include <stdexcept>
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include <set>

Connection::Connection(const char * testName) : TestEnv(testName)
{
  addTablePrefix(T1, "T1");
}

Connection::~Connection()
{
}

void
Connection::Init()
{
  coral::ConnectionService connServ;
  coral::IConnectionServiceConfiguration& config = connServ.configuration();
  config.setConnectionRetrialPeriod(2);
  config.setConnectionRetrialTimeOut(20);
}

coral::ISessionProxy* Connection::getSessionProxy( int connIndex,
                                                   coral::AccessMode mode)
{
  coral::ConnectionService connServ;
  coral::ISessionProxy * proxy = NULL;
  if(mode == coral::ReadOnly) {
    proxy = connServ.connect(getServiceName(connIndex, coral::ReadOnly), mode );
  }else{
    proxy = connServ.connect(getServiceName(connIndex), mode );
  }
  return proxy;
}

void
Connection::listTables(coral::ISessionProxy * proxy)
{
  proxy->transaction().start();
  std::cout << "Reference table T1: " << T1 <<std::endl;
  std::set< std::string > listOfTables = proxy->nominalSchema().listTables();
  std::cout << "Tables in the schema (" << listOfTables.size() << "):" << std::endl;
  unsigned int nTable = 0;
  for ( std::set< std::string >::const_iterator iTable = listOfTables.begin();
        iTable != listOfTables.end(); ++iTable )
  {
    if ( listOfTables.size() > 6 && nTable==listOfTables.size()-4 )
      std::cout << "..." << std::endl;
    if ( nTable<3 || nTable>listOfTables.size()-4 || *iTable==T1 )
      std::cout << *iTable << std::endl;
    if ( listOfTables.size() > 6 && nTable==3 )
      std::cout << "..." << std::endl;
    ++nTable;
  }
  proxy->transaction().commit();
}
