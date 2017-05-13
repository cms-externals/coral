#include "SchemaApp.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/TableDescription.h"

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Date.h"
#include "CoralBase/Blob.h"

#include <set>
#include <iostream>
#include <stdexcept>

SchemaApp::SchemaApp( const std::string& connectionString, const std::string& userName, const std::string& password )
  : TestBase(), m_connectionString( connectionString ), m_userName( userName ), m_password( password )
{
}

SchemaApp::~SchemaApp()
{
}

void SchemaApp::run()
{
  coral::ISession* session = this->connect( m_connectionString, m_userName, m_password, coral::Update );
  session->transaction().start();

  // Bug #31700
  try
  {
    coral::ISchema& wrongSchema = session->schema("BadSchemaName_____" );
    // Should not get here, it's just to prevent a compilation warning
    wrongSchema.listTables();
  }
  catch( const std::exception& e )
  {
    std::cout << "Caught the wrong schema name exception: " << e.what() << std::endl;
  }

  coral::ISchema& schema = session->nominalSchema();

  schema.listTables();

  std::cout << "About to drop previous tables" << std::endl;
  schema.dropIfExistsTable( "T_1" );
  schema.dropIfExistsTable( "T_0" );

  std::cout << "Describing new tables" << std::endl;
  coral::TableDescription description0;
  description0.setName( "T_0" );
  description0.insertColumn( "ID",
                             coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description0.insertColumn( "X",
                             coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description0.insertColumn( "Y",
                             coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description0.insertColumn( "cc",
                             coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
  description0.insertColumn( "blob",
                             coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ), 321, false );
  description0.insertColumn( "mblob",
                             coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ), 66000, false );

  // Create indices
  description0.createIndex( "T_0_IDX_cc", "cc" );

  std::vector< std::string > cols;

  // We need this index in order to make foreign key constraint working
  cols.push_back( "ID" );
  cols.push_back( "cc" );
  description0.createIndex( "T_0_IDX_ID_cc", cols );

  cols.clear();
  cols.push_back( "X" );
  cols.push_back( "Y" );
  description0.createIndex( "T_0_IDX", cols );

  coral::TableDescription description;
  description.setName( "T_1" );
  description.insertColumn( "ID",
                            coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description.setPrimaryKey( "ID" );
  description.insertColumn( "x",
                            coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description.setNotNullConstraint( "x" );
  description.insertColumn( "Y",
                            coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  description.setUniqueConstraint( "Y" );
  description.insertColumn( "Z",
                            coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  description.insertColumn( "b",
                            coral::AttributeSpecification::typeNameForId( typeid(bool) ) );
  description.insertColumn( "d",
                            coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
  std::vector< std::string > constraintColumns( 2 );
  constraintColumns[0] = "x";
  constraintColumns[1] = "Z";
  description.setUniqueConstraint( constraintColumns, "C_T1" );
  description.insertColumn( "data1",
                            coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                            123,
                            true );
  description.insertColumn( "data2",
                            coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                            321,
                            false );
  // Foreign key columns
  description.insertColumn( "ID_FK",
                            coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description.insertColumn( "cc_FK",
                            coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );

  // Create foreign key
  description.createForeignKey( "T_1_FK", "ID_FK", "T_0", "ID" );

  // Create composite foreign key
  std::vector<std::string> fkcols;
  fkcols.push_back( "ID_FK" );
  fkcols.push_back( "cc_FK" );
  std::vector<std::string> rfkcols;
  rfkcols.push_back( "ID" );
  rfkcols.push_back( "cc" );
  description.createForeignKey( "T_1_FK_composite", fkcols, "T_0", rfkcols );

  std::cout << "About to create the tables" << std::endl;
  coral::ITable& table0 = schema.createTable( description0 );
  // FIXME - table0.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
  table0.schemaEditor().setPrimaryKey( "ID" );
  table0.schemaEditor().renameColumn( "Y", "Z" );

  coral::ITable& table = schema.createTable( description );

  std::string tableSpaceName = table.description().tableSpaceName();
  std::cout << "Table T_1 created under the table space \"" << tableSpaceName << "\"" << std::endl;

  std::cout << "Retrieving the table description." << std::endl;

  std::cout << "Description : " << std::endl;
  int numberOfColumns = table.description().numberOfColumns();
  for ( int i = 0; i < numberOfColumns; ++i ) {
    const coral::IColumn& column = table.description().columnDescription( i );
    std::cout << column.name() << " : " << column.type();
    if ( column.isNotNull() ) std::cout << " NOT NULL";
    if ( column.isUnique() ) std::cout << " UNIQUE";
    std::cout << std::endl;
  }

  if ( table.description().hasPrimaryKey() ) {
    const coral::IPrimaryKey& pk = table.description().primaryKey();
    std::cout << "Table has primary key defined in tablespace " << pk.tableSpaceName() << std::endl
              << " for column(s) ";
    for ( std::vector<std::string>::const_iterator iColumn = pk.columnNames().begin();
          iColumn != pk.columnNames().end(); ++iColumn )
      std::cout << "\"" << *iColumn << "\" ";
    std::cout << std::endl;
  }

  std::cout << "Foreign keys defined:" << std::endl;
  for ( int i = 0; i < table.description().numberOfForeignKeys(); ++i ) {
    const coral::IForeignKey& fkey = table.description().foreignKey( i );
    std::cout << "   " << fkey.name() << " ( ";
    for ( std::vector<std::string>::const_iterator iColumn = fkey.columnNames().begin();
          iColumn != fkey.columnNames().end(); ++iColumn ) {
      if ( iColumn != fkey.columnNames().begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " ) -> " << fkey.referencedTableName() << " ( ";
    for ( std::vector<std::string>::const_iterator iColumn = fkey.referencedColumnNames().begin();
          iColumn != fkey.referencedColumnNames().end(); ++iColumn ) {
      if ( iColumn != fkey.referencedColumnNames().begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " )" << std::endl;
  }

  std::cout << "Indices for Table T_1 :" << std::endl;
  for ( int i = 0; i < table.description().numberOfIndices(); ++i ) {
    const coral::IIndex& index = table.description().index( i );
    std::cout << " " << index.name() << " -> ";
    for ( std::vector<std::string>::const_iterator iColumn = index.columnNames().begin();
          iColumn != index.columnNames().end(); ++iColumn ) {
      if ( iColumn != index.columnNames().begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " (tablespace : " << index.tableSpaceName() << ")" << std::endl;
  }

  session->transaction().commit();

  try
  {
    session->transaction().start( true );

    std::cout << "Retrieving the table description." << std::endl;
    coral::ITable& table00 = schema.tableHandle( "T_DECIMAL" );

    std::cout << "Description : " << std::endl;
    numberOfColumns = table00.description().numberOfColumns();
    for ( int i = 0; i < numberOfColumns; ++i )
    {
      const coral::IColumn& column = table00.description().columnDescription( i );
      std::cout << column.name() << " : " << column.type();
      if ( column.isNotNull() ) std::cout << " NOT NULL";
      if ( column.isUnique() ) std::cout << " UNIQUE";
      std::cout << std::endl;
    }

    session->transaction().commit();
  }
  catch( const std::exception& )
  {
    std::cout << "Apparently T_DECIMAL table does not exist" << std::endl;
    session->transaction().rollback();
  }

  session->transaction().start( true );

  std::cout << "Retrieving the table description." << std::endl;
  coral::ITable& table00 = schema.tableHandle( "T_0" );

  std::cout << "Description : " << std::endl;
  numberOfColumns = table00.description().numberOfColumns();
  for ( int i = 0; i < numberOfColumns; ++i ) {
    const coral::IColumn& column = table00.description().columnDescription( i );
    std::cout << column.name() << " : " << column.type();
    if ( column.isNotNull() ) std::cout << " NOT NULL";
    if ( column.isUnique() ) std::cout << " UNIQUE";
    std::cout << std::endl;
  }

  if ( table00.description().hasPrimaryKey() ) {
    const coral::IPrimaryKey& pk = table00.description().primaryKey();
    std::cout << "Table has primary key defined in tablespace " << pk.tableSpaceName() << std::endl
              << " for column(s) ";
    for ( std::vector<std::string>::const_iterator iColumn = pk.columnNames().begin();
          iColumn != pk.columnNames().end(); ++iColumn )
      std::cout << "\"" << *iColumn << "\" ";
    std::cout << std::endl;
  }

  std::cout << "Foreign keys defined:" << std::endl;
  for ( int i = 0; i < table00.description().numberOfForeignKeys(); ++i ) {
    const coral::IForeignKey& fkey = table00.description().foreignKey( i );
    std::cout << "   " << fkey.name() << " ( ";
    for ( std::vector<std::string>::const_iterator iColumn = fkey.columnNames().begin();
          iColumn != fkey.columnNames().end(); ++iColumn ) {
      if ( iColumn != fkey.columnNames().begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " ) -> " << fkey.referencedTableName() << " ( ";
    for ( std::vector<std::string>::const_iterator iColumn = fkey.referencedColumnNames().begin();
          iColumn != fkey.referencedColumnNames().end(); ++iColumn ) {
      if ( iColumn != fkey.referencedColumnNames().begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " )" << std::endl;
  }


  std::cout << "Indices for Table T_0 :" << std::endl;
  for ( int i = 0; i < table00.description().numberOfIndices(); ++i ) {
    const coral::IIndex& index = table00.description().index( i );
    std::cout << " " << index.name() << " -> ";
    for ( std::vector<std::string>::const_iterator iColumn = index.columnNames().begin();
          iColumn != index.columnNames().end(); ++iColumn ) {
      if ( iColumn != index.columnNames().begin() ) std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " (tablespace : " << index.tableSpaceName() << ")" << std::endl;
  }

  session->transaction().commit();

  delete session;
}
