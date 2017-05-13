#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/RelationalServiceException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Exception.h"

#include "CoralKernel/Context.h"

#include <iostream>
#include <stdexcept>

int main( int, char** )
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
    session->transaction().start(false);

    // Droping old tables
    std::cout << "Deleting old tables" << std::endl;
    session->nominalSchema().dropIfExistsTable( "Personnel" );
    session->nominalSchema().dropIfExistsTable( "Offices" );
    session->nominalSchema().dropIfExistsTable( "Departments" );

    // Creating the first table.
    std::cout << "Creating table \"Departments\"" << std::endl;
    coral::TableDescription description0;
    description0.setName( "Departments" );
    description0.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description0.insertColumn( "Name", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    description0.setNotNullConstraint( "id" );
    description0.setNotNullConstraint( "Name" );
    description0.setUniqueConstraint( "Name" );
    description0.setPrimaryKey( std::vector< std::string >( 1, "id" ) );
    coral::ITable& tableDepartments = session->nominalSchema().createTable(description0 );

    // Filling the table with two rows
    std::cout << "Filling table \"Departments\"" << std::endl;
    coral::AttributeList data1;
    tableDepartments.dataEditor().rowBuffer(data1);
    data1["id"].data<int>()=1 ;
    data1["Name"].data<std::string>()="Dep1";
    tableDepartments.dataEditor().insertRow( data1 ) ;

    data1["id"].data<int>()=2;
    data1["Name"].data<std::string>()= "Dep2" ;
    tableDepartments.dataEditor().insertRow( data1 ) ;

    // Creating the second table.
    std::cout << "Creating table \"Offices\"" << std::endl;
    coral::TableDescription description2;
    description2.setName( "Offices" );
    description2.insertColumn( "id", coral::AttributeSpecification::typeNameForId(typeid(int) ) );
    description2.insertColumn( "Number_of_tables", coral::AttributeSpecification::typeNameForId(typeid(int) ) );
    description2.insertColumn( "Orientation", coral::AttributeSpecification::typeNameForId(typeid(std::string) ) );
    description2.insertColumn( "Department", coral::AttributeSpecification::typeNameForId(typeid(int) )  );
    description2.setNotNullConstraint( "id" );
    description2.setNotNullConstraint( "Number_of_tables" );
    description2.setNotNullConstraint( "Orientation" );
    description2.setNotNullConstraint( "Department" );
    description2.setPrimaryKey( std::vector< std::string >( 1, "id" ) );
    description2.createForeignKey( "Offices_FK",
                                   std::vector< std::string >( 1, "Department" ),
                                   "Departments",
                                   std::vector< std::string >( 1, "id" ) );
    coral::ITable& tableOffices = session->nominalSchema().createTable( description2 );

    // Filling the table with several rows
    std::cout << "Filling table \"Offices\"" << std::endl;
    coral::AttributeList data2;
    tableOffices.dataEditor().rowBuffer(data2);
    data2["id"].data<int>()= 1 ;
    data2["Number_of_tables"].data<int>()=2 ;
    data2["Orientation"].data<std::string>()= "North" ;
    data2["Department"].data<int>()= 1 ;
    tableOffices.dataEditor().insertRow( data2 );

    data2["id"].data<int>()= 2 ;
    data2["Number_of_tables"].data<int>()= 1 ;
    data2["Orientation"].data<std::string>()= "South" ;
    data2["Department"].data<int>()= 1 ;
    tableOffices.dataEditor().insertRow( data2 );

    data2["id"].data<int>()=3;
    data2["Number_of_tables"].data<int>()=1;
    data2["Orientation"].data<std::string>()= "East" ;
    data2["Department"].data<int>()=2;
    tableOffices.dataEditor().insertRow( data2 );

    data2["id"].data<int>()=4 ;
    data2["Number_of_tables"].data<int>()=1;
    data2["Orientation"].data<std::string>()= "West" ;
    data2["Department"].data<int>()=2;
    tableOffices.dataEditor().insertRow( data2 ) ;

    data2["id"].data<int>()=5 ;
    data2["Number_of_tables"].data<int>()=2;
    data2["Orientation"].data<std::string>()="South" ;
    data2["Department"].data<int>()=2 ;
    tableOffices.dataEditor().insertRow( data2 ) ;

    // Creating the third table.
    std::cout << "Creating table \"Personnel\"" << std::endl;
    coral::TableDescription description3;
    description3.setName("Personnel");
    description3.insertColumn( "Name", coral::AttributeSpecification::typeNameForId(typeid(std::string) ) );
    description3.insertColumn( "Department", coral::AttributeSpecification::typeNameForId(typeid(int) )  );
    description3.insertColumn( "Office", coral::AttributeSpecification::typeNameForId(typeid(int) )  );
    description3.setNotNullConstraint( "Name" );
    description3.setUniqueConstraint( "Name" );
    description3.setNotNullConstraint( "Department" );
    description3.setNotNullConstraint( "Office" );
    description3.createForeignKey( "Personnel_Department",
                                   std::vector< std::string >( 1, "Department" ),
                                   "Departments",
                                   std::vector< std::string >( 1, "id" ) );
    description3.createForeignKey( "Personnel_Office",
                                   std::vector< std::string >( 1, "Office" ),
                                   "Offices",
                                   std::vector< std::string >( 1, "id" ) );

    coral::ITable& tablePersonnel = session->nominalSchema().createTable( description3 );
    std::cout << "Filling table \"Personnel\"" << std::endl;
    coral::AttributeList data3;
    tablePersonnel.dataEditor().rowBuffer(data3);
    data3["Name"].data<std::string>()= "Person1" ;
    data3["Department"].data<int>()=1;
    data3["Office"].data<int>()=1;
    tablePersonnel.dataEditor().insertRow( data3 );

    data3["Name"].data<std::string>()="Person2" ;
    data3["Department"].data<int>()=1;
    data3["Office"].data<int>()=1;
    tablePersonnel.dataEditor().insertRow( data3 );

    data3["Name"].data<std::string>()="Person3";
    data3["Department"].data<int>()=1;
    data3["Office"].data<int>()=2;
    tablePersonnel.dataEditor().insertRow( data3 ) ;

    data3["Name"].data<std::string>()="Person4";
    data3["Department"].data<int>()=2;
    data3["Office"].data<int>()=3 ;
    tablePersonnel.dataEditor().insertRow( data3 ) ;

    data3["Name"].data<std::string>()="Person5";
    data3["Department"].data<int>()=2;
    data3["Office"].data<int>()=4 ;
    tablePersonnel.dataEditor().insertRow( data3 ) ;

    data3["Name"].data<std::string>()="Person6" ;
    data3["Department"].data<int>()=2;
    data3["Office"].data<int>()=5;
    tablePersonnel.dataEditor().insertRow( data3 ) ;

    data3["Name"].data<std::string>()="Person7";
    data3["Department"].data<int>()=2 ;
    data3["Office"].data<int>()=5;
    tablePersonnel.dataEditor().insertRow( data3 ) ;

    session->transaction().commit();

    // Start a transaction
    std::cout << "Starting a new transaction" << std::endl;
    session->transaction().start();
    coral::AttributeList emptyBindVariableList;
    // Performing a query
    std::cout << "Performing the query \"SELECT Offices.Orientation, Departments.Name FROM Offices, Departments WHERE Offices.Department = Departments.id ORDER BY Offices.Orientation\"" << std::endl;
    std::cout << "retrieving only the first three rows" << std::endl;
    coral::IQuery* query1= session->nominalSchema().newQuery();
    query1->addToOutputList( "Offices.Orientation");
    query1->addToOutputList( "Departments.Name");
    query1->addToTableList( "Offices");
    query1->addToTableList( "Departments");
    query1->setCondition( "Offices.Department = Departments.id", emptyBindVariableList );
    query1->addToOrderList( "Offices.Orientation" );
    query1->setRowCacheSize( 10 );
    query1->limitReturnedRows( 3 );
    coral::ICursor& cursor1 = query1->execute();
    unsigned int s=0;
    while( cursor1.next() ) {
      const coral::AttributeList& row = cursor1.currentRow();
      row.toOutputStream( std::cout ) << std::endl;
      ++s;
    }
    cursor1.close();
    std::cout <<s<< " row(s) selected." << std::endl;
    delete query1;
    std::cout<<" Performing a query with a subquery"<<std::endl;
    std::cout << "Performing the query \"SELECT Personnel.Name, SelectedOffices.Orientation FROM ( SELECT Offices.Orientation, Offices.id AS office_id FROM Offices, Departments WHERE Offices.Department = Departments.id AND Departments.Name = :depname ) SelectedOffices, Personnel WHERE Personnel.Office = SelectedOffices.office_id ORDER BY SelectedOffices.Orientation\"" << std::endl;
    std::cout << "with depname = \"Dep2\"" << std::endl;
    coral::IQuery* query2=session->nominalSchema().newQuery();
    coral::IQueryDefinition& subquery = query2->defineSubQuery( "SelectedOffices" );
    subquery.addToOutputList( "Offices.Orientation","Orientation" );
    subquery.addToOutputList( "Offices.id", "office_id" );
    subquery.addToTableList( "Offices" );
    subquery.addToTableList( "Departments" );
    coral::AttributeList bindVariableList;
    bindVariableList.extend("depname", typeid(std::string));
    bindVariableList["depname"].data<std::string>()= "Dep2";
    subquery.setCondition( "Offices.Department = Departments.id AND Departments.Name = :depname", bindVariableList );
    query2->addToOutputList( "Personnel.Name" );
    query2->addToOutputList( "SelectedOffices.Orientation" );
    query2->addToTableList( "SelectedOffices" );
    query2->addToTableList( "Personnel" );
    query2->setCondition( "Personnel.Office = SelectedOffices.office_id", emptyBindVariableList );
    query2->addToOrderList( "SelectedOffices.Orientation" );
    query2->setRowCacheSize( 10 );
    coral::ICursor& cursor2 = query2->execute();
    s=0;
    while( cursor2.next() ) {
      const coral::AttributeList& row = cursor2.currentRow();
      row.toOutputStream( std::cout ) << std::endl;
      ++s;
    }
    cursor2.close();
    std::cout <<s << " row(s) selected." << std::endl;
    delete query2;
    // Committing the transaction
    std::cout << "Committing..." << std::endl;
    session->transaction().commit();

    // Start a transaction
    std::cout << "Starting a new transaction" << std::endl;
    session->transaction().start();
    //coral::AttributeList emptyBindVariableList;
    // Performing a query
    std::cout << "Performing the query \"SELECT o.Orientation, d.Name FROM Offices AS o, Departments AS d WHERE o.Department = d.id ORDER BY o.Orientation\"" << std::endl;
    std::cout << "retrieving only the first three rows" << std::endl;
    coral::IQuery* query3( session->nominalSchema().newQuery() );
    query3->addToOutputList( "o.Orientation");
    query3->addToOutputList( "d.Name");
    query3->addToTableList( "Offices","o");
    query3->addToTableList( "Departments","d");
    query3->setCondition( "o.Department=d.id", emptyBindVariableList );
    query3->addToOrderList( "o.Orientation" );
    query3->setRowCacheSize( 10 );
    query3->limitReturnedRows( 3 );
    coral::ICursor& cursor3 = query3->execute();
    s=0;
    while( cursor3.next() ) {
      const coral::AttributeList& row = cursor3.currentRow();
      row.toOutputStream( std::cout ) << std::endl;
      ++s;
    }
    cursor3.close();
    std::cout << s<< " row(s) selected." << std::endl;
    delete query3;

    std::cout<<" Performing a query with a subquery"<<std::endl;
    std::cout << "Performing the query \"SELECT Personnel.Name, SelectedOffices.Orientation FROM ( SELECT o.Orientation, o.id AS office_id FROM Offices AS o, Departments AS d WHERE o.Department = d.id AND d.Name = :depname ) SelectedOffices, Personnel WHERE Personnel.Office = SelectedOffices.office_id ORDER BY SelectedOffices.Orientation\"" << std::endl;
    std::cout << "with depname = \"Dep2\"" << std::endl;
    coral::IQuery* query4( session->nominalSchema().newQuery() );
    coral::IQueryDefinition& subquery2 = query4->defineSubQuery( "SelectedOffices" );
    subquery2.addToOutputList( "o.Orientation","Orientation" );
    subquery2.addToOutputList( "o.id", "office_id" );
    subquery2.addToTableList( "Offices","o" );
    subquery2.addToTableList( "Departments","d" );
    coral::AttributeList bindVariableList2;
    bindVariableList2.extend("depname", typeid(std::string) ) ;
    bindVariableList2["depname"].data<std::string>()= "Dep2" ;
    subquery2.setCondition( "o.Department = d.id AND d.Name = :depname", bindVariableList2 );
    query4->addToOutputList( "Personnel.Name" );
    query4->addToOutputList( "SelectedOffices.Orientation" );
    query4->addToTableList( "SelectedOffices" );
    query4->addToTableList( "Personnel" );
    query4->setCondition( "Personnel.Office = SelectedOffices.office_id", emptyBindVariableList );
    query4->addToOrderList( "SelectedOffices.Orientation" );
    query4->setRowCacheSize( 10 );
    coral::ICursor& cursor4 = query4->execute();
    s=0;
    while( cursor4.next() ) {
      const coral::AttributeList& row = cursor4.currentRow();
      row.toOutputStream( std::cout ) << std::endl;
      ++s;
    }
    cursor4.close();
    std::cout <<s << " row(s) selected." << std::endl;
    delete query4;
    session->transaction().commit();
    session->endUserSession();
    delete session;
    session=0;
    // Disconnecting
    std::cout << "Disconnecting..." << std::endl;
    connection->disconnect();
    delete connection;
  }
  catch ( coral::Exception& se ) {
    std::cerr << "CORAL Exception : " << se.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << "Standard C++ exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Exception caught (...)" << std::endl;
    return 1;
  }
  return 0;
}
