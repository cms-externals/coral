#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/RelationalServiceException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Exception.h"
#include "CoralBase/AttributeSpecification.h"

#include "CoralKernel/Context.h"

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cfloat>
#include <cmath>

static const std::string tableName = "UINT_TEST";

int main()
{
  std::string con = "sqlite_file:zhen.db";

  try
  {
    coral::Context& ctx = coral::Context::instance();

    ctx.loadComponent( "CORAL/RelationalPlugins/sqlite" );

    coral::IHandle<coral::IRelationalDomain> iHandle=ctx.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/sqlite" );

    if ( ! iHandle.isValid() )
    {
      throw coral::NonExistingDomainException( "sqlite" );
    }

    std::pair<std::string, std::string> connectionAndSchema = iHandle->decodeUserConnectionString( con );
    coral::IConnection*  connection=iHandle->newConnection( connectionAndSchema.first );
    connection->connect();
    coral::ISession* session=connection->newSession(connectionAndSchema.second);
    std::string userName, password;
    session->startUserSession( userName, password );

    std::vector<unsigned int> myvalues;
    myvalues.push_back(         0u);
    myvalues.push_back(       100u);
    myvalues.push_back(      1000u);
    myvalues.push_back(     10000u);
    myvalues.push_back(2147483648u);
    session->transaction().start(false);
    coral::ISchema& schema = session->nominalSchema();
    schema.dropIfExistsTable( tableName );
    // Create the new table
    coral::TableDescription description;
    description.setName( tableName );
    description.insertColumn( "ID",
                              coral::AttributeSpecification::typeNameForId( typeid( int ) ) );
    description.setNotNullConstraint( "ID" );
    description.setPrimaryKey( "ID" );
    coral::AttributeList rowBuffer;
    rowBuffer.extend<int>( "ID" );
    description.insertColumn( "V_UI",
                              coral::AttributeSpecification::typeNameForId( typeid( unsigned int ) ) );
    rowBuffer.extend<unsigned int>( "V_UI" );
    schema.createTable( description );
    session->transaction().commit();
    // Fill data
    session->transaction().start();
    coral::ITable& table = schema.tableHandle( tableName );
    coral::ITableDataEditor& dataEditor = table.dataEditor();
    for ( size_t i = 0; i < myvalues.size(); ++i ) {
      rowBuffer[0].data<int>() = i;
      rowBuffer[1].data<unsigned int>() = myvalues[i];
      dataEditor.insertRow( rowBuffer );
    }
    session->transaction().commit();
    // Check data
    std::cout<<"Checking data"<<std::endl;
    session->transaction().start();
    table = schema.tableHandle( tableName );
    coral::IQuery* query = table.newQuery();
    // query->addToOrderList( "ID" );
    query->addToOrderList( "V_UI" );
    coral::AttributeList result;
    result.extend<int>( "ID" );
    result.extend<unsigned int>( "V_UI" );
    query->defineOutput( result );
    //  query->setRowCacheSize( m_values.size() );
    coral::ICursor& cursor = query->execute();
    for ( size_t i = 0; i < myvalues.size(); ++i ) {
      if ( ! cursor.next() )
        throw std::runtime_error( "Unexpected number of returned rows" );
      if ( result[0].data<int>() != (int)i ) {
        std::ostringstream msg;
        msg << "Unexpected value for variable " << result[0].specification().name() << ": "<< result[0].data<int>() << " instead of " << i;
        throw std::runtime_error( msg.str() );
      }
      if ( result[1].data<unsigned int>() != myvalues[i] ) {
        std::ostringstream msg;
        msg << "Unexpected value for variable " << result[1].specification().name() << ": "
            << result[1].data<unsigned int>() << " instead of " << myvalues[i];
        throw std::runtime_error( "Unexpected value for variable " + result[1].specification().name() );
      }
    }
    if ( cursor.next() )
      throw std::runtime_error( "Unexpected number of returned rows" );
    delete query;
    session->transaction().commit();
    session->endUserSession();
    delete session;
    session=0;
    // Disconnecting
    std::cout << "Disconnecting..." << std::endl;
    connection->disconnect();
    delete connection;
  }catch ( coral::Exception& se ) {
    std::cerr << "CORAL Exception : " << se.what() << std::endl;
    return 1;
  }catch ( std::exception& e ) {
    std::cerr << "Standard C++ exception : " << e.what() << std::endl;
    return 1;
  }catch ( ... ) {
    std::cerr << "Exception caught (...)" << std::endl;
    return 1;
  }
}
