#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITablePrivilegeManager.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Exception.h"

#include "CoralKernel/Context.h"

#include <iostream>

int main( int, char** )
{
  std::string con="sqlite_file:zhen.db";

  try
  {
    coral::Context& ctx = coral::Context::instance();

    ctx.loadComponent( "CORAL/RelationalPlugins/sqlite" );

    coral::IHandle<coral::IRelationalDomain> iHandle=ctx.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/sqlite" );

    if ( ! iHandle.isValid() )
    {
      std::cerr<<"non existing domain "<<std::endl;
    }

    std::pair<std::string, std::string> connectionAndSchema = iHandle->decodeUserConnectionString( con );
    coral::IConnection*  connection=iHandle->newConnection( connectionAndSchema.first );
    connection->connect();
    coral::ISession* session=connection->newSession(connectionAndSchema.second);
    std::string userName, password;
    session->startUserSession( userName, password );

    session->transaction().start(false);
    coral::ISchema& schema = session->nominalSchema();
    std::cout << "About to drop previous tables and views" << std::endl;
    schema.dropIfExistsView( "VT" );
    schema.dropIfExistsTable( "T" );
    std::cout << "Creating a new table" << std::endl;
    coral::TableDescription description;
    description.setName( "T" );
    description.insertColumn( "ID",coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description.setPrimaryKey( "ID" );
    description.insertColumn( "x",coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    description.setNotNullConstraint( "x" );
    description.insertColumn( "Y",coral::AttributeSpecification::typeNameForId( typeid(double) ) );
    coral::ITable& table=schema.createTable( description );
    coral::AttributeList itemtoInsert;
    coral::ITableDataEditor& teditor=table.dataEditor();
    teditor.rowBuffer( itemtoInsert );
    for ( int i = 0; i < 10; ++i ) {
      itemtoInsert["ID"].data<int>()=i;
      itemtoInsert["x"].data<float>() = i * (float)1.5;
      itemtoInsert["Y"].data<double>()=i*3.14159;
      teditor.insertRow( itemtoInsert );
    }
    std::cout << "About to create a view" << std::endl;
    coral::IViewFactory* factory = schema.viewFactory();
    factory->addToOutputList( "tt.ID", "id" );
    factory->addToOutputList( "tt.x", "x" );
    factory->addToOutputList( "tt.Y", "y" );
    factory->addToTableList( "T", "tt" );
    factory->setCondition( "ID > 2", coral::AttributeList() );
    factory->create( "VT" ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
    delete factory;
    session->transaction().commit();

    session->transaction().start( true );
    coral::IView& view = session->nominalSchema().viewHandle( "VT" );
    std::cout << "View definition : " << view.definition() << std::endl;

    int numberOfColumns = view.numberOfColumns();
    for ( int i = 0; i < numberOfColumns; ++i ) {
      const coral::IColumn& column = view.column( i );
      std::cout << "  " << column.name() << " (" << column.type() << ")";
      ///!!sqlite view lost all the constrains of the source table
      if ( column.isNotNull() ) std::cout << " NOT NULL";
      std::cout << std::endl;
    }
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
  }catch ( ... ) {
    std::cerr << "Exception caught (...)" << std::endl;
    return 1;
  }
  return 0;
}
