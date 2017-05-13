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
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

#include <iostream>
#include <stdexcept>
#include <set>

SchemaApp::SchemaApp( const std::string& connectionString1, const std::string& connectionString2, const std::string& userName, const std::string& password )
  : TestBase(), m_connectionString1( connectionString1 ), m_connectionString2( connectionString2 ), m_userName( userName ), m_password( password )
{
}

SchemaApp::~SchemaApp() {
}

void SchemaApp::run()
{
  coral::ISession* session1 = this->connect( m_connectionString1, m_userName, m_password );
  coral::ISession* session2 = this->connect( m_connectionString2, m_userName, m_password );

  session1->transaction().start();

  coral::ISchema& schema = session1->nominalSchema();

  std::cout << "About to drop previous tables" << std::endl;
  schema.dropIfExistsTable( "T_1" );
  schema.dropIfExistsTable( "T_0" );

  std::cout << "Describing new tables" << std::endl;
  coral::TableDescription description0;
  description0.setName( "T_0" );
  description0.insertColumn( "ID",
                             coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description0.setUniqueConstraint( "ID" );
  description0.insertColumn( "X",
                             coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description0.insertColumn( "Y",
                             coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description0.insertColumn( "Booby",
                             coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );
  description0.insertColumn( "dd",
                             coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
  description0.insertColumn( "ts",
                             coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ) );
  std::vector< std::string > cols;
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
  description.insertColumn( "ID_FK",
                            coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description.createForeignKey( "T_1_FK", "ID_FK", "T_0", "ID" );

  std::cout << "About to create the tables" << std::endl;
  coral::ITable& table0 = schema.createTable( description0 );
  table0.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select ); // DON'T FORGET THIS FOR FRONTIER!!!!!
  table0.schemaEditor().renameColumn( "Y", "Z" );

  coral::ITable& table = schema.createTable( description );
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select ); // DON'T FORGET THIS FOR FRONTIER!!!!!

  std::string tableSpaceName = table.description().tableSpaceName();

  std::cout << "Table T_1 created under the table space \"" << tableSpaceName << "\"" << std::endl;

  session1->transaction().commit();

  session1->endUserSession();

  delete session1;

  std::cout << "Starting READ ONLY Frontier transaction" << std::endl;

  // Read the schema information via Frontier
  session2->transaction().start( true );

  coral::ISchema& schema2 = session2->schema( "CORAL" );

  std::cout << "Retrieved nominal schema which has the tables: " << std::endl;

  std::set<std::string> lti = schema2.listTables();

  for( std::set<std::string>::const_iterator tli = lti.begin(); tli != lti.end(); ++tli )
    std::cout << (*tli) << std::endl;

  coral::ITable& table20 = schema2.tableHandle( "T_0" );
  coral::ITable& table21  = schema2.tableHandle( "T_1" );

  std::cout << "Retrieving the table description." << std::endl;

  std::cout << "Description : " << std::endl;

  const coral::ITableDescription& descr21 = table21.description();

  int numberOfColumns = descr21.numberOfColumns();

  for ( int i = 0; i < numberOfColumns; ++i )
  {
    const coral::IColumn& column = table21.description().columnDescription( i );

    std::cout << column.name() << " : " << column.type();

    if ( column.isNotNull() )
      std::cout << " NOT NULL";

    if ( column.isUnique() )
      std::cout << " UNIQUE";

    std::cout << std::endl;
  }

  if ( table21.description().hasPrimaryKey() )
  {
    const coral::IPrimaryKey& pk = table21.description().primaryKey();

    std::cout << "Table has primary key defined in tablespace " << pk.tableSpaceName() << std::endl << " for column(s) ";

    for ( std::vector<std::string>::const_iterator iColumn = pk.columnNames().begin(); iColumn != pk.columnNames().end(); ++iColumn )
      std::cout << "\"" << *iColumn << "\" ";

    std::cout << std::endl;
  }

  std::cout << "Foreign keys defined:" << std::endl;
  for ( int i = 0; i < table21.description().numberOfForeignKeys(); ++i )
  {
    const coral::IForeignKey& fkey = table21.description().foreignKey( i );

    std::cout << "   " << fkey.name() << " ( ";

    for ( std::vector<std::string>::const_iterator iColumn = fkey.columnNames().begin(); iColumn != fkey.columnNames().end(); ++iColumn )
    {
      if ( iColumn != fkey.columnNames().begin() )
        std::cout << ", ";
      std::cout << *iColumn;
    }

    std::cout << " ) -> " << fkey.referencedTableName() << " ( ";

    for ( std::vector<std::string>::const_iterator iColumn = fkey.referencedColumnNames().begin(); iColumn != fkey.referencedColumnNames().end(); ++iColumn )
    {
      if ( iColumn != fkey.referencedColumnNames().begin() )
        std::cout << ", ";
      std::cout << *iColumn;
    }

    std::cout << " )" << std::endl;
  }

  std::cout << "Indices for Table T_0 :" << std::endl;

  for ( int i = 0; i < table20.description().numberOfIndices(); ++i )
  {
    const coral::IIndex& index = table20.description().index( i );

    std::cout << " " << index.name() << " -> ";

    for ( std::vector<std::string>::const_iterator iColumn = index.columnNames().begin(); iColumn != index.columnNames().end(); ++iColumn )
    {
      if ( iColumn != index.columnNames().begin() )
        std::cout << ", ";
      std::cout << *iColumn;
    }
    std::cout << " (tablespace : " << index.tableSpaceName() << ")" << std::endl;
  }

  session2->transaction().commit();

  delete session2;
}
