#include "QueriesApp.h"

#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

#include <iostream>
#include <stdexcept>

QueriesApp::QueriesApp( const std::string& connectionString, const std::string& userName, const std::string& password )
  : TestBase(), m_connectionString( connectionString ), m_userName( userName ), m_password( password )
{
}

QueriesApp::~QueriesApp()
{
}

void QueriesApp::run()
{
  this->fillData();
  this->readData();
}

void QueriesApp::fillData()
{
  coral::ISession* session = this->connect( m_connectionString, m_userName, m_password, coral::Update );
  session->transaction().start();

  coral::ISchema& schema = session->nominalSchema();

  std::cout << "About to drop previous table" << std::endl;
  schema.dropIfExistsTable( "T" );

  std::cout << "Describing new table" << std::endl;
  coral::TableDescription description;
  description.setName( "T" );

  description.insertColumn( "ID", coral::AttributeSpecification::typeNameForId( typeid(unsigned int) ) ); // We long even for ints as SQL<->C++ mapping is ambiguous
  description.setPrimaryKey( "ID" );

  description.insertColumn( "x", coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description.setNotNullConstraint( "x" );

  description.insertColumn( "Y", coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  description.insertColumn( "sc", coral::AttributeSpecification::typeNameForId( typeid(char) ) );
  description.insertColumn( "uc", coral::AttributeSpecification::typeNameForId( typeid(unsigned char) ) );
  description.insertColumn( "date", coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
  description.insertColumn( "data2", coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );
  description.insertColumn( "data1", coral::AttributeSpecification::typeNameForId( typeid(std::string) ), 100, false );
  description.insertColumn( "Z", coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  description.insertColumn( "ts", coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ), 14 );

  std::cout << "About to create the table" << std::endl;
  coral::ITable& table = schema.createTable( description );

  std::string s;
  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );

  for ( int i = 0; i < 5; ++i )
  {
    rowBuffer["ID"].data<unsigned int>() = i;
    rowBuffer["x"].data<float>() = (float)( i + 0.1 * i );
    rowBuffer["Y"].data<double>() = i + 0.01 * i;
    if ( i%2 == 1 ) rowBuffer["Y"].setNull( true );
    else rowBuffer["Y"].setNull( false );

    if( i % 2 )
      rowBuffer["sc"].data<char>() = (char)i;
    else
      rowBuffer["sc"].data<char>() = (char)(i + 'a');

    rowBuffer["uc"].data<unsigned char>() = i;

    rowBuffer["date"].data<coral::Date>() = coral::Date( 2005, 12, i+1 );
    rowBuffer["data1"].data<std::string>() = "Data";
    rowBuffer["Z"].data<double>() = i + 0.001 * i;
    rowBuffer["ts"].data<coral::TimeStamp>() = coral::TimeStamp::now();

    coral::Blob& blob = rowBuffer["data2"].data<coral::Blob>();

    int blobSize = ( i + 1 ) * 1000;
    blob.resize( blobSize );
    unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
    for ( int j = 0; j < blobSize; ++j, ++p ) *p = j%256;

    if ( i%2 == 1 ) rowBuffer["data2"].setNull( true );
    else rowBuffer["data2"].setNull( false );

    std::cout << "Going to insert row buffer:" << std::endl;
    rowBuffer.toOutputStream( std::cout );
    std::cout << std::endl;

    table.dataEditor().insertRow( rowBuffer );
  }

  session->transaction().commit();
  delete session;
}

void QueriesApp::readData()
{
  coral::ISession* session = this->connect( m_connectionString, m_userName, m_password, coral::ReadOnly );
  session->transaction().start( true );

  coral::ISchema& schema = session->nominalSchema();

  coral::IQuery* query = schema.tableHandle("T").newQuery();
  query->limitReturnedRows( 4 );
  coral::ICursor& cursor = query->execute();

  int i = 0;
  while ( cursor.next() )
  {
    const coral::AttributeList& currentRow = cursor.currentRow();
    currentRow.toOutputStream( std::cout ) << std::endl;

    const coral::Attribute& blobAttribute = currentRow["data2"];

    if ( ! blobAttribute.isNull() )
    {
      const coral::Blob& blob = blobAttribute.data<coral::Blob>();
      int blobSize = ( i + 1 ) * 1000;
      if ( blob.size() != blobSize )
        throw std::runtime_error( "Unexpected blob size" );
      const unsigned char* p = static_cast<const unsigned char*>( blob.startingAddress() );
      for ( int j = 0; j < blobSize; ++j, ++p )
        if( *p != j%256 )
          throw std::runtime_error( "Unexpected blob data" );
    }
    ++i;
  }

  if ( i != 4 )
    throw std::runtime_error( "Unexpected number of rows" );

  delete query;

  // Count the number of rows in the table
  query = schema.tableHandle("T").newQuery();
  query->addToOutputList( "COUNT(*)" );
  coral::ICursor& count_cursor = query->execute();

  std::cout << "Table T has ";
  while ( count_cursor.next() )
  {
    const coral::AttributeList& currentRow = count_cursor.currentRow();
    currentRow.toOutputStream( std::cout ) << std::endl;
  }
  std::cout << " rows" << std::endl;

  delete query;

  session->transaction().commit();

  delete session;
}
