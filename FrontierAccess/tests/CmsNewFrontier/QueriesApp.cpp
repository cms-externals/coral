#include <iostream>
#include <memory>
#include <stdexcept>
#include <limits.h>

#include "QueriesApp.h"

#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/IConnectionService.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"

#include "CoralCommon/Sleep.h"

static const unsigned char ucvalue = '\''; // Bug #23370

QueriesApp::QueriesApp( const std::string& connectionString1,
                        const std::string& connectionString2,
                        const std::string& userName,
                        const std::string& password )
  : TestBase(),
    m_connectionString1( connectionString1 ),
    m_connectionString2( connectionString2 ),
    m_userName( userName ), m_password( password )
{
}

QueriesApp::~QueriesApp()
{
}

void QueriesApp::run()
{
  this->fillData();
  coral::sys::sleep(1);
  this->readData();
}

void QueriesApp::fillData()
{
  try
  {
    std::unique_ptr<coral::ISessionProxy> session( connectionService()->connect( m_connectionString1 ) );

    session->transaction().start();

    coral::ISchema& schema = session->nominalSchema();

    std::cout << "About to drop previous table" << std::endl;
    schema.dropIfExistsTable( "T" );

    std::cout << "Describing new table" << std::endl;

    coral::AttributeList rowBuffer;
    coral::TableDescription description;

    description.setName( "T" );

    description.insertColumn( "ID",    coral::AttributeSpecification::typeNameForId( typeid(int) ) ); description.setPrimaryKey( "ID" );         rowBuffer.extend<int>( "ID" );
    description.insertColumn( "x",     coral::AttributeSpecification::typeNameForId( typeid(float) ) ); description.setNotNullConstraint( "x" ); rowBuffer.extend<float>( "x" );
    description.insertColumn( "Y",     coral::AttributeSpecification::typeNameForId( typeid(double) ) );                                         rowBuffer.extend<double>( "Y" );
    description.insertColumn( "Z",     coral::AttributeSpecification::typeNameForId( typeid(double) ) );                                         rowBuffer.extend<double>( "Z" );
    description.insertColumn( "data1", coral::AttributeSpecification::typeNameForId( typeid(std::string) ), 100, false );                        rowBuffer.extend<std::string>( "data1" );
    description.insertColumn( "data2", coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );                                    rowBuffer.extend<coral::Blob>( "data2" );
    description.insertColumn( "uc",    coral::AttributeSpecification::typeNameForId( typeid(unsigned char) ) );                                  rowBuffer.extend<unsigned char>( "uc" );
    description.insertColumn( "c",     coral::AttributeSpecification::typeNameForId( typeid(char) ) );                                           rowBuffer.extend<char>( "c" );
    description.insertColumn( "ul",    coral::AttributeSpecification::typeNameForId( typeid(unsigned long) ) );                                  rowBuffer.extend<unsigned long>( "ul" );

    std::cout << "About to create the table" << std::endl;
    coral::ITable& table = schema.createTable( description );
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select ); // DON'T FORGET THIS FOR FRONTIER!!!!!

    for ( int i = 0; i < 5; ++i )
    {
      rowBuffer["ID"].data<int>() = i;
      rowBuffer["x"].data<float>() = i + 0.1 * i;
      rowBuffer["Y"].data<double>() = i + 0.01 * i;
      if ( i%2 == 1 ) rowBuffer["Y"].setNull( true );
      else rowBuffer["Y"].setNull( false );
      rowBuffer["Z"].data<double>() = i + 0.001 * i;
      rowBuffer["data1"].data<std::string>() = "Data";

      coral::Blob& blob = rowBuffer["data2"].data<coral::Blob>();

      int blobSize = ( i + 1 ) * 1000;
      blob.resize( blobSize );
      unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
      for ( int j = 0; j < blobSize; ++j, ++p ) *p = j%256;

      if ( i%2 == 1 ) rowBuffer["data2"].setNull( true );
      else rowBuffer["data2"].setNull( false );

      rowBuffer["uc"].data<unsigned char>() = i;
      rowBuffer["c"].data<char>() = 'a'+i;

      if( i == 0 )
        rowBuffer["ul"].data<unsigned long>() = UINT_MAX;
      else
        rowBuffer["ul"].data<unsigned long>() = i;

      table.dataEditor().insertRow( rowBuffer );
    }


    // Test for bug #23334
    schema.dropIfExistsTable( "test_COOL_2_0_0_bug23334" );

    coral::AttributeList rowBufferlcd;
    coral::TableDescription lcd;
    lcd.setName( "test_COOL_2_0_0_bug23334" ); // We make sure the table name contains mixture of lower/upper case chars and numbers

    lcd.insertColumn( "ID",    coral::AttributeSpecification::typeNameForId( typeid(int) ) ); lcd.setPrimaryKey( "ID" );         rowBufferlcd.extend<int>( "ID" );
    lcd.insertColumn( "x",     coral::AttributeSpecification::typeNameForId( typeid(float) ) ); lcd.setNotNullConstraint( "x" ); rowBufferlcd.extend<float>( "x" );
    lcd.insertColumn( "uc",    coral::AttributeSpecification::typeNameForId( typeid(unsigned char) ) );                          rowBufferlcd.extend<unsigned char>( "uc" ); // Due to bugs #22307 & #23370

    std::cout << "About to create the table" << std::endl;
    coral::ITable& tablelcd = schema.createTable( lcd );
    tablelcd.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select ); // DON'T FORGET THIS FOR FRONTIER!!!!!

    for ( int i = 0; i < 5; ++i )
    {
      rowBufferlcd["ID"].data<int>() = i;
      rowBufferlcd["x"].data<float>() = i + 0.1 * i;

      if( i == 0 )
        rowBufferlcd["uc"].data<unsigned char>() = ucvalue;  // Bug #23370
      else
        rowBufferlcd["uc"].data<unsigned char>() = i+26;

      tablelcd.dataEditor().insertRow( rowBufferlcd );
    }

    session->transaction().commit();
  }
  catch( const std::exception& e )
  {
    std::cerr << "Problem filling data: " << e.what() << std::endl;
    throw;
  }

  //  delete session;
}

void QueriesApp::readData()
{
  try
  {
    std::unique_ptr<coral::ISessionProxy> session( connectionService()->connect( m_connectionString2, coral::ReadOnly ) );

    session->transaction().start( true );

    coral::ISchema& schema = session->nominalSchema();

    coral::ITable& table   = schema.tableHandle("T");
    coral::IQuery* query   = table.newQuery();
    coral::ICursor& cursor = query->execute();

    int i = 0;
    while ( cursor.next() )
    {
      const coral::AttributeList& currentRow = cursor.currentRow();
      currentRow.toOutputStream( std::cout ) << std::endl;
      //       const coral::Attribute& blobAttribute = currentRow["data2"];
      //       if ( ! blobAttribute.isNull() )
      //       {
      //         const coral::Blob& blob = blobAttribute.data<coral::Blob>();

      //         std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>Blob size: " << blob.size() << std::endl;

      //         int blobSize = ( i + 1 ) * 1000;
      //         if ( blob.size() != blobSize )
      //           throw std::runtime_error( "Unexpected blob size" );
      //         const unsigned char* p = static_cast<const unsigned char*>( blob.startingAddress() );
      //         for ( int j = 0; j < blobSize; ++j, ++p )
      //           if( *p != j%256 )
      //             throw std::runtime_error( "Unexpected blob data" );
      //       }
      ++i;
    }

    if ( i != 5 )
      throw std::runtime_error( "Unexpected number of rows" );

    delete query;

    // Test for bug #23334
    coral::ITable& tablelcd   = schema.tableHandle( "test_COOL_2_0_0_bug23334" );
    coral::IQuery* querylcd   = tablelcd.newQuery();
    coral::ICursor& cursorlcd = querylcd->execute();

    int ilcd = 0;
    while ( cursorlcd.next() )
    {
      const coral::AttributeList& currentRow = cursorlcd.currentRow();
      currentRow.toOutputStream( std::cout ) << std::endl;

      if( ilcd == 0 )
      {
        // Bug 23370
        const coral::Attribute& ucattr = currentRow["uc"];

        unsigned char rucvalue = ucattr.data<short>();

        if( rucvalue != ucvalue )
          std::cerr << "Bug #23370 still present, expected value: " << (unsigned short)ucvalue << " read value: " << (unsigned short)rucvalue << std::endl;
      }

      ilcd++;
    }

    delete querylcd;

    // Test for bug #16621
    coral::IQuery* querycnt   = tablelcd.newQuery();
    querycnt->addToOutputList( "count(*)", "lowercase count" );
    querycnt->addToOutputList( "COUNT(*)", "UPPERCASE COUNT" );
    coral::ICursor& cursorcnt = querycnt->execute();

    int icnt = 0;
    while ( cursorcnt.next() )
    {
      const coral::AttributeList& currentRow = cursorcnt.currentRow();
      currentRow.toOutputStream( std::cout ) << std::endl;
      icnt++;
    }

    delete querycnt;

    session->transaction().commit();
  }
  catch( const std::exception& e )
  {
    std::cerr << "Problem reading data: " << e.what() << std::endl;
    throw;
  }

  //  delete session;
}
