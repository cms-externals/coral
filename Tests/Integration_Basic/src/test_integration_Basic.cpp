#include <cstdio>
#include <iostream>
#include <string>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IBulkOperationWithQuery.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IOperationWithQuery.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/IQueryDefinition.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

#include "TestEnv/TestEnv.h"
#include "TestEnv/Testing.h"

//-----------------------------------------------------------------------------

class GroupApp : public Testing
{
public:
  GroupApp( const TestEnv& env );
  virtual ~GroupApp();
  void readData();
  void fillData();
};

GroupApp::GroupApp( const TestEnv& env )
: Testing(env)
{
}

//-----------------------------------------------------------------------------

GroupApp::~GroupApp()
{
}

//-----------------------------------------------------------------------------

void
GroupApp::fillData()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();

  std::cout << "About to drop previous table" << std::endl;
  schema.dropIfExistsTable( T3 );

  std::cout << "Describing new table" << std::endl;
  coral::TableDescription description;
  description.setName( T3 );
  description.insertColumn( "ID",
                            coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description.setPrimaryKey( "ID" );
  description.insertColumn( "x",
                            coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description.setNotNullConstraint( "x" );
  description.insertColumn( "GrB",
                            coral::AttributeSpecification::typeNameForId( typeid(int) ) );

  std::cout << "About to create the table" << std::endl;
  coral::ITable& table = schema.createTable( description );

  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );


  coral::AttributeList rowBuffer;
  rowBuffer.extend<int>( "ID" );
  rowBuffer.extend<float>( "x" );
  rowBuffer.extend<int>( "GrB" );

  for ( int i = 0; i < 100; ++i )
  {
    rowBuffer["ID"].data<int>() = i;
    rowBuffer["GrB"].data<int>() = i%10;
    rowBuffer["x"].data<float>() = (float)( (i%14) + 0.1 * (i%17) );

    table.dataEditor().insertRow( rowBuffer );
  }

  coral::sleepSeconds( 1 );

  session.transaction().commit();
}

//-----------------------------------------------------------------------------

void
GroupApp::readData()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  coral::ISchema& schema = session.nominalSchema();

  coral::IQuery* query = schema.tableHandle( T3 ).newQuery();

  coral::AttributeList rowBuffer;
  rowBuffer.extend<int>( "GrB" );
  rowBuffer.extend<float>( "RES" );

  query->addToOutputList( "GrB" );
  query->addToOrderList( "GrB" );
  query->addToOutputList( "AVG(x)", "RES" );
  query->groupBy( "GrB" );

  query->defineOutput( rowBuffer );

  coral::ICursor& cursor = query->execute();

  int i = 0;
  while ( cursor.next() )
  {
    rowBuffer.toOutputStream( std::cout );
    std::cout << std::endl;
    if ( i != rowBuffer["GrB"].data<int>() )
      throw std::runtime_error( "Unexpected data" );
    ++i;
  }

  if ( i != 10 )
    throw std::runtime_error( "Unexpected number of rows returned" );

  delete query;

  session.transaction().commit();
}

//-----------------------------------------------------------------------------

class Transaction : public Testing
{
public:
  Transaction(const TestEnv& env);
  void run();
};

Transaction::Transaction(const TestEnv& env)
  : Testing(env)
{
}

void Transaction::run()
{
  coral::ISessionProxy* session01 = newSession(0, coral::ReadOnly, false);
  // Start a readonly session and check the transaction modes
  session01->transaction().start(true);
  session01->transaction().commit();
  // Test update transaction - should throw coral::InvalidOperationInReadOnlyModeException
  try
  {
    session01->transaction().start(false);
    throw std::runtime_error( "Was able to start update transaction on read only session" );
  }
  catch (coral::InvalidOperationInReadOnlyModeException&)
  {
    // cool... let's continue
  }
  delete session01;
  // start an update session and check the transaction modes
  coral::ISessionProxy* session02 = newSession(0, coral::Update, false);
  // Test readonly transaction - should pass
  session02->transaction().start(true);
  session02->transaction().commit();
  // Test update transaction - should pass
  session02->transaction().start(false);
  session02->transaction().commit();

  delete session02;
}

//-----------------------------------------------------------------------------

class SetOperations : public Testing
{
private:
  std::string T4;
public:
  SetOperations( const TestEnv& env);
  ~SetOperations();
  void run();
};

SetOperations::SetOperations( const TestEnv& env)
  : Testing(env)
{
  env.addTablePrefix(T4, "T4");
}

SetOperations::~SetOperations()
{
}

void
SetOperations::run()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  coral::ISchema& workingSchema = session.nominalSchema();

  std::cout << " Query 1:" << std::endl;
  coral::IQuery* query1 = workingSchema.newQuery();
  query1->addToTableList( T2 );
  coral::IQueryDefinition& rhs1 = query1->applySetOperation( coral::IQueryDefinition::Union );
  rhs1.addToTableList( T4 );
  query1->setRowCacheSize( 100 );
  int nRows = 0;
  coral::ICursor& cursor1 = query1->execute();
  while ( cursor1.next() ) {
    cursor1.currentRow().toOutputStream( std::cout ) << std::endl;
    ++nRows;
  }
  delete query1;
  if ( nRows != 28 )
    throw std::runtime_error( "Unexpected number of rows" );

  // At the moment MINUS and INTERSECT only work on Oracle databases
  if ( m_env.getUrlRW() == m_env.BuildUrl( "Oracle", false ) )
  {
    std::cout << " Query 2:" << std::endl;
    coral::IQuery* query2 = workingSchema.newQuery();
    query2->addToTableList( T4 );
    coral::IQueryDefinition& rhs2 = query2->applySetOperation( coral::IQueryDefinition::Minus );
    rhs2.addToTableList( T2 );
    nRows = 0;
    coral::ICursor& cursor2 = query2->execute();
    while ( cursor2.next() ) {
      cursor2.currentRow().toOutputStream( std::cout ) << std::endl;
      ++nRows;
    }
    delete query2;
    if ( nRows != 3 )
      throw std::runtime_error( "Unexpected number of rows" );


    std::cout << " Query 3:" << std::endl;
    coral::IQuery* query3 = workingSchema.newQuery();
    query3->addToTableList( T2 );
    coral::IQueryDefinition& rhs3 = query3->applySetOperation( coral::IQueryDefinition::Minus );
    rhs3.addToTableList( T4 );
    nRows = 0;
    coral::ICursor& cursor3 = query3->execute();
    while ( cursor3.next() ) {
      cursor3.currentRow().toOutputStream( std::cout ) << std::endl;
      ++nRows;
    }
    delete query3;
    if ( nRows != 12 )
      throw std::runtime_error( "Unexpected number of rows" );

    std::cout << " Query 4:" << std::endl;
    coral::IQuery* query4 = workingSchema.newQuery();
    query4->addToTableList( T2 );
    coral::IQueryDefinition& rhs4 = query4->applySetOperation( coral::IQueryDefinition::Intersect );
    rhs4.addToTableList( T4 );
    nRows = 0;
    coral::ICursor& cursor4 = query4->execute();
    while ( cursor4.next() ) {
      cursor4.currentRow().toOutputStream( std::cout ) << std::endl;
      ++nRows;
    }
    delete query4;
    if ( nRows != 13 )
      throw std::runtime_error( "Unexpected number of rows" );


    std::cout << " Query 5:" << std::endl;
    coral::IQuery* query5 = workingSchema.newQuery();

    coral::IQueryDefinition& sub = query5->defineSubQuery( "A" );
    sub.addToTableList( T2 );
    sub.setCondition( "id>10", coral::AttributeList() );
    coral::IQueryDefinition& rhs5 = sub.applySetOperation( coral::IQueryDefinition::Intersect );
    rhs5.addToTableList( T4 );

    query5->addToTableList( "A", "B" );
    query5->addToOrderList( "ty-tx" );
    query5->limitReturnedRows( 4 );
    nRows = 0;
    coral::ICursor& cursor5 = query5->execute();
    while ( cursor5.next() ) {
      cursor5.currentRow().toOutputStream( std::cout ) << std::endl;
      ++nRows;
    }
    delete query5;
    if ( nRows != 4 )
      throw std::runtime_error( "Unexpected number of rows" );

  }

  session.transaction().commit();

}

//-----------------------------------------------------------------------------

class InsertSelect : public Testing
{
private:
  std::string T4;
public:
  InsertSelect(const TestEnv& env);
  ~InsertSelect();
  void run();
};

InsertSelect::InsertSelect(const TestEnv& env) : Testing(env)
{
  env.addTablePrefix(T4, "T4");
}

InsertSelect::~InsertSelect()
{
}


void
InsertSelect::run()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& workingSchema = session.nominalSchema();
  workingSchema.dropIfExistsTable(T4);
  coral::TableDescription description( workingSchema.tableHandle(T2).description() );
  description.setName(T4);
  coral::ITable& table = workingSchema.createTable( description );
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  coral::IOperationWithQuery* operation = table.dataEditor().insertWithQuery();
  coral::IQueryDefinition& query = operation->query();
  query.addToTableList(T2);
  query.setCondition( "id<10", coral::AttributeList() );
  long numberOfRowsInserted = operation->execute();
  delete operation;
  if ( numberOfRowsInserted != 6 )
    throw std::runtime_error( "Unexpected number of rows inserted." );

  coral::IBulkOperationWithQuery* bulkOperation = table.dataEditor().bulkInsertWithQuery( 3 );
  coral::IQueryDefinition& queryb = bulkOperation->query();
  queryb.addToTableList(T2);
  coral::AttributeList condition;
  condition.extend<long>( "idmin");
  condition.extend<long>( "idmax");
  queryb.setCondition( "id>:idmin AND id<:idmax", condition );
  long& idmin = condition[0].data<long>();
  long& idmax = condition[1].data<long>();

  idmin = 10;
  idmax = 13;
  for ( int i = 0; i < 5; ++i ) {
    bulkOperation->processNextIteration();
    idmin += 4;
    idmax += 4;
  }
  bulkOperation->flush();
  delete bulkOperation;

  // add three more rows (for the next tests to make better sense)
  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );

  if ( rowBuffer[0].specification().type() == typeid(long long)) {
    rowBuffer[0].data<long long>() = 100;
    rowBuffer[1].data<float>() = 200;
    rowBuffer[2].data<double>() = 300;
    for ( int i = 0; i < 3; ++i ) {
      table.dataEditor().insertRow( rowBuffer );
      rowBuffer[0].data<long long>() += 1;
      rowBuffer[1].data<float>() += 2;
      rowBuffer[2].data<double>() += 3;
    }
  }else{
    rowBuffer[0].data<long>() = 100;
    rowBuffer[1].data<float>() = 200;
    rowBuffer[2].data<double>() = 300;
    for ( int i = 0; i < 3; ++i ) {
      table.dataEditor().insertRow( rowBuffer );
      rowBuffer[0].data<long>() += 1;
      rowBuffer[1].data<float>() += 2;
      rowBuffer[2].data<double>() += 3;
    }
  }
  session.transaction().commit();
  coral::sleepSeconds( 1 );
}

//-----------------------------------------------------------------------------

class Queries : public Testing
{
public:
  Queries( const TestEnv& env);
  ~Queries();
  void run();
};

Queries::Queries( const TestEnv& env) : Testing(env)
{
}


Queries::~Queries()
{
}


void
Queries::run()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  coral::ISchema& workingSchema = session.nominalSchema();

  // Run a query on a table
  coral::IQuery* query0 = workingSchema.tableHandle(T1).newQuery();
  query0->addToOutputList( "Data" );
  query0->addToOutputList( "Z" );
  query0->addToOrderList( "id" );
  coral::ICursor& cursor0 = query0->execute();
  int i = 0;
  while ( cursor0.next() ) {
    std::cout << "Row " << i + 1 << std::endl;
    const coral::Blob& blob = cursor0.currentRow()[0].data<coral::Blob>();
    int expectedBlobSize = 1000 * ( i + 1 );
    if ( blob.size() != expectedBlobSize ) {
      std::cerr << "Error : returned blob size = " << blob.size() << " while expected is " << expectedBlobSize << std::endl;
      delete query0;
      throw std::runtime_error( "Unexpected blob data" );
    }
    const unsigned char* p = static_cast<const unsigned char*>( blob.startingAddress() );
    for ( int j = 0; j < expectedBlobSize; ++j, ++p ) {
      if ( *p != ( i + j )%256 ) {
        delete query0;
        throw std::runtime_error( "Unexpected blob data" );
      }
    }

    bool nullNess = ( i%4 == 2 ) ? true : false;
    if ( cursor0.currentRow()[1].isNull() != nullNess ) {
      delete query0;
      throw std::runtime_error( "Unexpected null-ness" );
    }
    ++i;
  }
  delete query0;
  if ( i != 7 ) throw std::runtime_error( "Unexpected number of returned rows" );

  // Run a wildcard query on the table
  coral::IQuery* query1 = workingSchema.newQuery();
  query1->addToTableList(T3);
  coral::ICursor& cursor1 = query1->execute();
  while ( cursor1.next() ) {
    cursor1.currentRow().toOutputStream( std::cout ) << std::endl;
  }
  delete query1;

  // Run a wildcard query on the view
  coral::IQuery* query2 = workingSchema.newQuery();
  query2->addToTableList(V0);
  coral::ICursor& cursor2 = query2->execute();
  while ( cursor2.next() ) {
    cursor2.currentRow().toOutputStream( std::cout ) << std::endl;
  }
  delete query2;

  // Run a query on a table
  coral::IQuery* query3 = workingSchema.tableHandle(T3).newQuery();
  query3->addToOutputList( "id" );
  query3->addToOrderList( "Qn" );
  query3->setRowCacheSize( 100 );
  query3->limitReturnedRows( 2, 2 );
  coral::ICursor& cursor3 = query3->execute();
  int nRows = 0;
  while ( cursor3.next() ) {
    cursor3.currentRow().toOutputStream( std::cout ) << std::endl;
    ++nRows;
  }
  delete query3;
  if ( nRows != 2 )
    throw std::runtime_error( "Unexpected number of rows returned #1" );

  // Perform a query involving two tables.
  coral::IQuery* query4 = workingSchema.newQuery();
  query4->addToOutputList(T1 + ".X", "x" );
  query4->addToOutputList(T1 + ".t", "t" );
  query4->addToOutputList(T3 + ".Qn", "Qn" );
  query4->addToTableList(T1);
  query4->addToTableList(T3);
  std::string condition = T1 + ".id = " + T3 + ".fk1 AND " + T1 + ".Z<:zmax";
  coral::AttributeList conditionData;
  conditionData.extend<float>( "zmax" );
  query4->setCondition( condition, conditionData );
  query4->setMemoryCacheSize( 5 );
  conditionData[0].data<float>() = 3;
  coral::ICursor& cursor4 = query4->execute();
  nRows = 0;
  while ( cursor4.next() ) {
    cursor4.currentRow().toOutputStream( std::cout ) << std::endl;
    ++nRows;
  }
  delete query4;
  if ( nRows != 2 )
    throw std::runtime_error( "Unexpected number of rows returned #2" );

  // Perform a query involving a subquery.
  coral::IQuery* query5 = workingSchema.newQuery();
  query5->addToOutputList( "A.t", "t" );
  query5->addToOutputList( "B.tx", "tx" );

  coral::IQueryDefinition& subQuery1 = query5->defineSubQuery( "A" );
  subQuery1.addToOutputList( "t" );
  subQuery1.addToOutputList( "id" );
  subQuery1.addToTableList(T1);
  coral::AttributeList cond1;
  cond1.extend<float>( "xmin" );
  subQuery1.setCondition( "X < :xmin", cond1 );
  query5->addToTableList( "A" );

  coral::IQueryDefinition& subQuery2 = query5->defineSubQuery( "B" );
  subQuery2.addToOutputList( "id" );
  subQuery2.addToOutputList( "TX", "tx" );
  subQuery2.addToTableList(V0);
  coral::AttributeList cond2;
  cond2.extend<float>( "tymax" );
  subQuery2.setCondition( "TX < :tymax", cond2 );
  query5->addToTableList( "B" );

  query5->setCondition( "A.id = B.id", coral::AttributeList() );


  //for mysql exact float values doesn't work propper
  //it could fluctuate between +0.1 and -0.1
  //cond1[0].data<float>() = 1.005;

  cond1[0].data<float>() = (float)1.0049;
  cond2[0].data<float>() = (float)210;
  nRows = 0;
  coral::ICursor& cursor5 = query5->execute();
  while ( cursor5.next() ) {
    cursor5.currentRow().toOutputStream( std::cout ) << std::endl;
    ++nRows;
  }
  delete query5;
  if ( nRows != 3 )
    throw std::runtime_error( "Unexpected number of rows returned #3" );

  session.transaction().commit();

}

//-----------------------------------------------------------------------------

class SchemaDefinition : public Testing
{
public:
  SchemaDefinition(const TestEnv& env);
  void prepareSchema();
  void readSchema();
  void printViewInfo( const coral::IView& view ) const;
  void printTableInfo( const coral::ITableDescription& description ) const;
};

SchemaDefinition::SchemaDefinition(const TestEnv& env)
  : Testing(env)
{
}

void SchemaDefinition::prepareSchema()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();

  // Get rid of the previous tables and views
  schema.dropIfExistsView(V0);
  schema.dropIfExistsTable(T3);
  schema.dropIfExistsTable(T2);
  schema.dropIfExistsTable(T1);

  // Create the first table
  coral::TableDescription description1( "SchemaDefinition_Test" );
  description1.setName(T1);

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
  coral::ITable& table = schema.createTable( description1 );

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

  ///follwing was commented out of fialing for mysql plugin
  ///related to bug #36513

  //   table.schemaEditor().setUniqueConstraint( constraintColumns );

  /// Add a unique index
  table.schemaEditor().createIndex(T1 + "_IDX_t", "t", true );

  // Give privileges
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  // Create the second table
  coral::TableDescription description2( "SchemaDefinition_Test" );
  description2.setName(T2);
  description2.insertColumn( "id",
                             coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  description2.setUniqueConstraint( "id" );
  description2.setNotNullConstraint( "id" );
  description2.insertColumn( "tx",
                             coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description2.setNotNullConstraint( "tx" );
  description2.insertColumn( "ty",
                             coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  description2.setNotNullConstraint( "ty" );
  session.nominalSchema().createTable( description2 ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  // Create the third table
  coral::TableDescription description3( "SchemaDefinition_Test" );
  description3.setName(T3);
  description3.insertColumn( "id",
                             coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  description3.setPrimaryKey( "id" );
  description3.insertColumn( "fk1",
                             coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  description3.createForeignKey( T3 + "_FK1", "fk1", T1, "id" );
  description3.insertColumn( "fk2",
                             coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  description3.createForeignKey( T3 + "_FK2", "fk2", T2, "id" );
  description3.insertColumn( "Qn",
                             coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  session.nominalSchema().createTable( description3 ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  coral::sleepSeconds( 1 );

  session.transaction().commit();

  //oracle feature
  coral::sleepSeconds( 1 );

}

void
SchemaDefinition::printViewInfo( const coral::IView& view ) const
{
  int numberOfColumns = view.numberOfColumns();
  std::cout << "View " << view.name();
  std::cout << " has" << std::endl
            << "  " << numberOfColumns << " columns :" << std::endl;
  for ( int i = 0; i < numberOfColumns; ++i ) {
    const coral::IColumn& column = view.column( i );
    std::cout << "    " << column.name() << " (" << column.type() << ")";
    if ( column.isUnique() ) std::cout << " UNIQUE";
    if ( column.isNotNull() ) std::cout << " NOT NULL";
    std::cout << std::endl;
  }
  std::cout << "  definition string : " << view.definition() << std::endl;
}

void
SchemaDefinition::readSchema()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  // Examine the tables
  this->printTableInfo( session.nominalSchema().tableHandle(T1).description() );
  this->printTableInfo( session.nominalSchema().tableHandle(T2).description() );
  this->printTableInfo( session.nominalSchema().tableHandle(T3).description() );

  std::cout << "read view" << std::endl;

  this->printViewInfo( session.nominalSchema().viewHandle(V0) );

  session.transaction().commit();

  std::cout << "read done" << std::endl;


}

void
SchemaDefinition::printTableInfo( const coral::ITableDescription& description ) const
{
  int numberOfColumns = description.numberOfColumns();
  std::cout << "Table " << description.name();
  std::string tableSpaceName = description.tableSpaceName();
  if ( ! tableSpaceName.empty() )
    std::cout << " (created in tablespace " << tableSpaceName << ")";
  std::cout << " has" << std::endl
            << "  " << numberOfColumns << " columns :" << std::endl;
  for ( int i = 0; i < numberOfColumns; ++i ) {
    const coral::IColumn& column = description.columnDescription( i );
    std::cout << "    " << column.name() << " (" << column.type() << ")";
    if ( column.isUnique() ) std::cout << " UNIQUE";
    if ( column.isNotNull() ) std::cout << " NOT NULL";
    std::cout << std::endl;
  }

  if ( description.hasPrimaryKey() ) {
    const std::vector<std::string>& columnNames = description.primaryKey().columnNames();
    std::cout << "  Primary key defined for column";
    if ( columnNames.size() > 1 ) std::cout << "s";
    std::cout << " ";
    for ( std::vector<std::string>::const_iterator iColumn = columnNames.begin();
          iColumn != columnNames.end(); ++iColumn ) {
      if ( iColumn != columnNames.begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::string tableSpace = description.primaryKey().tableSpaceName();
    if ( ! tableSpace.empty() )
      std::cout << " in tablespace " << tableSpace;
    std::cout << std::endl;
  }

  int numberOfUniqueConstraints = description.numberOfUniqueConstraints();
  std::cout << "  " << numberOfUniqueConstraints;
  if ( numberOfUniqueConstraints == 1 ) std::cout << " unique constraint:" << std::endl;
  else if ( numberOfUniqueConstraints == 0 ) std::cout << " unique constraints" << std::endl;
  else std::cout << " unique constraints:" << std::endl;
  for ( int i = 0; i < numberOfUniqueConstraints; ++i ) {
    const coral::IUniqueConstraint& uniqueConstraint = description.uniqueConstraint( i );
    std::cout << "    " << uniqueConstraint.name() << " defined for column";
    const std::vector<std::string>& columnNames = uniqueConstraint.columnNames();
    if ( columnNames.size() > 1 ) std::cout << "s";
    std::cout << " ";
    for (  std::vector<std::string>::const_iterator iColumn = columnNames.begin();
           iColumn != columnNames.end(); ++iColumn ) {
      if ( iColumn != columnNames.begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::string tableSpace = uniqueConstraint.tableSpaceName();
    if ( ! tableSpace.empty() )
      std::cout << " in tablespace " << tableSpace;
    std::cout << std::endl;
  }

  int numberOfIndices = description.numberOfIndices();
  std::cout << "  " << numberOfIndices;
  if ( numberOfIndices == 1 ) std::cout << " index:" << std::endl;
  else if ( numberOfIndices == 0 ) std::cout << " indices" << std::endl;
  else std::cout << " indices:" << std::endl;
  for ( int i = 0; i < numberOfIndices; ++i ) {
    const coral::IIndex& index = description.index( i );
    std::cout << "    " << index.name();
    if ( index.isUnique() ) std::cout << " (UNIQUE)";
    std::cout << " defined for column";
    const std::vector<std::string>& columnNames = index.columnNames();
    if ( columnNames.size() > 1 ) std::cout << "s";
    std::cout << " ";
    for (  std::vector<std::string>::const_iterator iColumn = columnNames.begin();
           iColumn != columnNames.end(); ++iColumn ) {
      if ( iColumn != columnNames.begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::string tableSpace = index.tableSpaceName();
    if ( ! tableSpace.empty() )
      std::cout << " in tablespace " << tableSpace;
    std::cout << std::endl;
  }

  int numberOfForeignKeys = description.numberOfForeignKeys();
  std::cout << "  " << numberOfForeignKeys;
  if ( numberOfForeignKeys == 1 ) std::cout << " foreign key:" << std::endl;
  else if ( numberOfForeignKeys == 0 ) std::cout << " foreign keys" << std::endl;
  else std::cout << " foreign keys:" << std::endl;
  for ( int i = 0; i < numberOfForeignKeys; ++i ) {
    const coral::IForeignKey& foreignKey = description.foreignKey( i );
    std::cout << "    " << foreignKey.name() << " defined for column";
    const std::vector<std::string>& columnNames = foreignKey.columnNames();
    if ( columnNames.size() > 1 ) std::cout << "s";
    std::cout << " ";
    for (  std::vector<std::string>::const_iterator iColumn = columnNames.begin();
           iColumn != columnNames.end(); ++iColumn ) {
      if ( iColumn != columnNames.begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " -> " << foreignKey.referencedTableName() << "( ";
    const std::vector<std::string>& columnNamesR = foreignKey.referencedColumnNames();
    for (  std::vector<std::string>::const_iterator iColumn = columnNamesR.begin();
           iColumn != columnNamesR.end(); ++iColumn ) {
      if ( iColumn != columnNamesR.begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " )" << std::endl;
  }
}

//-----------------------------------------------------------------------------

class DmlOperations : public Testing
{
public:
  DmlOperations( const TestEnv& env);

  void atomicOperations();
  void bulkOperations();

};
DmlOperations::DmlOperations( const TestEnv& env)
  : Testing(env)
{
}

void
DmlOperations::atomicOperations()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ITableDataEditor& editor1 = session.nominalSchema().tableHandle(T1).dataEditor();
  coral::AttributeList rowBuffer1;
  rowBuffer1.extend<long>( "id" );
  rowBuffer1.extend<long long>( "t" );
  rowBuffer1.extend<float>( "X" );
  rowBuffer1.extend<double>( "Y" );
  rowBuffer1.extend<double>( "Z" );
  rowBuffer1.extend<std::string>( "Comment" );
  rowBuffer1.extend<coral::Blob>( "Data" );

  long& id = rowBuffer1[ "id" ].data<long>();
  long long& t = rowBuffer1[ "t" ].data<long long>();
  float& x = rowBuffer1[ "X" ].data<float>();
  double& y = rowBuffer1[ "Y" ].data<double>();
  double& z = rowBuffer1[ "Z" ].data<double>();
  std::string& comment = rowBuffer1[ "Comment" ].data<std::string>();
  coral::Blob& blob = rowBuffer1[ "Data" ].data<coral::Blob>();

  for ( int i = 0; i < 10; ++i ) {
    id = i + 1;
    t = 1;
    t <<= 4*i;
    x = (float)( 1 + (i+1)*0.001 );
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

  // Delete some rows.
  std::string deleteCondition = "Z > :z";
  coral::AttributeList deleteData;
  deleteData.extend<float>( "z" );
  deleteData[0].data<float>() = 7;
  long rowsDeleted = editor1.deleteRows( deleteCondition, deleteData );
  std::cout << "Deleted " << rowsDeleted << " rows." << std::endl;
  if ( rowsDeleted != 3 )
    throw std::runtime_error( "Unexpected number of rows deleted" );

  // Update some rows.
  std::string updateAction = "t = t + :offset";
  std::string updateCondition = "X < :x";
  coral::AttributeList updateData;
  updateData.extend<int>("offset");
  updateData.extend<float>("x");
  updateData[0].data<int>() = 111;
  updateData[1].data<float>() = (float)1.003;
  long rowsUpdated = editor1.updateRows( updateAction, updateCondition, updateData );
  std::cout << "Updated " << rowsUpdated << " rows." << std::endl;
  if ( rowsUpdated != 2 ) {
    throw std::runtime_error( "Unexpected number of rows updated" );
  }
  session.transaction().commit();
}


void
DmlOperations::bulkOperations()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  // Fill in table T2
  coral::ITableDataEditor& editor2 = session.nominalSchema().tableHandle(T2).dataEditor();
  coral::AttributeList rowBuffer2;
  rowBuffer2.extend<long>( "id" );
  rowBuffer2.extend<float>( "tx" );
  rowBuffer2.extend<double>( "ty" );

  long& id = rowBuffer2[0].data<long>();
  float& tx = rowBuffer2[1].data<float>();
  double& ty = rowBuffer2[2].data<double>();
  coral::IBulkOperation* bulkInserter = editor2.bulkInsert( rowBuffer2, 10 );
  for ( int i = 0; i < 35; ++i ) {
    id = i;
    tx = (float)( i + 100.1 );
    ty = i + 200.234;
    bulkInserter->processNextIteration();
  }
  bulkInserter->flush();
  delete bulkInserter;

  // Bulk delete some rows
  std::string deleteCondition = "tx < :tx AND id > :id ";
  coral::AttributeList deleteData;
  deleteData.extend<float>( "tx" );
  deleteData.extend<long>( "id" );
  coral::IBulkOperation* bulkDeleter = editor2.bulkDeleteRows( deleteCondition,
                                                               deleteData,
                                                               3 );
  for ( int i = 0; i < 5; ++i ) {
    deleteData[0].data<float>() = (float)( 104 + 4*i );
    deleteData[1].data<long>() = 1 + 4*i;
    bulkDeleter->processNextIteration();
  }
  bulkDeleter->flush();
  delete bulkDeleter;

  // Bulk update some rows
  std::string updateAction = "tx = :offset";
  std::string updateCondition = "ty > :minValue AND ty < :maxMalue";
  coral::AttributeList updateData;
  updateData.extend<float>( "offset" );
  updateData.extend<float>( "minValue" );
  updateData.extend<float>( "maxMalue" );
  float& offset = updateData[0].data<float>();
  float& minValue = updateData[1].data<float>();
  float& maxValue = updateData[2].data<float>();

  coral::IBulkOperation* bulkUpdater = editor2.bulkUpdateRows( updateAction,
                                                               updateCondition,
                                                               updateData,
                                                               3 );
  for ( int i = 0; i < 5; ++i ) {
    offset = (float)i;
    minValue = (float)(200 + i*3);
    maxValue = (float)(202 + i*3);
    bulkUpdater->processNextIteration();
  }
  bulkUpdater->flush();
  delete bulkUpdater;

  // Fill-in table T3
  coral::ITableDataEditor& editor3 = session.nominalSchema().tableHandle(T3).dataEditor();
  coral::AttributeList rowBuffer3;
  rowBuffer3.extend<long>( "id" );
  rowBuffer3.extend<long>( "fk1" );
  rowBuffer3.extend<long>( "fk2" );
  rowBuffer3.extend<float>( "Qn" );

  long& id3 = rowBuffer3[0].data<long>();
  long& fk1 = rowBuffer3[1].data<long>();
  long& fk2 = rowBuffer3[2].data<long>();
  float& q = rowBuffer3[3].data<float>();
  bulkInserter = editor3.bulkInsert( rowBuffer3, 3 );
  for ( int i = 0; i < 6; ++i ) {
    id3 = i+1;
    fk1 = i+1;
    fk2 = i*4 + i%2;
    q = (float)( (i+1) * 0.3 );
    bulkInserter->processNextIteration();
  }
  bulkInserter->flush();
  delete bulkInserter;
  session.transaction().commit();
}

//-----------------------------------------------------------------------------

int main(int argc, char *argv[]){

  TestEnv TC01("BASIS");

  if(TC01.check(argc, argv)) {

    //add the default connection strings to the test application
    TC01.addServiceName(TEST_CORE_SCHEME_ADMIN, TEST_CORE_SCHEME_ADMIN);

    try
    {
      std::cout << "Test 1: defining and reading back tables and views for writer " << TC01.getUrlRW() << std::endl;

      SchemaDefinition SchD(TC01);
      //setup session for service string 0
      SchD.createSession(0);
      //create schema and view
      SchD.createSchemaDefault();
      SchD.createViewDefault();
      //delete old session and create new one for reading
      SchD.deleteSession();
      std::cout << "ende schema" << std::endl;

      std::cout << "Test 2: DML operations" << std::endl;

      DmlOperations DmlO(TC01);
      //setup session for writing
      DmlO.createSession(0);
      DmlO.atomicOperations();
      DmlO.bulkOperations();
      DmlO.deleteSession();

      std::cout << "Test 3: Read Only Part, SchemaDefinition, Queries" << std::endl;

      SchD.createSession(0, coral::ReadOnly);
      //read from schema
      SchD.readSchemaDefault();

      SchD.deleteSession();

      Queries Quer(TC01);

      Quer.createSession(0, coral::ReadOnly);
      Quer.run();
      Quer.deleteSession();

      std::cout << "Test 4: Insert/Select statements" << std::endl;

      InsertSelect InsS(TC01);

      InsS.createSession(0);
      InsS.run();
      InsS.deleteSession();

      std::cout << "Test 5: Queries involving set operations" << std::endl;

      SetOperations SetO(TC01);

      SetO.createSession(0);
      SetO.run();
      SetO.deleteSession();

      std::cout << "Test 6: Test transactions" << std::endl;
      Transaction tr(TC01);
      tr.run();

      std::cout << "Test 7: Grouby" << std::endl;

      GroupApp group(TC01);

      group.createSession(0);
      group.fillData();
      group.deleteSession();

      group.createSession(0, coral::ReadOnly);
      group.readData();
      group.deleteSession();

    }
    catch ( coral::Exception& e )
    {
      std::cerr << "CORAL Exception : " << e.what() << std::endl;
      return 1;
    }
    catch ( std::exception& e )
    {
      std::cerr << "C++ Exception : " << e.what() << std::endl;
      return 1;
    }
    catch ( ... )
    {
      std::cerr << "Unhandled exception " << std::endl;
      return 1;
    }
    std::cout << "[OVAL] Success" << std::endl;
    return 0;
  }
  return 1;
}

//-----------------------------------------------------------------------------
