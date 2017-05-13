#include "TestDriver.h"

#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"

#include "CoralCommon/Utilities.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include <stdexcept>
#include <sstream>
#include <cfloat>
#include <cmath>


static const std::string tableName = "TABLE_FOR_TEST_SP";
static const std::string storedProcedureName = "TEST_SP";

static const int arg1 = 10;
static const int arg2 = 12341;


TestDriver::TestDriver()
{
}


TestDriver::~TestDriver()
{
}


void
TestDriver::run( const std::string& serviceName )
{
  this->prepareSchema( serviceName );
  this->writeData( serviceName );
  this->readData( serviceName );
}

void
TestDriver::prepareSchema( const std::string& serviceName )
{
  coral::ConnectionService connectionService;

  coral::ISessionProxy* session = connectionService.connect( serviceName, coral::Update );
  if ( ! session )
    throw std::runtime_error( "Could not connect to " + serviceName );

  session->transaction().start();

  coral::ISchema& schema = session->nominalSchema();

  // Clear the old table
  schema.dropIfExistsTable( tableName );

  // Create the new table
  coral::TableDescription description;
  description.setName( tableName );

  description.insertColumn( "I",
                            coral::AttributeSpecification::typeNameForId( typeid( int ) ) );
  description.setPrimaryKey( "I" );

  description.insertColumn( "X",
                            coral::AttributeSpecification::typeNameForId( typeid( int ) ) );
  schema.createTable( description );


  // Run the stored procedure

  session->transaction().commit();

  delete session;

  coral::sleepSeconds( 1 );
}


void
TestDriver::writeData( const std::string& serviceName )
{
  coral::ConnectionService connectionService;

  coral::ISessionProxy* session = connectionService.connect( serviceName, coral::Update );
  if ( ! session )
    throw std::runtime_error( "Could not connect to " + serviceName );

  session->transaction().start();

  coral::ISchema& schema = session->nominalSchema();



  // Call the stored procedure
  coral::AttributeList inputData;
  inputData.extend<int>( "ARG1" );
  inputData.extend<int>( "ARG2" );
  inputData[0].data<int>() = arg1;
  inputData[1].data<int>() = arg2;

  schema.callProcedure( storedProcedureName, inputData );

  session->transaction().commit();

  delete session;
}



void
TestDriver::readData( const std::string& serviceName )
{
  coral::ConnectionService connectionService;

  coral::ISessionProxy* session = connectionService.connect( serviceName );
  if ( ! session )
    throw std::runtime_error( "Could not connect to " + serviceName );

  session->transaction().start( true );

  coral::ISchema& schema = session->nominalSchema();

  coral::IQuery* query = schema.tableHandle( tableName ).newQuery();

  coral::AttributeList output;
  output.extend<int>( "I" );
  output.extend<int>( "X" );

  query->addToOutputList( "I" );
  query->addToOutputList( "X" );
  query->addToOrderList( "I" );
  query->defineOutput( output );

  int nRows = 0;
  coral::ICursor& cursor = query->execute();

  while ( cursor.next() ) {
    ++nRows;
    if ( output[0].data<int>() != nRows ||
         output[1].data<int>() != nRows + arg2 )
      throw std::runtime_error( "Unexpected output" );
  }

  if ( nRows != arg1 )
    throw std::runtime_error( "Unexpected number of returned rows" );

  delete query;

  session->transaction().commit();

  delete session;
}
