#include "TestApp.h"

#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"

#include "CoralCommon/Utilities.h"

#include <stdexcept>
#include <iostream>
#include <sstream>

TestApp::TestApp(const char * testName)
  : TestEnv(testName)
{
  addTablePrefix(T1, "T1");
}

TestApp::~TestApp()
{
}

void
TestApp::loadServices()
{
  sw.loadComponent("CORAL/Services/XMLAuthenticationService");
  sw.loadComponent("CORAL/Services/MonitoringService");
  sw.loadComponent("CORAL/Services/RelationalService");
}

void
TestApp::run()
{
  try
  {
    this->writeData();
    this->readData();
    this->report();
  }
  catch( const std::exception& e )
  {
    std::cerr << "Std. exception caught: " << e.what() << std::endl;
    throw;
  }
}

void TestApp::writeData()
{
  ///new independent object sw is handling now all connection stuff (seal, noseal version)
  //extract the connection string from the service name
  std::vector<std::string> connstrings = sw.getAllReplicas(getServiceName(0), coral::Update, 1);
  std::string connectionString(connstrings[0]);

  std::cout << "Try to connect to " << connectionString << std::endl;
  //get the authentication credentials
  const coral::IAuthenticationCredentials& crds = sw.getCredentials(connectionString);
  //get the domain
  std::pair<std::string,std::string> cs = sw.getDomain( connectionString ).decodeUserConnectionString( connectionString );
  //create a new connection
  std::cout << "Create connection to database='" << cs.first << "'" << std::endl;
  std::auto_ptr<coral::IConnection> connection( sw.getDomain( connectionString ).newConnection( cs.first ) );
  connection->connect();
  //create a new session
  std::cout << "Create session for schema='" << cs.second << "'" << std::endl;
  std::auto_ptr<coral::ISession> session( connection->newSession( cs.second, coral::Update ) );
  //start monitoring
  std::cout << "Start monitoring for the session" << std::endl;
  session->monitoring().start( coral::monitor::Trace );
  //start monitoring
  std::cout << "Start user session for user='" << crds.valueForItem("user") << "'" << std::endl;
  session->startUserSession( crds.valueForItem("user"), crds.valueForItem("password") );

  // Create the table
  session->transaction().start();
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
    // y, z double
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


void
TestApp::readData()
{
  //extract the connection string from the service name
  std::vector<std::string> connstrings = sw.getAllReplicas(getServiceName(0), coral::Update, 1);
  std::string connectionString(connstrings[0]);
  //get the authentication credentials
  const coral::IAuthenticationCredentials& crds = sw.getCredentials(connectionString);
  //get the domain
  std::pair<std::string,std::string> cs = sw.getDomain( connectionString ).decodeUserConnectionString( connectionString );

  std::auto_ptr<coral::IConnection> connection( sw.getDomain( connectionString ).newConnection( cs.first ) );

  connection->connect();

  std::auto_ptr<coral::ISession> session( connection->newSession( cs.second, coral::ReadOnly ) );

  session->monitoring().start( coral::monitor::Trace );

  session->startUserSession( crds.valueForItem("user"), crds.valueForItem("password") );

  session->transaction().start( true );

  for ( int i = 0; i < 10; ++i ) {
    coral::IQuery* query0 = session->nominalSchema().tableHandle( T1 ).newQuery();
    query0->addToOutputList( "Data" );
    query0->addToOutputList( "Z" );
    query0->addToOrderList( "id" );
    coral::ICursor& cursor0 = query0->execute();
    while ( cursor0.next() ) {}
    if ( i == 5 )
      session->monitoring().stop();
    delete query0;
  }

  session->transaction().commit();
}


void
TestApp::report()
{
  //extract the connection string from the replicas
  std::vector<std::string> connstrings = sw.getAllReplicas(getServiceName(0), coral::Update, 1);
  std::string connectionString(connstrings[0]);
  //get the domain
  // std::pair<std::string,std::string> cs = sw.getDomain( connectionString ).decodeUserConnectionString( connectionString );
  sw.getMonitoringReporter().report();

  std::cout << "Available reporter : " << std::endl;

  std::set< std::string > m_reports = sw.getMonitoringReporter().monitoredDataSources();
  std::set< std::string >::iterator m_iter;

  for ( m_iter = m_reports.begin( ); m_iter != m_reports.end( ); m_iter++ )
    std::cout << "reporter : " << *m_iter << std::endl;

  //std::cout << "Monitoring report for connection" << std::endl;
  //std::string::size_type pos = connectionString.rfind( '/' );
  //std::string connstr = connectionString.substr( 0, pos );
  //monitoringService->reporter().report( connectionString );

  std::cout << "Monitoring report for session" << std::endl;
  sw.getMonitoringReporter().report( connectionString );
}
