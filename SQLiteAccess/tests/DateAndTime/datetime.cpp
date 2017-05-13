#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/RelationalServiceException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Date.h"
#include "CoralBase/Exception.h"
#include "CoralBase/TimeStamp.h"

#include "CoralKernel/Context.h"

#include <iostream>
#include <stdexcept>

int main()
{
  std::string con="sqlite_file:zhen.db";
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
    session->transaction().start(false);
    coral::ISchema& schema = session->nominalSchema();
    std::cout << "About to drop previous table" << std::endl;
    schema.dropIfExistsTable( "T" );
    std::cout << "Describing new table" << std::endl;
    coral::TableDescription description;
    description.setName( "T" );
    description.insertColumn( "ID",
                              coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description.setPrimaryKey( "ID" );
    description.insertColumn( "TheDate",
                              coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
    description.insertColumn( "TheTime",
                              coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ),
                              6 );

    std::cout << "About to create the table" << std::endl;
    coral::ITable& table = schema.createTable( description );
    coral::AttributeList rowBuffer;
    table.dataEditor().rowBuffer( rowBuffer );

    for ( int i = 0; i < 5; ++i )
    {
      rowBuffer["ID"].data<int>() = i;
      rowBuffer["TheDate"].data<coral::Date>() = coral::Date( 2005, 11, i+1 );
      rowBuffer["TheTime"].data<coral::TimeStamp>() = coral::TimeStamp::now();
      table.dataEditor().insertRow( rowBuffer );
    }
    coral::IBulkOperation* bulkInserter = table.dataEditor().bulkInsert( rowBuffer, 3 );

    long fraction = 111111111;
    for ( int i = 5; i < 10; ++i, fraction /= 10 )
    {
      rowBuffer["ID"].data<int>() = i;
      rowBuffer["TheDate"].data<coral::Date>() = coral::Date( 2005, 11, i+1 );
      coral::TimeStamp thisMoment = coral::TimeStamp::now();
      rowBuffer["TheTime"].data<coral::TimeStamp>() = coral::TimeStamp( thisMoment.year(),
                                                                        thisMoment.month(),
                                                                        thisMoment.day(),
                                                                        thisMoment.hour(),
                                                                        thisMoment.minute(),
                                                                        thisMoment.second(),
                                                                        (i-4) * fraction );
      bulkInserter->processNextIteration();
    }
    bulkInserter->flush();
    delete bulkInserter;
    session->transaction().commit();
    std::cout<<"table created"<<std::endl;
    session->transaction().start( true );
    schema = session->nominalSchema();
    coral::IQuery* query = schema.tableHandle("T").newQuery();
    query->setRowCacheSize( 20 );
    coral::ICursor& cursor = query->execute();
    int j = 0;
    while ( cursor.next() ) {
      const coral::AttributeList& currentRow = cursor.currentRow();
      currentRow.toOutputStream( std::cout ) << std::endl;
      ++j;
    }
    if ( j != 10 )
      throw std::runtime_error( "Unexpected number of rows" );
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
  return 0;
}
