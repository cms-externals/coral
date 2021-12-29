#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ITableDataEditor.h"

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
    //
    // create table t1
    //
    session->nominalSchema().dropIfExistsTable( "t1" );
    std::cout<<"creating table t1"<<std::endl;
    coral::TableDescription desc1;
    desc1.setName("t1");
    desc1.insertColumn( "id",coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    desc1.insertColumn( "first", coral::AttributeSpecification::typeNameForId(typeid(std::string) ) );
    desc1.insertColumn( "datacol", coral::AttributeSpecification::typeNameForId(typeid(std::string) ) );
    desc1.setPrimaryKey( "datacol", "" );
    coral::ITable& t1 = session->nominalSchema().createTable(desc1);
    std::cout<<"table t1 created"<<std::endl;

    //
    // create table t2
    //
    std::cout<<"creating table t2"<<std::endl;
    session->nominalSchema().dropIfExistsTable( "t2" );
    coral::TableDescription desc2;
    desc2.setName("t2");
    desc2.insertColumn( "id2", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    desc2.insertColumn( "col1", coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    desc2.insertColumn( "col2", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    desc2.insertColumn( "col3", coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    desc2.insertColumn( "col4", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
    desc2.insertColumn( "col5", coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    bool isUnique=false;
    std::vector<std::string> nonuniqueidx;
    nonuniqueidx.push_back("col3");
    nonuniqueidx.push_back("col4");
    desc2.createIndex( "t2_idxNU", nonuniqueidx ,isUnique);
    isUnique=true;
    std::vector<std::string> uniqueidx;
    uniqueidx.push_back("col1");
    uniqueidx.push_back("col5");
    desc2.createIndex( "t2_idxU", uniqueidx ,isUnique);
    coral::ITable& t2 = session->nominalSchema().createTable( desc2 );
    session->transaction().commit();
    std::cout<<"table t2 created"<<std::endl;
    //
    // change t1 schema
    //
    session->transaction().start(false);
    std::cout<<"adding column to t1"<<std::endl;
    t1.schemaEditor().insertColumn( "extra", coral::AttributeSpecification::typeNameForId(typeid(short) ) );
    std::cout<<"setting null constraint to first"<<std::endl;
    t1.schemaEditor().setNotNullConstraint( "first" );
    std::cout<<"setting unique constraint to first"<<std::endl;
    t1.schemaEditor().setUniqueConstraint( "first" );
    std::cout<<"change column type id"<<std::endl;
    t1.schemaEditor().changeColumnType("id",coral::AttributeSpecification::typeNameForId(typeid(unsigned int) ) );
    session->transaction().commit();
    std::cout<<"column added to t1"<<std::endl;

    //
    // change t2 schema
    //
    session->transaction().start(false);
    std::cout<<"[OVAL] rename column from t2"<<std::endl;
    t2.schemaEditor().renameColumn("col1","newcol1");
    std::cout<<"[OVAL] drop column from t2"<<std::endl;
    t2.schemaEditor().dropColumn( "col2" );
    std::cout<<"[OVAL] insert column col2 back to t2"<<std::endl;
    t2.schemaEditor().insertColumn( "col2", coral::AttributeSpecification::typeNameForId(typeid(int) ) );
    session->transaction().commit();
    std::cout<<"[OVAL] column changed for t2"<<std::endl;
    session->endUserSession();


    session->startUserSession( userName, password );

    coral::TableDescription depDesc;
    depDesc.setName( "CORALTEST_DEP" );
    depDesc.insertColumn( "DEP_ID", "int", 0, false );
    depDesc.insertColumn( "DEP_NAME", "string", 255, false );
    depDesc.setPrimaryKey( "DEP_ID" );

    coral::TableDescription empDesc;
    empDesc.setName( "CORALTEST_EMP" );
    empDesc.insertColumn( "EMP_ID", "int", 0, false );
    empDesc.insertColumn( "EMP_NAME", "string", 255, false );
    empDesc.insertColumn( "EMP_DEPID", "int", 0, false );
    empDesc.setPrimaryKey( "EMP_ID" );

    std::string fk1Name = "CORALTEST_EMP_DEPID_FK";
    std::string fk1Tgt = depDesc.name();
    std::vector<std::string> fk1ColumnsSrc;
    fk1ColumnsSrc.push_back( "EMP_DEPID" );
    std::vector<std::string> fk1ColumnsTgt;
    fk1ColumnsTgt.push_back( "DEP_ID" );

    // Block 1
    std::cout<<"Block 1"<<std::endl;
    session->transaction().start( false ); // read-write
    try {
      session->nominalSchema().dropIfExistsTable( empDesc.name() );
      session->nominalSchema().dropIfExistsTable( depDesc.name() );
      session->nominalSchema().createTable( depDesc );
      session->nominalSchema().createTable( empDesc );
      coral::ITable& empTable =
        session->nominalSchema().tableHandle( empDesc.name() );
      coral::ITableSchemaEditor& empTSE = empTable.schemaEditor();
      std::cout<<"about to create key "<<std::endl;
      empTSE.createForeignKey( fk1Name, fk1ColumnsSrc, fk1Tgt, fk1ColumnsTgt );
      std::cout<<"about to drop key 0"<<std::endl;
      empTSE.dropForeignKey( fk1Name );
      std::cout<<"about to recreate key 0"<<std::endl;
      empTSE.createForeignKey( fk1Name, fk1ColumnsSrc, fk1Tgt, fk1ColumnsTgt );
      session->transaction().commit();
    } catch ( coral::InvalidForeignKeyIdentifierException& ) {
      session->transaction().rollback();
    } catch ( ... ) {
      session->transaction().rollback();
      throw;
    }

    // Block 2
    std::cout<<"Block 2"<<std::endl;
    session->transaction().start( false ); // read-write
    try {
      coral::ITable& empTable =
        session->nominalSchema().tableHandle( empDesc.name() );
      coral::ITableSchemaEditor& empTSE = empTable.schemaEditor();
      std::cout<<"about to drop key 1"<<std::endl;
      empTSE.dropForeignKey( fk1Name );
      std::cout<<"dropped key 1"<<std::endl;
      empTSE.createForeignKey( fk1Name, fk1ColumnsSrc, fk1Tgt, fk1ColumnsTgt );
      std::cout<<"recreated key 1"<<std::endl;
      session->transaction().commit();
    } catch ( coral::InvalidForeignKeyIdentifierException& ) {
      session->transaction().rollback();
    } catch ( ... ) {
      session->transaction().rollback();
      throw;
    }
    session->endUserSession();
    session->startUserSession( userName, password );
    // Block 3
    std::cout<<"Block 3"<<std::endl;
    session->transaction().start( false ); // read-write
    try {
      coral::ITable& empTable =
        session->nominalSchema().tableHandle( empDesc.name() );
      coral::ITableSchemaEditor& empTSE = empTable.schemaEditor();
      std::cout<<"about to drop key"<<std::endl;
      empTSE.dropForeignKey( fk1Name );
      std::cout<<"foreign key dropped"<<std::endl;
      empTSE.createForeignKey( fk1Name, fk1ColumnsSrc, fk1Tgt, fk1ColumnsTgt );
      std::cout<<"foreign key recreated"<<std::endl;
      session->transaction().commit();
    } catch ( coral::InvalidForeignKeyIdentifierException& ) {
      std::cout<<"caught coral::InvalidForeignKeyIdentifierException"<<std::endl;
      session->transaction().rollback();
    } catch ( ... ) {
      session->transaction().rollback();
      throw;
    }
    // Block 4
    session->transaction().start( false ); // read-write
    try {
      {
        std::unique_ptr<coral::IQuery> query
          ( session->nominalSchema().newQuery() );
        query->addToTableList( "coral_sqlite_fk" );
        std::cout << std::endl;
        std::cout << "*** coral_sqlite_fk start" << std::endl;
        coral::ICursor& cursor = query->execute();
        while ( cursor.next() ) {
          cursor.currentRow().toOutputStream( std::cout ) << std::endl;
        }
        std::cout << "*** coral_sqlite_fk end" << std::endl;
      }
      std::cout << "DROP TABLES" << std::endl;
      session->nominalSchema().dropIfExistsTable( empDesc.name() );
      session->nominalSchema().dropIfExistsTable( depDesc.name() );
      {
        std::unique_ptr<coral::IQuery> query
          ( session->nominalSchema().newQuery() );
        query->addToTableList( "coral_sqlite_fk" );
        std::cout << std::endl;
        std::cout << "*** coral_sqlite_fk start" << std::endl;
        coral::ICursor& cursor = query->execute();
        while ( cursor.next() ) {
          cursor.currentRow().toOutputStream( std::cout ) << std::endl;
        }
        std::cout << "*** coral_sqlite_fk end" << std::endl;
      }
      session->transaction().commit();
    } catch ( ... ) {
      session->transaction().rollback();
      throw;
    }
    session->endUserSession();

    delete session;
    session=0;
    // Disconnecting
    std::cout << "Disconnecting..." << std::endl;
    connection->disconnect();
    delete connection;
  }catch ( coral::SchemaException& ce) {
    std::cerr <<"caught coral Exception : "<< ce.what() <<std::endl;
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
