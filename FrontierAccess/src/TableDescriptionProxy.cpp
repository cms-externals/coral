// $Id: TableDescriptionProxy.cpp,v 1.12 2011/03/22 10:36:50 avalassi Exp $
#include <sstream>

#include "TableDescriptionProxy.h"
#include "ISessionProperties.h"
#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "Statement.h"
#include "OracleTableBuilder.h"
#include "ColumnProxy.h"

#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/ITypeConverter.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

#include "CoralKernel/Service.h"
#include "CoralBase/MessageStream.h"

coral::FrontierAccess::TableDescriptionProxy::TableDescriptionProxy( const coral::FrontierAccess::ISessionProperties& properties, const std::string& tableName )
  : m_sessionProperties( properties )
  , m_description( new coral::TableDescription( properties.domainProperties().service()->name() ) )
  , m_columnDescriptionRead( false )
  , m_columns()
  , m_uniqueConstraintsRead( false )
  , m_primaryKeyRead( false )
  , m_indicesRead( false )
  , m_foreignKeysRead( false )
{
  m_description->setName( tableName );
}

coral::FrontierAccess::TableDescriptionProxy::~TableDescriptionProxy()
{
  for ( std::vector< coral::FrontierAccess::ColumnProxy* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn ) delete *iColumn;

  delete m_description;
}

std::string coral::FrontierAccess::TableDescriptionProxy::name() const
{
  return m_description->name();
}

std::string coral::FrontierAccess::TableDescriptionProxy::type() const
{
  return m_description->type();
}

void coral::FrontierAccess::TableDescriptionProxy::readColumnDescription()
{
  if ( m_columnDescriptionRead )
    return;

  std::string describeSql =
    "SELECT COLUMN_ID, COLUMN_NAME, NULLABLE, DATA_TYPE, DATA_LENGTH, DATA_PRECISION, CHAR_LENGTH, DATA_SCALE \
     FROM ALL_TAB_COLUMNS \
     WHERE OWNER=:\"schema\" AND TABLE_NAME=:\"table\" ORDER BY COLUMN_ID";

  coral::FrontierAccess::Statement statement( m_sessionProperties, describeSql );

  coral::AttributeList inputData;
  inputData.extend<std::string>( "schema" ); inputData["schema"].data<std::string>() = m_sessionProperties.schemaName();
  inputData.extend<std::string>( "table" ) ; inputData["table" ].data<std::string>() =  this->name();

  coral::AttributeList outputData;

  outputData.extend<int>        ( "COLUMN_ID"        ); //const int&         columnId        = outputData["COLUMN_ID"     ].data<int>();
  outputData.extend<std::string>( "COLUMN_NAME"      ); const std::string& columnName      = outputData["COLUMN_NAME"   ].data<std::string>();
  outputData.extend<std::string>( "NULLABLE"         ); const std::string& columnNullable  = outputData["NULLABLE"      ].data<std::string>();
  outputData.extend<std::string>( "DATA_TYPE"        ); const std::string& columnType      = outputData["DATA_TYPE"     ].data<std::string>();
  outputData.extend<int>        ( "DATA_LENGTH"      ); const int&         columnSize      = outputData["DATA_LENGTH"   ].data<int>();
  outputData.extend<int>        ( "DATA_PRECISION"   ); const int&         columnPrecision = outputData["DATA_PRECISION"].data<int>();
  outputData.extend<int>        ( "CHAR_LENGTH"      ); //const int&         columnLength    = outputData["CHAR_LENGTH"   ].data<int>();
  outputData.extend<int>        ( "DATA_SCALE"       ); const int&         columnScale     = outputData["DATA_SCALE"    ].data<int>();

  if ( ! statement.execute( inputData, false ) )
  {
    // FIXME - error reporting
    ;
  }

  statement.defineOutput( outputData );

  // Retrieving the number of columns
  unsigned short numberOfColumns = statement.numberOfRowsProcessed();

  std::string columnTypeName;

  // Retrieving the column list
  while( statement.fetchNext() )
  {
    bool isNull = ( (columnNullable == "Y") ? true : false );
    //std::cout << "FrontierAccess::TableDescriptionProxy::readColumnDescription: name=" << columnName << ", type=" << columnType << ", precision=" << columnPrecision << std::endl;

    if ( columnType == "VARCHAR2" || columnType == "VARCHAR" || columnType == "CLOB" )
    {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(std::string) );
    }
    else if ( columnType == "DATE" )
    {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(coral::Date) );
    }
    else if ( columnType.find( "TIMESTAMP" ) != std::string::npos )
    {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) );
    }
    else if ( columnType.find( "CHAR" ) != std::string::npos )
    {
      if ( columnSize == 1 )
      {
        columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(char) );
      }
      else
      {
        columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(std::string) );
      }
    }
    else if ( columnType == "NUMBER" )
    {
      std::ostringstream os;
      if ( columnPrecision == 126 )
      { // This is a float
        os << "FLOAT(" << static_cast<int>(columnPrecision) << ")";
      }
      else if( columnPrecision == 63 )
      { // This is a float(REAL)
        os << "FLOAT(" << static_cast<int>(columnPrecision) << ")";
      }
      else {
        os << "NUMBER";
        if ( columnPrecision != 0 ) {
          os << "(" << static_cast<int>(columnPrecision);
          if ( columnScale != 0 ) {
            os << "," << static_cast<int>(columnScale);
          }
          os << ")";
        }
        else // Take "NUMBER" to mean "NUMBER(38)" (fix bug #70208)
        {
          os << "(38)";
        }
      }
      std::string sqlType = os.str();
      columnTypeName = m_sessionProperties.typeConverter().cppTypeForSqlType( sqlType );
    }
    else if ( columnType == "FLOAT" && columnPrecision == 63 ) // Fix bug #70208
    {
      columnTypeName = m_sessionProperties.typeConverter().cppTypeForSqlType( "FLOAT(63)" );
    }
    else if ( columnType == "FLOAT" && columnPrecision == 126 ) // Fix bug #70208
    {
      columnTypeName = m_sessionProperties.typeConverter().cppTypeForSqlType( "FLOAT(126)" );
    }
    else if ( columnType == "BINARY_FLOAT" )
    {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(float) );
    }
    else if ( columnType == "BINARY_DOUBLE" )
    {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(double) );
    }
    else if ( columnType == "BLOB" )
    {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) );
    }
    else
    {
      std::stringstream msg;
      msg << "Cannot handle type '" << columnType
          << "' with precision '" << columnPrecision
          << "' of column '" << columnName << "' in table "
          << m_sessionProperties.schemaName() << "." << m_description->name();
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Error << msg.str() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), msg.str(), "TableDescriptionProxy::readColumnDescription" );
    }

    bool fixedSize = true;
    int size = 0;
    if ( columnTypeName == coral::AttributeSpecification::typeNameForId( typeid(std::string) ) )
    {
      size = columnSize;
      if ( columnType == "VARCHAR2" || columnType == "VARCHAR" || columnType == "CLOB" )
        fixedSize = false;
    }

    m_description->insertColumn( columnName, columnTypeName, size, fixedSize );

    if ( isNull )
    {
      m_description->setNotNullConstraint( columnName );
    }
  }

  // Construct the column proxies
  for ( int i = 0; i < numberOfColumns; ++i )
    m_columns.push_back( new coral::FrontierAccess::ColumnProxy( m_description->columnDescription( i ), *this ) );

  m_columnDescriptionRead = true;
}

void coral::FrontierAccess::TableDescriptionProxy::refreshConstraints() const
{
  if ( m_uniqueConstraintsRead )
    return;

  const_cast< coral::FrontierAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  std::ostringstream os;
  os << "SELECT ALL_CONSTRAINTS.CONSTRAINT_NAME AS CONSTRAINT_NAME, "
     << "ALL_CONS_COLUMNS.COLUMN_NAME AS COLUMN_NAME, "
     << "ALL_INDEXES.TABLESPACE_NAME AS TABLESPACE_NAME "
     << "FROM ALL_CONSTRAINTS, ALL_CONS_COLUMNS, ALL_INDEXES "
     << "WHERE ALL_CONSTRAINTS.OWNER = :\"OWNER\" "
     << "AND ALL_CONSTRAINTS.TABLE_NAME = :\"TABLE_NAME\" "
     << "AND ALL_CONSTRAINTS.CONSTRAINT_TYPE = 'U' "
     << "AND ALL_CONS_COLUMNS.OWNER = ALL_CONSTRAINTS.OWNER "
     << "AND ALL_CONS_COLUMNS.TABLE_NAME = ALL_CONSTRAINTS.TABLE_NAME "
     << "AND ALL_INDEXES.OWNER = ALL_CONSTRAINTS.OWNER "
     << "AND ALL_INDEXES.TABLE_NAME = ALL_CONSTRAINTS.TABLE_NAME "
     << "AND ALL_CONS_COLUMNS.CONSTRAINT_NAME = ALL_CONSTRAINTS.CONSTRAINT_NAME " // The two
     << "AND ALL_INDEXES.INDEX_NAME = ALL_CONS_COLUMNS.CONSTRAINT_NAME " // join conditions
     << "ORDER BY ALL_CONSTRAINTS.CONSTRAINT_NAME, ALL_CONS_COLUMNS.POSITION";
  coral::FrontierAccess::Statement statement( m_sessionProperties, os.str() );
  coral::AttributeList* bindData = new coral::AttributeList;
  bindData->extend<std::string>( "OWNER" );
  bindData->extend<std::string>( "TABLE_NAME" );
  (*bindData)[0].data<std::string>() = m_sessionProperties.schemaName();
  (*bindData)[1].data<std::string>() = m_description->name();

  if ( ! statement.execute( *bindData, false ) )
  {
    throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not retrieve the unique constraints defined in a table", "TableDescriptionProxy::refreshConstraints" );
  }

  delete bindData; // I have no idea why the static crashes...

  coral::AttributeList output;
  output.extend<std::string>( "CONSTRAINT_NAME" );
  output.extend<std::string>( "COLUMN_NAME" );
  output.extend<std::string>( "TABLESPACE_NAME" );
  statement.defineOutput( output );
  const std::string& constraintName = output[0].data<std::string>();
  const std::string& columnName = output[1].data<std::string>();
  const std::string& tableSpaceName = output[2].data<std::string>();
  std::map< std::string, std::pair< std::string, std::vector< std::string > > > constraints;
  while( statement.fetchNext() )
  {
    std::map< std::string, std::pair< std::string, std::vector< std::string > > >::iterator iConstraint = constraints.find( constraintName );

    if ( iConstraint == constraints.end() )
    {
      iConstraint = constraints.insert( std::make_pair( constraintName, std::make_pair( tableSpaceName, std::vector< std::string >() ) ) ).first;
    }
    iConstraint->second.second.push_back( columnName );
  }

  for ( std::map< std::string, std::pair< std::string, std::vector< std::string > > >::const_iterator iConstraint = constraints.begin(); iConstraint != constraints.end(); ++iConstraint )
  {
    m_description->setUniqueConstraint( iConstraint->second.second, iConstraint->first, true, iConstraint->second.first );
  }

  // The indices should be refreshed as well!!
  this->refreshIndexInfo();
  this->refreshPrimaryKeyInfo();

  m_uniqueConstraintsRead = true;
}

void coral::FrontierAccess::TableDescriptionProxy::refreshPrimaryKeyInfo() const
{
  if ( m_primaryKeyRead ) return;

  const_cast< coral::FrontierAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  std::ostringstream os;
  os << "SELECT "
     << "B.COLUMN_NAME AS COLUMN_NAME, "
     << "C.TABLESPACE_NAME AS TABLESPACE_NAME "
     << "FROM "
     << "ALL_CONSTRAINTS A, "
     << "ALL_CONS_COLUMNS B, "
     << "ALL_INDEXES C "
     << "WHERE "
     << "A.OWNER=:\"OWNER\" "
     << "AND A.TABLE_NAME=:\"TABLE_NAME\" "
     << "AND A.CONSTRAINT_TYPE = 'P' "
     << "AND B.OWNER = A.OWNER "
     << "AND B.TABLE_NAME = A.TABLE_NAME "
     << "AND B.CONSTRAINT_NAME = A.CONSTRAINT_NAME "
     << "AND B.CONSTRAINT_NAME = C.INDEX_NAME "
     << "AND C.OWNER = B.OWNER "
     << "AND C.TABLE_NAME = B.TABLE_NAME "
     << "ORDER BY B.POSITION";
  coral::FrontierAccess::Statement statement( m_sessionProperties, os.str() );
  coral::AttributeList* bindData = new coral::AttributeList;
  bindData->extend<std::string>( "OWNER" );
  bindData->extend<std::string>( "TABLE_NAME" );
  (*bindData)[0].data<std::string>() = m_sessionProperties.schemaName();
  (*bindData)[1].data<std::string>() = m_description->name();

  if ( ! statement.execute( *bindData, false ) )
  {
    throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not retrieve the primary key information defined in a table", "TableDescriptionProxy::refreshPrimaryKeyInfo" );
  }

  delete bindData; // I have no idea why the static crashes...

  coral::AttributeList output;
  output.extend<std::string>( "COLUMN_NAME" );
  output.extend<std::string>( "TABLESPACE_NAME" );
  statement.defineOutput( output );
  const std::string& columnName = output[0].data<std::string>();
  const std::string& tableSpaceName = output[1].data<std::string>();
  std::pair<std::string, std::vector<std::string> > pk = std::make_pair( std::string(""), std::vector<std::string>() );

  while( statement.fetchNext() )
  {
    pk.first = tableSpaceName;
    pk.second.push_back( columnName );
  }

  if ( ! pk.first.empty() )
    m_description->setPrimaryKey( pk.second, pk.first );

  m_primaryKeyRead = true;
}

// A simple structure to hold the index information
namespace coral
{
  namespace FrontierAccess
  {
    class IndexInfo
    {
    public:
      IndexInfo( bool _unique = false, std::string tsName = "" ) : unique(_unique), tableSpaceName( tsName ), columns() { }
      bool unique;
      std::string tableSpaceName;
      std::vector<std::string> columns;
    };
  }
}

void coral::FrontierAccess::TableDescriptionProxy::refreshIndexInfo() const
{
  if ( m_indicesRead )
    return;

  const_cast< coral::FrontierAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  std::ostringstream os;
  os << "SELECT "
     << "A.INDEX_NAME AS INDEX_NAME, "
     << "A.UNIQUENESS AS UNIQUENESS, "
     << "A.TABLESPACE_NAME AS TABLESPACE_NAME, "
     << " B.COLUMN_NAME AS COLUMN_NAME "
     << "FROM "
     << "ALL_INDEXES A, "
     << "ALL_IND_COLUMNS B "
     << "WHERE "
     << "A.OWNER =:\"OWNER\" "
     << "AND A.TABLE_NAME =:\"TABLE_NAME\" "
     << "AND A.INDEX_NAME = B.INDEX_NAME "
     << "AND B.INDEX_OWNER = A.OWNER "
     << "AND B.TABLE_NAME = A.TABLE_NAME "
     << "AND A.INDEX_NAME NOT IN "
     << "( SELECT CONSTRAINT_NAME "
     << "FROM ALL_CONSTRAINTS "
     << "WHERE "
     << "OWNER = A.OWNER "
     << "AND TABLE_NAME = A.TABLE_NAME "
     << "AND ( CONSTRAINT_TYPE = 'P' OR CONSTRAINT_TYPE = 'U' ) ) "
     << "ORDER BY A.INDEX_NAME, B.COLUMN_POSITION";

  coral::FrontierAccess::Statement statement( m_sessionProperties, os.str() );

  coral::AttributeList bindData; // = new coral::AttributeList;
  bindData.extend<std::string>( "OWNER" );      bindData["OWNER"].data<std::string>() = m_sessionProperties.schemaName();
  bindData.extend<std::string>( "TABLE_NAME" ); bindData["TABLE_NAME"].data<std::string>() = m_description->name();

  coral::AttributeList output;
  output.extend<std::string>( "INDEX_NAME" )     ; const std::string& indexName      = output["INDEX_NAME"     ].data<std::string>();
  output.extend<std::string>( "UNIQUENESS" )     ; const std::string& uniqueness     = output["UNIQUENESS"     ].data<std::string>();
  output.extend<std::string>( "TABLESPACE_NAME" ); const std::string& tableSpaceName = output["TABLESPACE_NAME"].data<std::string>();
  output.extend<std::string>( "COLUMN_NAME" )    ; const std::string& columnName     = output["COLUMN_NAME"    ].data<std::string>();

  if ( ! statement.execute( bindData, false ) )
    throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not retrieve the index information defined in a table", "TableDescriptionProxy::refreshPrimaryKeyInfo" );

  statement.defineOutput( output );

  // Read the index information
  std::map< std::string, coral::FrontierAccess::IndexInfo > indices;

  while( statement.fetchNext() )
  {
    std::map< std::string, IndexInfo >::iterator index = indices.find( indexName );

    if ( index == indices.end() )
    {
      index = indices.insert( std::make_pair( indexName, coral::FrontierAccess::IndexInfo( ( uniqueness == "UNIQUE") ? true : false, tableSpaceName ) ) ).first;
    }

    index->second.columns.push_back( columnName );
  }

  for ( std::map< std::string, coral::FrontierAccess::IndexInfo >::const_iterator index = indices.begin(); index != indices.end(); ++index )
  {
    m_description->createIndex( index->first, index->second.columns, index->second.unique, index->second.tableSpaceName );
  }

  m_indicesRead = true;
}

// A simple structure to hold the foreign key information
namespace coral
{
  namespace FrontierAccess
  {
    class ForeignKeyInfo
    {
    public:
      ForeignKeyInfo( std::string _tableName = "" ) : tableName(_tableName), columns(), refColumns() { }
      std::string tableName;
      std::vector<std::string> columns;
      std::vector<std::string> refColumns;
    };
  }
}

void coral::FrontierAccess::TableDescriptionProxy::refreshForeignKeyInfo() const
{
  const_cast< coral::FrontierAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  std::ostringstream os;
  os << "SELECT "
     << "A.CONSTRAINT_NAME AS FK_NAME, "
     << "B.TABLE_NAME AS REF_TABLE, "
     << "C.COLUMN_NAME AS FK_COL, "
     << "D.COLUMN_NAME AS REF_COL "
     << "FROM "
     << "(SELECT CONSTRAINT_NAME, R_CONSTRAINT_NAME FROM ALL_CONSTRAINTS WHERE OWNER=:\"OWNER\" AND TABLE_NAME =:\"TABLE_NAME\" AND CONSTRAINT_TYPE='R') A, "
     << "(SELECT TABLE_NAME, CONSTRAINT_NAME FROM ALL_CONSTRAINTS WHERE OWNER=:\"OWNER\") B, "
     << "(SELECT COLUMN_NAME, CONSTRAINT_NAME, POSITION FROM ALL_CONS_COLUMNS WHERE OWNER=:\"OWNER\" AND TABLE_NAME=:\"TABLE_NAME\" ORDER BY POSITION) C, "
     << "(SELECT COLUMN_NAME, CONSTRAINT_NAME, TABLE_NAME, POSITION FROM ALL_CONS_COLUMNS WHERE OWNER=:\"OWNER\") D "
     << "WHERE "
     << "A.R_CONSTRAINT_NAME = B.CONSTRAINT_NAME "
     << "AND A.CONSTRAINT_NAME = C.CONSTRAINT_NAME "
     << "AND B.CONSTRAINT_NAME = D.CONSTRAINT_NAME "
     << "AND C.POSITION = D.POSITION "
     << "ORDER BY FK_NAME, D.POSITION";

  coral::FrontierAccess::Statement statement( m_sessionProperties, os.str() );

  coral::AttributeList bindData;

  bindData.extend<std::string>( "OWNER" )     ; bindData["OWNER"     ].data<std::string>() = m_sessionProperties.schemaName();
  bindData.extend<std::string>( "TABLE_NAME" ); bindData["TABLE_NAME"].data<std::string>() = m_description->name();

  coral::AttributeList output;

  output.extend<std::string>( "FK_NAME"   ); const std::string& foreignKeyName       = output["FK_NAME"  ].data<std::string>();
  output.extend<std::string>( "REF_TABLE" ); const std::string& referencedTableName  = output["REF_TABLE"].data<std::string>();
  output.extend<std::string>( "FK_COL"    ); const std::string& foreignKeyColumnName = output["FK_COL"   ].data<std::string>();
  output.extend<std::string>( "REF_COL"   ); const std::string& referencedColumnName = output["REF_COL"  ].data<std::string>();

  if ( ! statement.execute( bindData, false ) )
  {
    throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not retrieve the foreign key information defined in a table", "TableDescriptionProxy::refreshForeignKeyInfo" );
  }

  statement.defineOutput( output );

  // Read the foreign key information
  std::map< std::string, coral::FrontierAccess::ForeignKeyInfo > fkeys;
  while( statement.fetchNext() )
  {
    std::map< std::string, ForeignKeyInfo >::iterator fkey = fkeys.find( foreignKeyName );
    if ( fkey == fkeys.end() )
    {
      fkey = fkeys.insert( std::make_pair( foreignKeyName, coral::FrontierAccess::ForeignKeyInfo( referencedTableName ) ) ).first;
    }
    fkey->second.columns.push_back( foreignKeyColumnName );
    fkey->second.refColumns.push_back( referencedColumnName );
  }

  for ( std::map< std::string, coral::FrontierAccess::ForeignKeyInfo >::const_iterator iKey = fkeys.begin(); iKey != fkeys.end(); ++iKey )
  {
    m_description->createForeignKey( iKey->first, iKey->second.columns, iKey->second.tableName, iKey->second.refColumns );
  }

  m_foreignKeysRead = true;
}

std::string coral::FrontierAccess::TableDescriptionProxy::tableSpaceName() const
{
  std::string tableSpaceName = m_description->tableSpaceName();
  if ( tableSpaceName.empty() )
  {
    coral::FrontierAccess::Statement statement( m_sessionProperties, "SELECT TABLESPACE_NAME FROM ALL_TABLES WHERE OWNER=:\"owner\" AND TABLE_NAME=:\"table_name\"" );

    coral::AttributeList bindData;
    bindData.extend<std::string>( "owner" );
    bindData.extend<std::string>( "table_name" );
    bindData[0].data<std::string>() = m_sessionProperties.schemaName();
    bindData[1].data<std::string>() = m_description->name();

    if ( ! statement.execute( bindData, false ) )
    {
      throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not retrieve the tablespace for a table", "ITableDescription::tableSpaceName" );
    }

    coral::AttributeList outputData;
    outputData.extend<std::string>( "TABLESPACE_NAME" );
    statement.defineOutput( outputData );
    statement.fetchNext();
    tableSpaceName = outputData.begin()->data<std::string>();
    const_cast< coral::TableDescription* >( m_description )->setTableSpaceName( tableSpaceName );
  }
  return tableSpaceName;
}

int coral::FrontierAccess::TableDescriptionProxy::numberOfColumns() const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::FrontierAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  return m_description->numberOfColumns();
}

const coral::IColumn& coral::FrontierAccess::TableDescriptionProxy::columnDescription( int columnIndex ) const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::FrontierAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  if ( columnIndex < 0 || columnIndex >= static_cast<int>( m_columns.size() ) )
    throw coral::InvalidColumnIndexException( m_sessionProperties.domainProperties().service()->name() );
  return *( m_columns[ columnIndex ] );
}

const coral::IColumn& coral::FrontierAccess::TableDescriptionProxy::columnDescription( const std::string& columnName ) const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::FrontierAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  const coral::IColumn* pColumn = 0;
  for ( std::vector< coral::FrontierAccess::ColumnProxy* >::const_iterator iColumn = m_columns.begin(); iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == columnName )
    {
      pColumn = *iColumn;
      break;
    }

  if ( pColumn == 0 )
    throw coral::InvalidColumnNameException( m_sessionProperties.domainProperties().service()->name() );
  return *pColumn;
}

bool coral::FrontierAccess::TableDescriptionProxy::hasPrimaryKey() const
{
  if ( ! m_primaryKeyRead )
    this->refreshPrimaryKeyInfo();
  return m_description->hasPrimaryKey();
}

const coral::IPrimaryKey& coral::FrontierAccess::TableDescriptionProxy::primaryKey() const
{
  if ( ! m_primaryKeyRead )
    this->refreshPrimaryKeyInfo();
  return m_description->primaryKey();
}

int coral::FrontierAccess::TableDescriptionProxy::numberOfIndices() const
{
  if ( ! m_indicesRead )
    this->refreshIndexInfo();
  return m_description->numberOfIndices();
}

const coral::IIndex& coral::FrontierAccess::TableDescriptionProxy::index( int indexId ) const
{
  if ( ! m_indicesRead )
    this->refreshIndexInfo();
  return m_description->index( indexId );
}

int coral::FrontierAccess::TableDescriptionProxy::numberOfForeignKeys() const
{
  if ( ! m_foreignKeysRead )
    this->refreshForeignKeyInfo();
  return m_description->numberOfForeignKeys();
}

const coral::IForeignKey& coral::FrontierAccess::TableDescriptionProxy::foreignKey( int foreignKeyIdentifier ) const
{
  if ( ! m_foreignKeysRead )
    this->refreshForeignKeyInfo();
  return m_description->foreignKey( foreignKeyIdentifier );
}

int coral::FrontierAccess::TableDescriptionProxy::numberOfUniqueConstraints() const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::FrontierAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  this->refreshConstraints();
  return m_description->numberOfUniqueConstraints();
}

const coral::IUniqueConstraint& coral::FrontierAccess::TableDescriptionProxy::uniqueConstraint( int uniqueConstraintIdentifier ) const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::FrontierAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  this->refreshConstraints();
  return m_description->uniqueConstraint( uniqueConstraintIdentifier );
}
