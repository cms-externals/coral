#include "TopThread.h"
#include "ReadThread.h"
#include "Reporter.h"
#include "GlobalStatus.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IBulkOperation.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"

#include "CoralCommon/Utilities.h"

#include "CoralBase/boost_thread_headers.h"
#include <sstream>
#include <memory>

void
TopThread::operator()()
{
  std::ostringstream osi;
  osi << "Starting top thread " << m_id;
  m_reporter.reportToSTDOUT( osi.str() );

  try {
    // Get an updatable session to clear old tables and write the new ones
    for ( int i = 0; i < 3; ++i ) {

      //      std::ostringstream osServiceName;
      //      osServiceName << "/system/service_" << m_id;
      std::unique_ptr<coral::ISessionProxy> session( m_connectionService.connect( m_env->getServiceName( m_id ) ) );
      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      std::ostringstream osTableName;
      osTableName << "T_" << m_id << "_" << i;
      std::string tableName;
      m_env->addTablePrefix(tableName, osTableName.str() );

      schema.dropIfExistsTable( tableName );
      session->transaction().commit();

      // Sleep for 1 second
      coral::sleepSeconds( 1 );
    }




    // Create three tables
    //    std::ostringstream osServiceName;
    //    osServiceName << "/system/service_" << m_id;
    std::unique_ptr<coral::ISessionProxy> session( m_connectionService.connect( m_env->getServiceName( m_id ) ) );
    session->transaction().start();

    coral::ISchema& schema = session->nominalSchema();

    // Create three tables
    for ( int i = 0; i < 3; ++i ) {

      std::ostringstream osTableName;
      osTableName << "T_" << m_id << "_" << i;
      std::string tableName;
      m_env->addTablePrefix(tableName, osTableName.str() );

      coral::TableDescription description;
      description.setName( tableName );

      description.insertColumn( "ID",
                                coral::AttributeSpecification::typeNameForType<int>() );
      description.insertColumn( "DATA",
                                coral::AttributeSpecification::typeNameForType<int>() );
      description.setPrimaryKey( "ID" );

      coral::ITable& table = schema.createTable( description );
      table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

      coral::AttributeList rowBuffer;
      rowBuffer.extend<int>( "ID" );
      rowBuffer.extend<int>( "DATA" );
      int& id = rowBuffer[0].data<int>();
      int& data = rowBuffer[1].data<int>();

      std::unique_ptr<coral::IBulkOperation> operation( table.dataEditor().bulkInsert( rowBuffer, 100 ) );

      for ( id = 0; id < 1000; ++id ) {
        data = id + 1000 * i + 10000 * m_id;
        operation->processNextIteration();
      }
      operation->flush();

    }
    session->transaction().commit();

    // Sleep for 1 second
    coral::sleepSeconds( 1 );

    session->transaction().start( true );
    coral::ISchema& schemaR = session->nominalSchema();

    // Spawn three reader threads and read back the data

    std::vector< boost::thread* > threads;
    for ( int i = 0; i < 3; ++i )
      threads.push_back( new boost::thread( ReadThread( schemaR, m_reporter, m_id, i, m_ok, m_env ) ) );

    for ( int i = 0; i < 3; ++i )
      threads[i]->join();

    for ( int i = 0; i < 3; ++i ) {
      delete threads[i];
    }

    session->transaction().commit();

  }
  catch ( std::exception& e ) {
    m_ok.setFalse();
    std::ostringstream os;
    os << "Exception caught in top thread " << m_id << " : " << e.what();
    m_reporter.reportToSTDERR( os.str() );
  }
  catch ( ... ) {
    m_ok.setFalse();
    std::ostringstream os;
    os << "Unknown exception caught in top thread " << m_id;
    m_reporter.reportToSTDERR( os.str() );
  }
  std::ostringstream osf;
  osf << "End of thread " << m_id;
  m_reporter.reportToSTDOUT( osf.str() );
}
