#include <cstdlib>
#include <fstream>
#include <set>
#include <stdexcept>

#include "CoralBase/boost_filesystem_headers.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralCommon/Utilities.h"

#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoring.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"

#include "TestApp.h"
#include "UsermonService.h"
#include "UserReporter.h"

TestApp::TestApp(const char * testName) : TestEnv(testName)
{
  addTablePrefix(T1, "T1");
}

TestApp::~TestApp()
{
}

void TestApp::run()
{
  sw.loadComponent("CORAL/Services/ConnectionService");
  sw.loadComponent("CORAL/Services/MonitoringService");

  coral::IConnectionService& conservice = sw.getConnectionService();

  // Retrieve handle to conection service configuration
  coral::IConnectionServiceConfiguration& csConfig = conservice.configuration();

  // Enable CORAL client side monitoring at trace level (maximum verbosity)

#ifdef CORAL_SEALED

  usermon::UsermonService * m_monSvc = new usermon::UsermonService(sw.getSealContext().get());
  usermon::UserReporter * m_monReporter = new usermon::UserReporter(sw.getSealContext().get());

#else

  usermon::UsermonService * m_monSvc = new usermon::UsermonService();
  usermon::UserReporter * m_monReporter = new usermon::UserReporter( *m_monSvc );

  m_monSvc->setReporter( m_monReporter );

  csConfig.setMonitoringService( dynamic_cast<coral::monitor::IMonitoringService&>(*m_monSvc) );

#endif


  //  csConfig.setMonitoringService( *m_monSvc );


  csConfig.setMonitoringLevel( coral::monitor::Trace );

  this->writeData(conservice);
  this->readData(conservice);
  this->report(conservice);

  delete m_monReporter;
  delete m_monSvc;
}

void TestApp::writeData(coral::IConnectionService& connservice)
{
  std::auto_ptr<coral::ISessionProxy> session( connservice.connect( getServiceName(0) ) );

  session->transaction().start();

  // Create the table
  session->nominalSchema().dropIfExistsTable( T1 );
  coral::TableDescription description1( "SchemaDefinition_Test" );
  description1.setName( T1 );
  description1.insertColumn( "id",
                             coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  // Define primary key
  description1.setPrimaryKey( "id" );
  description1.insertColumn( "t",
                             coral::AttributeSpecification::typeNameForId( typeid(short) ) );
  description1.insertColumn( "Xx",
                             coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  // Set column not null
  description1.setNotNullConstraint( "Xx" );
  description1.insertColumn( "Y",
                             coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  // Add a unique constraint specifying a name
  description1.setUniqueConstraint( "Y", "U_" + T1 + "_Y" );
  description1.insertColumn( "Z",
                             coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  // Add a variable size string
  description1.insertColumn( "Comment",
                             coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                             100,
                             false );
  // Add a blob
  description1.insertColumn( "Data",
                             coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );

  // Create the actual table
  coral::ITable& table = session->nominalSchema().createTable( description1 );


  // Change the column name
  table.schemaEditor().renameColumn( "Xx", "X" );

  // Change the column type
  table.schemaEditor().changeColumnType( "t",
                                         coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  // Add a not null constaint
  table.schemaEditor().setNotNullConstraint( "t" );

  // Add a unique constraint made out of two columns
  std::vector< std::string > constraintColumns( 2 );
  constraintColumns[0] = "X";
  constraintColumns[1] = "Z";
  table.schemaEditor().setUniqueConstraint( constraintColumns );

  /// Add a unique index
  table.schemaEditor().createIndex( T1 + "_IDX_t", "t", true );

  // Give privileges
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  coral::ITableDataEditor& editor1 = table.dataEditor();
  coral::AttributeList rowBuffer1;

  rowBuffer1.extend<long>( "id" );
  long& id = rowBuffer1[ "id" ].data<long>();

  rowBuffer1.extend< long long >( "t" );
  long long& t = rowBuffer1[ "t" ].data<long long>();

  rowBuffer1.extend< float >( "X" );
  float& x = rowBuffer1[ "X" ].data<float>();

  rowBuffer1.extend< double >( "Y" );
  double& y = rowBuffer1[ "Y" ].data<double>();

  rowBuffer1.extend< double >( "Z" );
  double& z = rowBuffer1[ "Z" ].data<double>();

  rowBuffer1.extend< std::string >( "Comment" );
  std::string& comment = rowBuffer1[ "Comment" ].data<std::string>();

  rowBuffer1.extend< coral::Blob >( "Data" );
  coral::Blob& blob = rowBuffer1[ "Data" ].data<coral::Blob>();

  for ( int i = 0; i < 10; ++i ) {
    id = i + 1;
    t = 1;
    t <<= 4*i;
    // x is float
    x = 1 + (i+1)*(float)0.001;
    // y and z are double
    y = 2 + (i+1)*0.0001;
    z = i*1.1;
    std::ostringstream os;
    os << "Row " << i + 1;
    comment = os.str();

    int blobSize = 1000 * ( i + 1 );
    blob.resize( blobSize );
    unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
    for ( int j = 0; j < blobSize; ++j, ++p ) *p = ( i + j )%256;

    if ( i%4 == 2 ) {
      rowBuffer1[ "Z" ].setNull( true );
    }
    else {
      rowBuffer1[ "Z" ].setNull( false );
    }

    // Insert the row
    editor1.insertRow( rowBuffer1 );
  }


  session->transaction().commit();

  coral::sleepSeconds( 1 );
}

void TestApp::readData(coral::IConnectionService& connservice)
{
  std::auto_ptr<coral::ISessionProxy> session( connservice.connect( getServiceName(0) ) );

  session->transaction().start( /* read-only is */ true );

  for ( int i = 0; i < 10; ++i ) {
    coral::IQuery* query0 = session->nominalSchema().tableHandle( T1 ).newQuery();
    query0->addToOutputList( "Data" );
    query0->addToOutputList( "Z" );
    query0->addToOrderList( "id" );
    coral::ICursor& cursor0 = query0->execute();
    while ( cursor0.next() ) {}

    if ( i == 5 )
      connservice.configuration().setMonitoringLevel( coral::monitor::Off );  // Stop CORAL client side monitoring

    delete query0;
  }

  session->transaction().commit();
}

void TestApp::report(coral::IConnectionService& connservice) const
{
  boost::filesystem::path filepath( "report.txt" );
  if ( ::getenv( "CMTCONFIG" ) ) filepath = ::getenv( "CMTCONFIG" ) / filepath;
  std::ofstream log( filepath.string().c_str() );

  std::set<std::string> ds = connservice.monitoringReporter().monitoredDataSources();

  for( std::set<std::string>::iterator dsit = ds.begin(); dsit != ds.end(); ++dsit )
  {
    log << "Report for data source >> " << (*dsit) << " <<" << std::endl;
    connservice.monitoringReporter().reportToOutputStream( (*dsit), log, coral::monitor::Trace );
  }

  log << std::flush;

  log.close();
}
