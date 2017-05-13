#include "TestDriver.h"

#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"


#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include "CoralBase/Blob.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/Date.h"

#include "CoralCommon/Utilities.h"

#include <climits>

static const int rowsWithSimpleInsert = 10;
static const int rowsWithBulkInsert = 55;

InputOutput::InputOutput( const TestEnv& env )
  : Testing(env)
{
}

void InputOutput::prepareSchema()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();
  schema.dropIfExistsTable(T1);

  if ( m_rowBuffer ) delete m_rowBuffer;
  m_rowBuffer = new coral::AttributeList;
  coral::AttributeList& rowBuffer = *m_rowBuffer;

  coral::TableDescription description;
  description.setName(T1);
  description.insertColumn( "ID",
                            coral::AttributeSpecification::typeNameForId( typeid( int ) ) );
  description.setNotNullConstraint( "ID" );
  description.setPrimaryKey( "ID" );
  rowBuffer.extend<int>( "ID" );

  description.insertColumn( "V_B",
                            coral::AttributeSpecification::typeNameForId( typeid( bool ) ) );
  rowBuffer.extend<bool>( "V_B" );

  description.insertColumn( "V_UC",
                            coral::AttributeSpecification::typeNameForId( typeid( unsigned char ) ) );
  rowBuffer.extend<unsigned char>( "V_UC" );

  description.insertColumn( "V_SC",
                            coral::AttributeSpecification::typeNameForId( typeid( char ) ) );
  rowBuffer.extend<char>( "V_SC" );

  description.insertColumn( "V_US",
                            coral::AttributeSpecification::typeNameForId( typeid( unsigned short ) ) );
  rowBuffer.extend<unsigned short>( "V_US" );

  description.insertColumn( "V_SS",
                            coral::AttributeSpecification::typeNameForId( typeid( short ) ) );
  rowBuffer.extend<short>( "V_SS" );

  description.insertColumn( "V_UI",
                            coral::AttributeSpecification::typeNameForId( typeid( unsigned int ) ) );
  rowBuffer.extend<unsigned int>( "V_UI" );

  description.insertColumn( "V_SI",
                            coral::AttributeSpecification::typeNameForId( typeid( int ) ) );
  rowBuffer.extend<int>( "V_SI" );

  description.insertColumn( "V_UL",
                            coral::AttributeSpecification::typeNameForId( typeid( unsigned long ) ) );
  rowBuffer.extend<unsigned long>( "V_UL" );

  description.insertColumn( "V_SL",
                            coral::AttributeSpecification::typeNameForId( typeid( long ) ) );
  rowBuffer.extend<long>( "V_SL" );

  description.insertColumn( "V_ULL",
                            coral::AttributeSpecification::typeNameForId( typeid( unsigned long long ) ) );
  rowBuffer.extend<unsigned long long>( "V_ULL" );

  description.insertColumn( "V_SLL",
                            coral::AttributeSpecification::typeNameForId( typeid( long long ) ) );
  rowBuffer.extend<long long>( "V_SLL" );

  description.insertColumn( "V_F",
                            coral::AttributeSpecification::typeNameForId( typeid( float ) ) );
  rowBuffer.extend<float>( "V_F" );

  description.insertColumn( "V_D",
                            coral::AttributeSpecification::typeNameForId( typeid( double ) ) );
  rowBuffer.extend<double>( "V_D" );

  description.insertColumn( "V_LD",
                            coral::AttributeSpecification::typeNameForId( typeid( double ) ) );
  rowBuffer.extend<double>( "V_LD" );

  description.insertColumn( "V_BLOB",
                            coral::AttributeSpecification::typeNameForId( typeid( coral::Blob ) ) );
  rowBuffer.extend<coral::Blob>( "V_BLOB" );

  description.insertColumn( "V_DATE",
                            coral::AttributeSpecification::typeNameForId( typeid( coral::Date ) ) );
  rowBuffer.extend<coral::Date>( "V_DATE" );

  description.insertColumn( "V_TIMESTAMP",
                            coral::AttributeSpecification::typeNameForId( typeid( coral::TimeStamp ) ) );
  rowBuffer.extend<coral::TimeStamp>( "V_TIMESTAMP" );

  description.insertColumn( "V_SG",
                            coral::AttributeSpecification::typeNameForId( typeid( std::string ) ) );
  rowBuffer.extend<std::string>( "V_SG" );

  description.insertColumn( "V_SF",
                            coral::AttributeSpecification::typeNameForId( typeid( std::string ) ),
                            5 );
  rowBuffer.extend<std::string>( "V_SF" );

  description.insertColumn( "V_SV",
                            coral::AttributeSpecification::typeNameForId( typeid( std::string ) ),
                            50,
                            false );
  rowBuffer.extend<std::string>( "V_SV" );

  schema.createTable( description ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  session.transaction().commit();

  coral::sleepSeconds(2);

}

void InputOutput::writeSimple()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T1);
  coral::ITableDataEditor& dataEditor = table.dataEditor();
  coral::AttributeList& rowBuffer = *m_rowBuffer;

  for ( int i = 0; i < rowsWithSimpleInsert; ++i )
  {
    this->fillData( i );
    dataEditor.insertRow( rowBuffer );
  }

  session.transaction().commit();
}

void InputOutput::writeBulk()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T1);
  coral::ITableDataEditor& dataEditor = table.dataEditor();
  coral::AttributeList& rowBuffer = *m_rowBuffer;

  coral::IBulkOperation* bulkOperation = dataEditor.bulkInsert( rowBuffer,
                                                                rowsWithSimpleInsert );

  for ( int i = rowsWithSimpleInsert; i < rowsWithSimpleInsert + rowsWithBulkInsert; ++i ) {
    this->fillData( i );
    bulkOperation->processNextIteration();
  }
  bulkOperation->flush();

  delete bulkOperation;

  session.transaction().commit();

}

void InputOutput::read()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start(true);

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T1);
  coral::IQuery* query = table.newQuery();
  query->addToOrderList( "ID" );
  query->defineOutput( *m_rowBuffer );

  std::cout << "Default values" << std::endl;

  m_rowBuffer->toOutputStream(std::cout) << std::endl;

  query->setRowCacheSize( rowsWithSimpleInsert );

  coral::ICursor& cursor = query->execute();
  for ( int i = 0; i < rowsWithSimpleInsert + rowsWithBulkInsert; ++i ) {
    if ( ! cursor.next() )
      throw std::runtime_error( "Unexpected number of returned rows" );

    std::cout << "Values of row " << i << std::endl;

    //  m_rowBuffer->toOutputStream(std::cout) << std::endl;

    this->checkData( i );
  }
  if ( cursor.next() )
    throw std::runtime_error( "Unexpected number of returned rows" );

  delete query;

  session.transaction().commit();

}

void
InputOutput::prepareBigSchema()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( false );

  coral::ISchema& schema = session.nominalSchema();

  schema.dropIfExistsTable(T1);

  coral::TableDescription description;

  description.setName(T1);

  description.insertColumn( "A", coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ) );
  description.insertColumn( "B", coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  description.insertColumn( "C1", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
  description.insertColumn( "C2", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
  description.insertColumn( "D", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description.insertColumn( "E", coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );

  schema.createTable( description );

  session.transaction().commit();

  coral::sleepSeconds(2);

  session.transaction().start( false );

  coral::ITable& table = schema.tableHandle(T1);

  coral::ITableDataEditor& dataEditor = table.dataEditor();

  for(size_t i = 0; i < 20; i++ )
  {
    coral::AttributeList attr;

    attr.extend("A", typeid(coral::TimeStamp));
    attr.extend("B", typeid(double));
    attr.extend("C1", typeid(std::string));
    attr.extend("C2", typeid(std::string));
    attr.extend("D", typeid(int));
    attr.extend("E", typeid(coral::Blob));

    coral::TimeStamp time(2006, 4, 23, 12, i, 6, 23);

    attr[0].data<coral::TimeStamp>() = time;
    attr[1].data<double>() = 23.54 * i;
    attr[2].data<std::string>() = "2009-02-05_09:44:08.269264000 GMT";
    attr[3].data<std::string>() = "2009-02-05_09:44:08.269264000 GMT";
    attr[4].data<int>() = i;
    coral::Blob& myblob = attr[5].data<coral::Blob>();

    size_t maxc = 5000000; //10MB
    myblob.resize( maxc + 1 );
    unsigned char* p = static_cast<unsigned char*>( myblob.startingAddress() );
    for ( size_t j = 0; j < maxc; ++j, ++p ) *p = rand() % 256;

    dataEditor.insertRow( attr );
  }

  session.transaction().commit();
}

void
InputOutput::readBigSchema()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  coral::ISchema& schema = session.nominalSchema();

  coral::ITable& table = schema.tableHandle(T1);

  coral::IQuery* query = table.newQuery();

  coral::AttributeList al01;

  std::cout << std::endl << "Test with RowBuffer at 10 rows per request, output via currentrow" << std::endl;

  coral::ICursor& cursor = query->execute();

  size_t counter = 0;

  while( cursor.next() )
  {
    counter++;
    const coral::AttributeList& alh = cursor.currentRow();

    alh.toOutputStream(std::cout) << std::endl;

  }

  delete query;

  session.transaction().commit();

}

void
InputOutput::prepareTypes()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( false );

  coral::ISchema& schema = session.nominalSchema();

  schema.dropIfExistsTable(T2);

  coral::TableDescription description;

  description.setName(T2);

  description.insertColumn( "C01", coral::AttributeSpecification::typeNameForId( typeid(bool) ) );
  description.insertColumn( "C02", coral::AttributeSpecification::typeNameForId( typeid(char) ) );
  description.insertColumn( "C03", coral::AttributeSpecification::typeNameForId( typeid(unsigned char) ) );
  description.insertColumn( "C04", coral::AttributeSpecification::typeNameForId( typeid(signed char) ) );
  description.insertColumn( "C05", coral::AttributeSpecification::typeNameForId( typeid(short) ) );
  description.insertColumn( "C06", coral::AttributeSpecification::typeNameForId( typeid(unsigned short) ) );
  description.insertColumn( "C07", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  description.insertColumn( "C08", coral::AttributeSpecification::typeNameForId( typeid(unsigned int) ) );
  description.insertColumn( "C09", coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  description.insertColumn( "C0a", coral::AttributeSpecification::typeNameForId( typeid(unsigned long) ) );
  description.insertColumn( "C0b", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  description.insertColumn( "C0c", coral::AttributeSpecification::typeNameForId( typeid(unsigned long long) ) );
  description.insertColumn( "C0d", coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  description.insertColumn( "C0e", coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  description.insertColumn( "C0f", coral::AttributeSpecification::typeNameForId( typeid(double) ) ); //long double
  description.insertColumn( "C10", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
  description.insertColumn( "C11", coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
  description.insertColumn( "C12", coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ) );
  description.insertColumn( "C13", coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );

  schema.createTable( description ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  session.transaction().commit();

  coral::sleepSeconds(2);

  session.transaction().start( false );

  coral::ITable& table = schema.tableHandle(T2);

  coral::ITableDataEditor& dataEditor = table.dataEditor();


  coral::AttributeList attr01;

  attr01.extend("C01", typeid(bool));
  attr01.extend("C02", typeid(char));
  attr01.extend("C03", typeid(unsigned char));
  attr01.extend("C04", typeid(signed char));
  attr01.extend("C05", typeid(short));
  attr01.extend("C06", typeid(unsigned short));
  attr01.extend("C07", typeid(int));
  attr01.extend("C08", typeid(unsigned int));
  attr01.extend("C09", typeid(long));
  attr01.extend("C0a", typeid(unsigned long));
  attr01.extend("C0b", typeid(long long));
  attr01.extend("C0c", typeid(unsigned long long));
  attr01.extend("C0d", typeid(float));
  attr01.extend("C0e", typeid(double));
  attr01.extend("C0f", typeid(double)); //long double
  attr01.extend("C10", typeid(std::string));
  attr01.extend("C11", typeid(coral::Date));
  attr01.extend("C12", typeid(coral::TimeStamp));
  attr01.extend("C13", typeid(coral::Blob));

  //fill the 1. row all empty

  attr01[0].data<bool>() = false;
  attr01[1].data<char>() = 0;
  attr01[2].data<unsigned char>() = 0;
  attr01[3].data<char>() = (signed char)0; //bug #46922
  attr01[4].data<short>() = 0;
  attr01[5].data<unsigned short>() = 0;
  attr01[6].data<int>() = 0;
  attr01[7].data<unsigned int>() = 0;
  attr01[8].data<long>() = 0;
  attr01[9].data<unsigned long>() = 0;
  attr01[10].data<long long>() = 0;
  attr01[11].data<unsigned long long>() = 0;
  attr01[12].data<float>() = 0;
  attr01[13].data<double>() = 0;
  //  attr01[14].data<long double>() = 0;  bug oracle ORA-03115
  attr01[15].data<std::string>() = "";
  attr01[16].data<coral::Date>() = coral::Date(2001, 1, 1);
  attr01[17].data<coral::TimeStamp>() = coral::TimeStamp(1972, 1, 1, 12, 32, 34, 0, false);

  {
    coral::Blob& blob = attr01[18].data<coral::Blob>();
    int blobSize = 1;
    blob.resize( blobSize );
    unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
    for ( int j = 0; j < blobSize; ++j, ++p ) *p = j%256;
  }

  dataEditor.insertRow( attr01 );

  //fill the 2. row the maximum value

  attr01[0].data<bool>() = true;
  attr01[1].data<char>() = (char)0xFF;
  attr01[2].data<unsigned char>() = 0xFF;
  attr01[3].data<char>() = (signed char)0xFF; //bug #46922
  attr01[4].data<short>() = 0xFF;
  attr01[5].data<unsigned short>() = 0xFF;
  attr01[6].data<int>() = INT_MAX;
  attr01[7].data<unsigned int>() = UINT_MAX;
  attr01[8].data<long>() = LONG_MAX;
  attr01[9].data<unsigned long>() = ULONG_MAX;
#ifdef LLONG_MAX
  attr01[10].data<long long>() = LLONG_MAX;
#else
  attr01[10].data<long long>() = LONG_MAX;
#endif
#ifdef ULLONG_MAX
  attr01[11].data<unsigned long long>() = ULLONG_MAX;
#else
  attr01[11].data<unsigned long long>() = ULONG_MAX;
#endif
  attr01[12].data<float>() = (float)-12.42;
  attr01[13].data<double>() = -233.4543;
  //  attr01[14].data<long double>() = -233.4543; bug oracle ORA-03115
  attr01[15].data<std::string>() = "Hello World!";
  attr01[16].data<coral::Date>() = coral::Date(2001, 1, 1);
  attr01[17].data<coral::TimeStamp>() = coral::TimeStamp(1972, 1, 1, 12, 32, 34, 0, false);

  {
    coral::Blob& blob = attr01[18].data<coral::Blob>();
    int blobSize = 1000;
    blob.resize( blobSize );
    unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
    for ( int j = 0; j < blobSize; ++j, ++p ) *p = j%256;
  }

  dataEditor.insertRow( attr01 );


  session.transaction().commit();
}

void
InputOutput::readTypes()
{
  coral::ISessionProxy& session = getSession();

  coral::AttributeList attr02;

  attr02.extend("C01", typeid(bool));
  attr02.extend("C02", typeid(char));
  attr02.extend("C03", typeid(unsigned char));
  attr02.extend("C04", typeid(signed char));
  attr02.extend("C05", typeid(short));
  attr02.extend("C06", typeid(unsigned short));
  attr02.extend("C07", typeid(int));
  attr02.extend("C08", typeid(unsigned int));
  attr02.extend("C09", typeid(long));
  attr02.extend("C0a", typeid(unsigned long));
  attr02.extend("C0b", typeid(long long));
  attr02.extend("C0c", typeid(unsigned long long));
  attr02.extend("C0d", typeid(float));
  attr02.extend("C0e", typeid(double));
  attr02.extend("C0f", typeid(long double));
  attr02.extend("C10", typeid(std::string));
  attr02.extend("C11", typeid(coral::Date));
  attr02.extend("C12", typeid(coral::TimeStamp));
  attr02.extend("C13", typeid(coral::Blob));

  session.transaction().start( true );

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T2);

  coral::IQuery* query = table.newQuery();
  query->defineOutput( attr02 );

  query->defineOutputType( "C01", coral::AttributeSpecification::typeNameForId( typeid(bool) ) );
  query->defineOutputType( "C02", coral::AttributeSpecification::typeNameForId( typeid(char) ) );
  query->defineOutputType( "C03", coral::AttributeSpecification::typeNameForId( typeid(unsigned char) ) );
  query->defineOutputType( "C04", coral::AttributeSpecification::typeNameForId( typeid(signed char) ) );
  query->defineOutputType( "C05", coral::AttributeSpecification::typeNameForId( typeid(short) ) );
  query->defineOutputType( "C06", coral::AttributeSpecification::typeNameForId( typeid(unsigned short) ) );
  query->defineOutputType( "C07", coral::AttributeSpecification::typeNameForId( typeid(int) ) );
  query->defineOutputType( "C08", coral::AttributeSpecification::typeNameForId( typeid(unsigned int) ) );
  query->defineOutputType( "C09", coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  query->defineOutputType( "C0a", coral::AttributeSpecification::typeNameForId( typeid(unsigned long) ) );
  query->defineOutputType( "C0b", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  query->defineOutputType( "C0c", coral::AttributeSpecification::typeNameForId( typeid(unsigned long long) ) );
  query->defineOutputType( "C0d", coral::AttributeSpecification::typeNameForId( typeid(float) ) );
  query->defineOutputType( "C0e", coral::AttributeSpecification::typeNameForId( typeid(double) ) );
  query->defineOutputType( "C0f", coral::AttributeSpecification::typeNameForId( typeid(double) ) ); //long double
  query->defineOutputType( "C10", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );
  query->defineOutputType( "C11", coral::AttributeSpecification::typeNameForId( typeid(coral::Date) ) );
  query->defineOutputType( "C12", coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ) );
  query->defineOutputType( "C13", coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );


  coral::ICursor& cursor = query->execute();
  if( cursor.next() )
  {
    const coral::AttributeList& alh = cursor.currentRow();

    if( alh[0].data<bool>() != false )
      throw;
    if( alh[1].data<char>() != 0 )
      throw;
    if( alh[2].data<unsigned char>() != 0 )
      throw;
    if( alh[3].data<char>() != (signed char)0 ) //bug #46922
      throw;
    if( alh[4].data<short>() != 0 )
      throw;
    if( alh[5].data<unsigned short>() != 0 )
      throw;
    if( alh[6].data<int>() != 0 )
      throw;
    if( alh[7].data<unsigned int>() != 0 )
      throw;
    if( alh[8].data<long>() != 0 )
      throw;
    if( alh[9].data<unsigned long>() != 0 )
      throw;
    if( alh[10].data<long long>() != 0 )
      throw;
    if( alh[11].data<unsigned long long>() != 0 )
      throw;
    if( alh[12].data<float>() != 0 )
      throw;
    if( alh[13].data<double>() != 0 )
      //  attr01[14].data<long double>() = 0;  bug oracle ORA-03115
      if( alh[15].data<std::string>() != "" )
        throw;
    if( alh[16].data<coral::Date>() != coral::Date(2001, 1, 1) )
      throw;
    if( alh[17].data<coral::TimeStamp>() != coral::TimeStamp(1972, 1, 1, 12, 32, 34, 0, false) )
      throw;

  }
  if( cursor.next() )
  {
    const coral::AttributeList& alh = cursor.currentRow();

    if( alh[0].data<bool>() != true )
      throw;
    if( alh[1].data<char>() != (char)0xFF )
      throw;
    if( alh[2].data<unsigned char>() != 0xFF )
      throw;
    if( alh[3].data<char>() != (signed char)0xFF ) //bug #46922
      throw;
    if( alh[4].data<short>() != 0xFF )
      throw;
    if( alh[5].data<unsigned short>() != 0xFF )
      throw;
    if( alh[6].data<int>() != INT_MAX )
      throw;
    if( alh[7].data<unsigned int>() != UINT_MAX )
      throw;
    if( alh[8].data<long>() != LONG_MAX )
      throw;
    if( alh[9].data<unsigned long>() != ULONG_MAX )
      throw;
#ifdef LLONG_MAX
    if( alh[10].data<long long>() != LLONG_MAX )
      throw;
#else
    if( alh[10].data<long long>() != LONG_MAX )
      throw;
#endif
#ifdef ULLONG_MAX
    if( alh[11].data<unsigned long long>() != ULLONG_MAX )
      throw;
#else
    if( alh[11].data<unsigned long long>() != ULONG_MAX )
      throw;
#endif
    if( alh[12].data<float>() != (float)-12.42 )
      throw;
    if( alh[13].data<double>() != -233.4543 )
      //  attr01[14].data<long double>() = 0;  bug oracle ORA-03115
      if( alh[15].data<std::string>() != "Hello World!" )
        throw;
    if( alh[16].data<coral::Date>() != coral::Date(2001, 1, 1) )
      throw;
    if( alh[17].data<coral::TimeStamp>() != coral::TimeStamp(1972, 1, 1, 12, 32, 34, 0, false) )
      throw;

  }

  delete query;

  session.transaction().commit();

}
