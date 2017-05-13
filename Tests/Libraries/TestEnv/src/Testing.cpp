#include <cmath>
#include <sstream>

#include "CoralBase/MessageStream.h"
#include "CoralKernel/Context.h"

#include "TestEnv/Testing.h"

//#include <map>
//#include <set>
//#include <string>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

//#include "RelationalAccess/ConnectionService.h"
//#include "RelationalAccess/IAuthenticationService.h"
//#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IBulkOperation.h"
//#include "RelationalAccess/IBulkOperationWithQuery.h"
#include "RelationalAccess/IColumn.h"
//#include "RelationalAccess/IConnection.h"
//#include "RelationalAccess/IConnectionService.h"
//#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ILookupService.h"
//#include "RelationalAccess/IMonitoringReporter.h"
//#include "RelationalAccess/IMonitoringService.h"
//#include "RelationalAccess/IOperationWithQuery.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IQuery.h"
//#include "RelationalAccess/IQueryDefinition.h"
//#include "RelationalAccess/IRelationalDomain.h"
//#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/ISchema.h"
//#include "RelationalAccess/ISession.h"
//#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/TableDescription.h"

//#include "TestEnv/TestEnv.h"

#define LOG( msg ){ coral::MessageStream myMsg("Testing"); myMsg << coral::Info << msg << coral::MessageStream::endmsg; }

//-----------------------------------------------------------------------------

Testing::Testing(const TestEnv& env)
  : m_env( env )
  , m_connsvc( 0 )
  , m_session( 0 )
  , m_rowBuffer( 0 )
{
  //set default table names
  env.addTablePrefix(T1, "T1");
  env.addTablePrefix(T2, "T2");
  env.addTablePrefix(T3, "T3");
  env.addTablePrefix(V0, "V0");
}

//-----------------------------------------------------------------------------

Testing::~Testing()
{
  deleteSession();
}

//-----------------------------------------------------------------------------

void
Testing::createSession(int index, coral::AccessMode mode, bool sharing)
{
  if ( mode == coral::ReadOnly )
  {
    LOG("Start readonly Session");
  }
  else
  {
    LOG("Start write Session");
  }
  setSession( newSession(index, mode, sharing) );
}

//-----------------------------------------------------------------------------

coral::IConnectionService&
Testing::getConnectionService()
{
  if( !m_connsvc )
  {
    coral::IHandle<coral::IConnectionService> handle;
    handle = coral::Context::instance().query<coral::IConnectionService>("CORAL/Services/ConnectionService");

    if(!handle.isValid())
    {
      coral::Context::instance().loadComponent("CORAL/Services/ConnectionService");
      // Try find out if timing was enabled
      handle = coral::Context::instance().query<coral::IConnectionService>("CORAL/Services/ConnectionService");

      if(!handle.isValid())
      {
        throw std::runtime_error("Can't get handle for ConnectionService");
      }
    }
    m_connsvc = handle.get();
  }
  return *m_connsvc;
}

//-----------------------------------------------------------------------------

coral::ISessionProxy*
Testing::newSession(int index, coral::AccessMode mode, bool sharing)
{
  const std::string& lookup = m_env.lookupFile();
  if( !lookup.empty() )
  {
    coral::IHandle<coral::ILookupService> handle;
    handle = coral::Context::instance().query<coral::ILookupService>("CORAL/Services/XMLLookupService");

    if(!handle.isValid())
    {
      coral::Context::instance().loadComponent("CORAL/Services/XMLLookupService");
      // Try find out if timing was enabled
      handle = coral::Context::instance().query<coral::ILookupService>("CORAL/Services/XMLLookupService");

      if(!handle.isValid())
      {
        throw std::runtime_error("Can't get handle for XMLLookupService");
      }
    }

    handle.get()->setInputFileName( lookup );

  }

  coral::IConnectionService& csv = getConnectionService();

  if(!sharing)
  {
    coral::IConnectionServiceConfiguration& config = csv.configuration();
    config.disableConnectionSharing();
  }

  const std::string& serviceName = m_env.getServiceName(index, mode);

  coral::ISessionProxy* session = csv.connect( serviceName, mode );
  if ( ! session )
    throw std::runtime_error( "Could not connect to " + serviceName );

  return session;
}

//-----------------------------------------------------------------------------

/*
void
Testing::createSession(int index, coral::IHandle<coral::IConnectionService> handle, coral::AccessMode mode)
{
//  deleteSession();
//  session = testCore->createSession(index, handle, mode);
}
*/

 //-----------------------------------------------------------------------------

void
Testing::deleteSession()
{
  if( m_session )
  {
    delete m_session;
    m_session = 0;
  }
}

//-----------------------------------------------------------------------------

void
Testing::setSession(coral::ISessionProxy* session)
{
  deleteSession();

  m_session = session;
}

//-----------------------------------------------------------------------------

std::string
Testing::getSchemaName() const
{
  if( m_session )
  {
    return m_session->nominalSchema().schemaName();
  }
  throw std::runtime_error( "Session was not started" );
}

//-----------------------------------------------------------------------------

/*
bool
Testing::isDBType(const std::string& type)
{
//  return (testCore->getDBType(0).compare(type) == 0);
}
*/

 //-----------------------------------------------------------------------------

void
Testing::fillData( int i )
{
  coral::AttributeList& row = *m_rowBuffer;

  row[0].data<int>() = i;

  if ( i%3 == 0 ) {
    row[1].setNull( true );
  }
  else {
    row[1].setNull( false );
    row[1].data<bool>() = ( i%2 == 0 ) ? true : false;
  }

  row[2].data<unsigned char>() = ( i*2 ) % 256;

  row[3].data<char>() = ( i*2 ) % 128;

  row[4].data<unsigned short>() = ( i*3 ) % 2048;

  row[5].data<short>() = ( i*3 ) % 2048 - 1024;

  row[6].data<unsigned int>() = 2147483648U - i; //( i*4 ) % 1000;

  row[7].data<int>() = ( i*4 ) % 1000 - 500;

  row[8].data<unsigned long>() = ( i*4 ) % 1001;

  row[9].data<long>() = ( i*4 ) % 1001 - 500;

  row[10].data<unsigned long long>() =  2147483648U - i;

  row[11].data<long long>() = ( (i%2 == 0 ) ? ( 2147483648ll - 2*i ) : ( i - 2147483648ll ) );

  if ( i%4 == 0 ) {
    row[12].setNull( true );
  }
  else {
    row[12].setNull( false );
    row[12].data<float>() = i + (float)0.123;
  }

  row[13].data<double>() = 0.123456789 - 2.3*i;

  row[14].data<double>() = 0.123456789 + 2.3*i;

  coral::Blob& blob = row[15].data<coral::Blob>();
  int blobSize = 1000 * ( i%100 + 1 );
  blob.resize( blobSize );
  unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
  for ( int j = 0; j < blobSize; ++j, ++p ) *p = ( i + j )%256;

  row[16].data<coral::Date>() = coral::Date( 2006, 1, i%31 + 1 );

  row[17].data<coral::TimeStamp>() = coral::TimeStamp( 2006, 1, 12, 15, 47, i%60, 0 );

  std::ostringstream os1;
  os1 << "A general String : " << i % 5;
  std::string s1 = os1.str();
  row[18].data<std::string>() = s1;

  std::ostringstream os2;
  os2 << "...." << i % 10;
  std::string s2 = os2.str();
  row[19].data<std::string>() = s2;

  std::ostringstream os3;
  os3 << "Value : " << i;
  std::string s3 = os3.str();
  row[20].data<std::string>() = s3;
}

//-----------------------------------------------------------------------------

void
Testing::checkData( int i ) const
{
  const coral::AttributeList& row = *m_rowBuffer;

  if ( row[0].data<int>() != i )
    throw std::runtime_error( "Unexpected value for variable " + row[0].specification().name() );

  if ( i%3 == 0 ) {
    if ( ! row[1].isNull() )
      throw std::runtime_error( "Unexpected NOT NULL data for variable " + row[1].specification().name() );
  }
  else {
    if ( row[1].isNull() )
      throw std::runtime_error( "Unexpected NULL data for variable " + row[1].specification().name() );
    if ( row[1].data<bool>() != ( ( i%2 == 0 ) ? true : false ) )
      throw std::runtime_error( "Unexpected value for variable " + row[1].specification().name() );
  }

  if ( row[2].data<unsigned char>() != ( i*2 ) % 256 )
    throw std::runtime_error( "Unexpected value for variable " + row[2].specification().name() );

  if ( row[3].data<char>() != ( i*2 ) % 128 )
    throw std::runtime_error( "Unexpected value for variable " + row[3].specification().name() );

  if ( row[4].data<unsigned short>() != ( i*3 ) % 2048 )
    throw std::runtime_error( "Unexpected value for variable " + row[4].specification().name() );

  if ( row[5].data<short>() != ( i*3 ) % 2048 - 1024 )
    throw std::runtime_error( "Unexpected value for variable " + row[5].specification().name() );

  if ( row[6].data<unsigned int>() != static_cast<unsigned int>( 2147483648U - i ) ) // ( i*4 ) % 1000 ) )
    throw std::runtime_error( "Unexpected value for variable " + row[6].specification().name() );

  if ( row[7].data<int>() != ( i*4 ) % 1000 - 500 )
    throw std::runtime_error( "Unexpected value for variable " + row[7].specification().name() );

  if ( row[8].data<unsigned long>() != static_cast<unsigned long>( ( i*4 ) % 1001 ) )
    throw std::runtime_error( "Unexpected value for variable " + row[8].specification().name() );

  // FIXME for some reasons this line results in segfault
  if ( row[9].data<long>() != static_cast<long>( ( i*4 ) % 1001 - 500 ) )
    throw std::runtime_error( "Unexpected value for variable " + row[9].specification().name() );
  //compareValues( row[9].data<long>() , static_cast<long>(( i*4 ) % 1001 - 500), row[9].specification().name());

  if ( row[10].data<unsigned long long>() != static_cast<unsigned long long>( 2147483648U - i ) )
    throw std::runtime_error( "Unexpected value for variable " + row[10].specification().name() );

  if ( row[11].data<long long>() != ( (i%2 == 0 ) ? ( 2147483648ll - 2*i ) : ( i - 2147483648ll ) ) )
    throw std::runtime_error( "Unexpected value for variable " + row[11].specification().name() );

  if ( i%4 == 0 ) {
    if ( ! row[12].isNull() )
      throw std::runtime_error( "Unexpected NOT NULL data for variable " + row[12].specification().name() );
  }
  else {
    if ( row[12].isNull() )
      throw std::runtime_error( "Unexpected NULL data for variable " + row[12].specification().name() );
    if ( std::abs( row[12].data<float>() / ( i + 0.123 ) - 1 ) > std::numeric_limits<float>::epsilon() )
      throw std::runtime_error( "Unexpected value for variable " + row[12].specification().name() );
  }

  if ( std::abs( row[13].data<double>() / ( 0.123456789 - 2.3*i ) - 1 ) > std::numeric_limits<double>::epsilon() )
    throw std::runtime_error( "Unexpected value for variable " + row[13].specification().name() );

  if ( std::abs( row[14].data<double>() / ( 0.123456789 + 2.3*i ) - 1 ) > std::numeric_limits<double>::epsilon() )
    throw std::runtime_error( "Unexpected value for variable " + row[14].specification().name() );

  const coral::Blob& blob = row[15].data<coral::Blob>();
  int blobSize = 1000 * ( i%100 + 1 );
  if ( blob.size() != blobSize )
    throw std::runtime_error( "Unexpected blob size for variable " + row[15].specification().name() );
  const unsigned char* p = static_cast<const unsigned char*>( blob.startingAddress() );
  for ( int j = 0; j < blobSize; ++j, ++p )
    if ( *p != ( i + j )%256 )
      throw std::runtime_error( "Unexpected value for variable " + row[15].specification().name() );

  if ( row[16].data<coral::Date>() != coral::Date( 2006, 1, i%31 + 1 ) )
    throw std::runtime_error( "Unexpected value for variable " + row[16].specification().name() );

  if ( row[17].data<coral::TimeStamp>() != coral::TimeStamp( 2006, 1, 12, 15, 47, i%60, 0 ) )
    throw std::runtime_error( "Unexpected value for variable " + row[17].specification().name() );

  std::ostringstream os1;
  os1 << "A general String : " << i % 5;
  std::string s1 = os1.str();
  if ( row[18].data<std::string>() != s1 )
    throw std::runtime_error( "Unexpected value for variable " + row[16].specification().name() );

  std::ostringstream os2;
  os2 << "...." << i % 10;
  std::string s2 = os2.str();
  if ( row[19].data<std::string>() != s2 )
    throw std::runtime_error( "Unexpected value for variable " + row[17].specification().name() );

  std::ostringstream os3;
  os3 << "Value : " << i;
  std::string s3 = os3.str();
  if ( row[20].data<std::string>() != s3 )
    throw std::runtime_error( "Unexpected value for variable " + row[18].specification().name() );
}

//-----------------------------------------------------------------------------

void
Testing::createSchemaDefault()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  // Get rid of the previous tables and views
  session.nominalSchema().dropIfExistsTable(T3);
  session.nominalSchema().dropIfExistsTable(T2);
  session.nominalSchema().dropIfExistsTable(T1);

  // Create the first table
  coral::TableDescription description1( "SchemaTest_Test" );
  description1.setName(T1);


  // mysql type problem, id should be long instead of int otherwise
  // foreign key constraint will fail of type missmatch
  //    description1.insertColumn( "id",
  //                               coral::AttributeSpecification::typeNameForId( typeid(int) ) );

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
  {
    coral::ITable& table = session.nominalSchema().createTable( description1 );
    // Change the column name
    ///not workin in mysql !!!!
    table.schemaEditor().renameColumn( "Xx", "X" );

    // Change the column type
    table.schemaEditor().changeColumnType( "t", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
    // Add a not null constaint
    table.schemaEditor().setNotNullConstraint( "t" );

    //bug#1 small hack to solve it
    table.schemaEditor().changeColumnType( "t", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
    // Add a unique constraint made out of two columns

    //this is not working for mysql !!!!!!
    std::vector< std::string > constraintColumns( 2 );
    constraintColumns[0] = "X";
    constraintColumns[1] = "Z";
    ///    table.schemaEditor().setUniqueConstraint( constraintColumns );

    /// Add a unique index
    table.schemaEditor().createIndex( T1 + "_IDX_t", "t", true );

    // Give privileges
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
  }

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

  {
    coral::ITable& table = session.nominalSchema().createTable( description2 );

    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
  }

  // Create the third table
  coral::TableDescription description3( "SchemaTest_Test" );
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

  {
    coral::ITable& table = session.nominalSchema().createTable( description3 );

    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
  }

  session.transaction().commit();
  coral::sleepSeconds(2);
}

//-----------------------------------------------------------------------------

void
Testing::readSchemaDefault()
{
  readSchemaDefault(getSchemaName());
}

//-----------------------------------------------------------------------------

void
Testing::readSchemaDefault(std::string SchemaName)
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  // Examine the tables
  this->printTableInfo( session.schema(SchemaName).tableHandle(T1).description() );
  this->printTableInfo( session.schema(SchemaName).tableHandle(T2).description() );
  this->printTableInfo( session.schema(SchemaName).tableHandle(T3).description() );

  session.transaction().commit();
}

//-----------------------------------------------------------------------------

void
Testing::printTableInfo( const coral::ITableDescription& description ) const
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

void
Testing::createViewDefault()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();
  //delete previous view creations
  schema.dropIfExistsView(V0);

  // Create a view

  coral::IViewFactory* factory = schema.viewFactory();

  factory->addToOutputList( T3 + ".id", "id" );
  factory->addToOutputList( T1 + ".X", "x" );
  factory->addToOutputList( T1 + ".Y", "y" );
  factory->addToOutputList( T1 + ".Z", "z" );
  factory->addToOutputList( T2 + ".tx", "TX" );
  factory->addToOutputList( T2 + ".ty", "TY" );
  factory->addToTableList(T3);
  factory->addToTableList(T1);
  factory->addToTableList(T2);

  factory->setCondition( T1 + ".id = " + T3 + ".fk1 AND " + T2 + ".id = " + T3 + ".fk2 AND " + T3 + ".Qn > 0.5", coral::AttributeList() );

  //    factory->setCondition( "T1.id = T3.fk1 AND T2.id = T3.fk2 ",
  //                          coral::AttributeList() );

  //   if(isDBType("mysql")){
  //     factory->create(V0);
  //   }else{
  factory->create(V0).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
  //   }
  //here happens the segmentation fault somewhere
  //.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
  delete factory;

  session.transaction().commit();
}

//-----------------------------------------------------------------------------

void
Testing::fillSchemaDefault()
{
  coral::ISessionProxy& session = getSession();

  int i;

  session.transaction().start();

  coral::ISchema& schema = m_session->nominalSchema();
  coral::ITable& table = schema.tableHandle(T1);
  coral::ITableDataEditor& dataEditor = table.dataEditor();

  if ( m_rowBuffer ) delete m_rowBuffer;
  m_rowBuffer = new coral::AttributeList;
  coral::AttributeList& row = *m_rowBuffer;

  const coral::ITableDescription& description = table.description();
  int numColumns = description.numberOfColumns();
  for ( i = 0; i < numColumns; i++ ) {
    const coral::IColumn& column = description.columnDescription( i );
    row.extend( column.name(), column.type() );
  }

  for ( i = 1; i < 11; i++ ) {
    row[0].data<long long>() = i;
    row[1].data<long long>() = 100 - i;
    row[2].data<float>() = (float)0.123456;
    row[3].data<double>() = 0.1 * i;
    row[4].data<double>() = 0.2;
    row[5].data<std::string>() = "hello world";

    coral::Blob& blob = row[6].data<coral::Blob>();
    int blobSize = 1000 * ( i%100 + 1 );
    blob.resize( blobSize );
    unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
    for ( int j = 0; j < blobSize; ++j, ++p ) *p = ( i + j )%256;

    dataEditor.insertRow( row );
  }

  session.transaction().commit();

}

//-----------------------------------------------------------------------------

void
Testing::printSchemaDefault()
{
  coral::ISessionProxy& session = getSession();

  int i;

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T1);
  coral::IQuery * query = table.newQuery();

  //  if ( m_rowBuffer ) delete m_rowBuffer;
  //  m_rowBuffer = new coral::AttributeList;


  //  int numColumns = description.numberOfColumns();
  //  for ( i = 0; i < numColumns; i++ ) {
  //    const coral::IColumn& column = description.columnDescription( i );
  //    row.extend( column.name(), column.type() );
  //  }

  //  query->addToOrderList( "id" );
  //  std::cout << row.size() << std::endl;
  //  query->defineOutput( *buffer );
  query->setRowCacheSize( 0 );

  coral::ICursor& cursor = query->execute();

  for ( i = 1; i < 11; i++ ) {
    if ( ! cursor.next() ) break;
    const coral::AttributeList& row = cursor.currentRow();
    std::cout << row[0].data<long long>() << "\t" << row[1].data<long long>() << "\t" << row[3].data<double>() << std::endl;

  }

  delete query;

  session.transaction().commit();
}

//-----------------------------------------------------------------------------

void
Testing::createSimpleTable()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  session.nominalSchema().dropIfExistsTable(T1);

  coral::TableDescription description1( "SchemaTest_Test" );
  description1.setName(T1);

  description1.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(long) ) );
  description1.setPrimaryKey( "id" );

  session.nominalSchema().createTable( description1 ).privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  session.transaction().commit();
}

//-----------------------------------------------------------------------------

void
Testing::fillBlob(coral::Blob& blob, size_t maxc)
{
  blob.resize( maxc + 1 );
  unsigned char* p = static_cast<unsigned char*>( blob.startingAddress() );
  for ( size_t j = 0; j < maxc; ++j, ++p ) *p = rand() % 256;
}

//-----------------------------------------------------------------------------

void
Testing::fillString(std::string& s, size_t maxc, size_t offset)
{
  s.clear();

  size_t pos = 0;
  while(pos < maxc)
  {
    char c = ((pos + offset) % 256);
    s = s + c;
    pos++;
  }
}


//-----------------------------------------------------------------------------

void
Testing::writeTableT1(coral::ISessionProxy& session, size_t numberOfFilledRows)
{
  // open an update transaction
  session.transaction().start(false);
  // drop the table if it already exists
  session.nominalSchema().dropIfExistsTable(T1);

  coral::TableDescription description( T1 + "_description" );
  description.setName(T1);

  description.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(unsigned long long) ) );
  description.setPrimaryKey( "id" );
  description.insertColumn( "content", coral::AttributeSpecification::typeNameForId( typeid(std::string) ) );

  coral::ITable& table = session.nominalSchema().createTable( description );
  // set permissions (for frontier)
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );
  coral::IBulkOperation* ins = table.dataEditor().bulkInsert( rowBuffer, 100 );

  for(size_t i = 0; i < numberOfFilledRows; ++i )
  {
    rowBuffer["id"].data<unsigned long long>() = (i + 1);
    // all characters
    rowBuffer["content"].data<std::string>() = "hello world";
    // add row
    ins->processNextIteration();
  }
  ins->flush();

  delete ins;

  session.transaction().commit();
  // Avoid ORA-01466
  coral::sleepSeconds(1);
}

//-----------------------------------------------------------------------------

size_t
Testing::readTableT1(coral::ISessionProxy& session)
{
  // open a read only transaction
  session.transaction().start(true);

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T1);
  coral::IQuery * query = table.newQuery();

  //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
  // coral::MessageStream::setMsgVerbosity( coral::Verbose );

  query->addToOutputList( "id" );
  query->addToOutputList( "content" );

  coral::AttributeList rowBuffer;
  rowBuffer.extend( "id", typeid(unsigned long long));
  rowBuffer.extend( "content", typeid(std::string) );

  query->defineOutput( rowBuffer );

  coral::ICursor& cursor = query->execute();

  size_t counter = 0;
  while(cursor.next())
  {
    counter++;
    // compare the content
    if( rowBuffer["id"].data<unsigned long long>() != counter )
      throw std::runtime_error( "value missmatch in T1 in column 'id'" );
    if( rowBuffer["content"].data<std::string>().compare("hello world") != 0 )
      throw std::runtime_error( "value missmatch in T1 in column 'content'" );
  }
  session.transaction().commit();

  return counter;
}

//-----------------------------------------------------------------------------

void
Testing::writeTableT2(coral::ISessionProxy& session, size_t numberOfFilledRows)
{
  // open an update transaction
  session.transaction().start(false);
  // drop the table if it already exists
  session.nominalSchema().dropIfExistsTable(T2);

  coral::TableDescription description( T2 + "_description" );
  description.setName(T2);

  description.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(unsigned long long) ) );
  description.setPrimaryKey( "id" );
  description.insertColumn( "s01", coral::AttributeSpecification::typeNameForId(typeid(std::string)), 255, false );
  description.insertColumn( "s02", coral::AttributeSpecification::typeNameForId(typeid(std::string)), 255, false );
  description.insertColumn( "d01", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
  description.insertColumn( "d02", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );

  coral::ITypeConverter& typec = session.typeConverter();

  std::set<std::string> types = typec.supportedSqlTypes();
  for(std::set<std::string>::iterator i = types.begin(); i != types.end(); i++ )
  {
    std::cout << *i << std::endl;
  }
  if(types.find("CLOB") != types.end())
  {
    //we can use a clob here
  }
  // create table
  coral::ITable& table = session.nominalSchema().createTable( description );
  // set permissions (for frontier)
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );
  // Get an bulk operator
  coral::IBulkOperation* ins = table.dataEditor().bulkInsert( rowBuffer, 100 );

  for(size_t i = 0; i < numberOfFilledRows; ++i )
  {
    rowBuffer["id"].data<unsigned long long>() = (i + 1);
    //all characters
    rowBuffer["s01"].data<std::string>() = "hello world";
    //all viewable characters
    fillString(rowBuffer["s02"].data<std::string>(), 94, 32);
    //fill a blob with random data
    fillBlob(rowBuffer["d01"].data<coral::Blob>(), 5632);
    //fill a blob with random data
    fillBlob(rowBuffer["d02"].data<coral::Blob>(), 5631);

    ins->processNextIteration();
  }
  ins->flush();

  delete ins;
  // done
  session.transaction().commit();
  // Avoid ORA-01466
  coral::sleepSeconds(1);
}

//-----------------------------------------------------------------------------

size_t
Testing::readTableT2(coral::ISessionProxy& session)
{
  // open a read only transaction
  session.transaction().start(true);

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T2);
  coral::IQuery * query = table.newQuery();

  //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
  // coral::MessageStream::setMsgVerbosity( coral::Verbose );

  query->addToOutputList( "id" );
  query->addToOutputList( "s01" );
  query->addToOutputList( "s02" );
  query->addToOutputList( "d01" );
  query->addToOutputList( "d02" );

  coral::AttributeList rowBuffer;
  rowBuffer.extend( "id", typeid(unsigned long long));
  rowBuffer.extend( "s01", typeid(std::string) );
  rowBuffer.extend( "s02", typeid(std::string) );
  rowBuffer.extend( "d01", typeid(coral::Blob) );
  rowBuffer.extend( "d02", typeid(coral::Blob) );

  query->defineOutput( rowBuffer );

  coral::ICursor& cursor = query->execute();

  size_t counter = 0;
  while(cursor.next())
  {
    counter++;
    // compare the content
    if( rowBuffer["id"].data<unsigned long long>() != counter )
      throw std::runtime_error( "value missmatch in T2 in column 'id'" );
  }
  session.transaction().commit();

  return counter;
}

//-----------------------------------------------------------------------------

void
Testing::writeTableT3(coral::ISessionProxy& session, size_t numberOfFilledRows)
{
  // open an update transaction
  session.transaction().start(false);
  // drop the table if it already exists
  session.nominalSchema().dropIfExistsTable(T3);

  coral::TableDescription description( T3 + "_description" );
  description.setName(T3);

  description.insertColumn( "NODE_ID", coral::AttributeSpecification::typeNameForId(typeid( long long )) );
  description.setPrimaryKey( "NODE_ID" );
  description.insertColumn( "NODE_PARENTID", coral::AttributeSpecification::typeNameForId(typeid(long long)));
  description.insertColumn( "NODE_NAME", coral::AttributeSpecification::typeNameForId(typeid(std::string)));
  description.insertColumn( "NODE_FULLPATH", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "NODE_DESCRIPTION", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "NODE_ISLEAF", coral::AttributeSpecification::typeNameForId(typeid(long long)) );
  description.insertColumn( "NODE_SCHEMA_VERSION", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "NODE_INSTIME", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "LASTMOD_DATE", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "FOLDER_VERSIONING", coral::AttributeSpecification::typeNameForId(typeid(long long)) );
  description.insertColumn( "FOLDER_PAYLOADSPEC", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
  description.insertColumn( "FOLDER_PAYLOAD_INLINE", coral::AttributeSpecification::typeNameForId(typeid(long long)) );
  description.insertColumn( "FOLDER_PAYLOAD_EXTREF", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
  description.insertColumn( "FOLDER_CHANNELSPEC", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
  description.insertColumn( "FOLDER_CHANNEL_EXTREF", coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
  description.insertColumn( "FOLDER_IOVTABLENAME", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "FOLDER_TAGTABLENAME", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "FOLDER_IOV2TAGTABLENAME", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  description.insertColumn( "FOLDER_CHANNELTABLENAME", coral::AttributeSpecification::typeNameForId(typeid(std::string)) );

  // create table
  coral::ITable& table = session.nominalSchema().createTable( description );
  // set permissions (for frontier)
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );
  // Local row buffer
  coral::AttributeList rowBuffer;
  // Setup the row buffer
  table.dataEditor().rowBuffer( rowBuffer );
  // Get an bulk operator
  coral::IBulkOperation* ins = table.dataEditor().bulkInsert( rowBuffer, 100 );

  for(size_t i = 0; i < numberOfFilledRows; i++ )
  {
    rowBuffer[0].data<long long>() = (i + 1);
    rowBuffer[1].data<long long>() = (i + 1) * 2;
    rowBuffer[2].data<std::string>() = "My Node";
    fillString( rowBuffer[3].data<std::string>(), 94, 32);
    fillString( rowBuffer[4].data<std::string>(), 94, 32);
    rowBuffer[5].data<long long>() = 0;
    fillString( rowBuffer[6].data<std::string>(), 94, 32);
    fillString( rowBuffer[7].data<std::string>(), 94, 32);
    fillString( rowBuffer[8].data<std::string>(), 94, 32);
    rowBuffer[9].data<long long>() = (i + 1) * 4;
    fillBlob(rowBuffer[10].data<coral::Blob>(), 5632);
    rowBuffer[11].data<long long>() = (i + 1) * 4;
    fillBlob(rowBuffer[12].data<coral::Blob>(), 5631);
    fillBlob(rowBuffer[13].data<coral::Blob>(), 5631);
    fillBlob(rowBuffer[14].data<coral::Blob>(), 5631);
    fillString( rowBuffer[15].data<std::string>(), 94, 32);
    fillString( rowBuffer[16].data<std::string>(), 94, 32);
    fillString( rowBuffer[17].data<std::string>(), 94, 32);
    rowBuffer[18].data<std::string>() = "MyTable";

    ins->processNextIteration();
  }

  ins->flush();

  delete ins;

  session.transaction().commit();
  // Avoid ORA-01466
  coral::sleepSeconds(1);
}

//-----------------------------------------------------------------------------

size_t
Testing::readTableT3(coral::ISessionProxy& session)
{
  // open a read only transaction
  session.transaction().start(true);

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T2);
  coral::IQuery * query = table.newQuery();

  //  coral::MsgLevel oldLevel = coral::MessageStream::msgVerbosity();
  // coral::MessageStream::setMsgVerbosity( coral::Verbose );
  /*
  query->addToOutputList( "NODE_ID" );
  query->addToOutputList( "NODE_PARENTID" );
  query->addToOutputList( "FOLDER_PAYLOADSPEC" );
  query->addToOutputList( "FOLDER_IOV2TAGTABLENAME" );
  */
  coral::ICursor& cursor = query->execute();

  size_t counter = 0;
  while(cursor.next())
  {
    counter++;
  }
  session.transaction().commit();

  return counter;
}

//-----------------------------------------------------------------------------
