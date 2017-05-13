#include "SchemaTest.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"

#include "RelationalAccess/SchemaException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Blob.h"

#include "CoralCommon/Utilities.h"

#include <stdexcept>
#include <sstream>
#include <memory>
#include <iostream>

SchemaTest::SchemaTest(coral::ISessionProxy *proxy)
{
  m_proxy = proxy;
}


SchemaTest::~SchemaTest()
{

  //  delete m_proxy;
}


bool SchemaTest::createSchema()
{
  try {
    m_proxy->transaction().start();

    // Get rid of the previous tables and views
    m_proxy->nominalSchema().dropIfExistsView( "V0" );
    m_proxy->nominalSchema().dropIfExistsTable( "T3" );
    m_proxy->nominalSchema().dropIfExistsTable( "T2" );
    m_proxy->nominalSchema().dropIfExistsTable( "T1" );

    // Create the first table
    coral::TableDescription description1( "SchemaTest_Test" );
    description1.setName( "T1" );
    description1.insertColumn( "id",
                               coral::AttributeSpecification::typeNameForId( typeid(int) ) );
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
    description1.setUniqueConstraint( "Y", "U_T1_Y" );
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
    coral::ITable& table = m_proxy->nominalSchema().createTable( description1 );


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
    table.schemaEditor().createIndex( "T1_IDX_t", "t", true );

    // Give privileges
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

    // Create the second table
    coral::TableDescription description2( "SchemaDefinition_Test" );
    description2.setName( "T2" );
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
    m_proxy->nominalSchema().createTable( description2 ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );



    // Create the third table
    coral::TableDescription description3( "SchemaTest_Test" );
    description3.setName( "T3" );
    description3.insertColumn( "id",
                               coral::AttributeSpecification::typeNameForId( typeid(long) ) );
    description3.setPrimaryKey( "id" );
    description3.insertColumn( "fk1",
                               coral::AttributeSpecification::typeNameForId( typeid(long) ) );
    description3.createForeignKey( "T3_FK1", "fk1", "T1", "id" );
    description3.insertColumn( "fk2",
                               coral::AttributeSpecification::typeNameForId( typeid(long) ) );
    description3.createForeignKey( "T3_FK2", "fk2", "T2", "id" );
    description3.insertColumn( "Qn",
                               coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    m_proxy->nominalSchema().createTable( description3 ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

    m_proxy->transaction().commit();
    coral::sleepSeconds(1);
  }
  catch ( coral::SchemaException& e ) {
    m_proxy->transaction().rollback();
    std::cerr << "Schema exception " << e.what() << std::endl;
    return false;
  }
  return true;
}

bool SchemaTest::readSchema(const std::string& schema)
{
  try {

    m_proxy->transaction().start( true );

    // Examine the tables
    this->printTableInfo( m_proxy->schema(schema).tableHandle( "T1" ).description() );
    this->printTableInfo( m_proxy->schema(schema).tableHandle( "T2" ).description() );
    this->printTableInfo( m_proxy->schema(schema).tableHandle( "T3" ).description() );

    m_proxy->transaction().commit();

  }
  catch ( coral::SchemaException& e ) {
    std::cerr << e.what() << std::endl;
    return false;
  }
  return true;
}

void SchemaTest::printTableInfo( const coral::ITableDescription& description ) const
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
