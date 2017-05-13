#include <iostream>
#include <stdexcept>
#include "CoralBase/Exception.h"
#include "CoralBase/AttributeSpecification.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IPrimaryKey.h"

int main( int, char** )
{
  try {
    coral::TableDescription description;
    description.setName( "MyTable" );
    description.insertColumn( "Id",
                              coral::AttributeSpecification::typeNameForId( typeid(int) ) );
    description.insertColumn( "Data",
                              coral::AttributeSpecification::typeNameForId( typeid(std::string) ),
                              1000, true );
    description.setPrimaryKey( "Id" );
    description.insertColumn( "x",
                              coral::AttributeSpecification::typeNameForId( typeid(float) ) );
    description.insertColumn( "y",
                              coral::AttributeSpecification::typeNameForId( typeid(double) ) );
    if ( description.numberOfColumns() != 4 )
      throw std::runtime_error( "Unexpected number of columns 1" );

    description.dropColumn( "x" );
    int nCols = description.numberOfColumns();
    if ( nCols != 3 )
      throw std::runtime_error( "Unexpected number of columns 2" );

    description.setNotNullConstraint( "Id" );
    description.setNotNullConstraint( "y" );
    description.createIndex( "idx", "y", true );

    const coral::IColumn& col1 = description.columnDescription( 0 );
    if ( ! col1.isNotNull() )
      throw std::runtime_error( "Missing not null attribute" );

    const coral::IColumn& col2 = description.columnDescription( "y" );
    if ( col2.indexInTable() != 2 )
      throw std::runtime_error( "Unexpected column index" );

    if ( ! description.hasPrimaryKey() )
      throw std::runtime_error( "Primary key not found" );

    const coral::IPrimaryKey& primaryKey = description.primaryKey();
    if ( primaryKey.columnNames().size() != 1 )
      throw std::runtime_error( "Unexpected number of columns in the primary key" );
    if ( primaryKey.columnNames()[0] != "Id" )
      throw std::runtime_error( "Unexpected column name in the primary key" );

  }
  catch ( coral::Exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
