#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include <memory>

#include "oci.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "CoralKernel/Service.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/TableDescription.h"

#include "ColumnProxy.h"
#include "DomainProperties.h"
#include "SessionProperties.h"
#include "OracleErrorHandler.h"
#include "OracleStatement.h"
#include "OracleTableBuilder.h"
#include "TableDescriptionProxy.h"

coral::OracleAccess::TableDescriptionProxy::TableDescriptionProxy( boost::shared_ptr<const SessionProperties> properties,
                                                                   const std::string& schemaName,
                                                                   OCIDescribe* ociDescribeHandle,
                                                                   const std::string& tableName ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
  m_ociDescribeHandle( ociDescribeHandle ),
  m_description( new coral::TableDescription( properties->domainServiceName() ) ),
  m_columnDescriptionRead( false ),
  m_columns(),
  m_uniqueConstraintsRead( false ),
  m_primaryKeyRead( false ),
  m_indicesRead( false ),
  m_foreignKeysRead( false ),
  m_mutex()
{
  m_description->setName( tableName );
}


coral::OracleAccess::TableDescriptionProxy::~TableDescriptionProxy()
{
  for ( std::vector< coral::OracleAccess::ColumnProxy* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn ) delete *iColumn;

  delete m_description;

  OCIHandleFree( m_ociDescribeHandle, OCI_HTYPE_DESCRIBE );
}


std::string
coral::OracleAccess::TableDescriptionProxy::name() const
{
  return m_description->name();
}


std::string
coral::OracleAccess::TableDescriptionProxy::type() const
{
  return m_description->type();
}


void
coral::OracleAccess::TableDescriptionProxy::readColumnDescription()
{
  boost::mutex::scoped_lock lock( m_mutex );
  if ( m_columnDescriptionRead ) return;

  // Retrieving the describe parameter handle
  OCIParam* ociParamHandle = 0;
  sword status = OCIAttrGet( m_ociDescribeHandle, OCI_HTYPE_DESCRIBE,
                             &ociParamHandle, 0, OCI_ATTR_PARAM,
                             m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the describe parameter for a table" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the OCI describe parameter for a table",
                                  "TableDescriptionProxy::readColumnDescription" );
  }

  // Retrieving the column list
  OCIParam* columnList = 0;
  status = OCIAttrGet( ociParamHandle, OCI_DTYPE_PARAM,
                       &columnList, 0, OCI_ATTR_LIST_COLUMNS,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the column list for a table" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the column list for a table",
                                  "TableDescriptionProxy::readColumnDescription" );
  }


  // Retrieving the number of columns
  ub2 numberOfColumns = 0;
  status = OCIAttrGet( ociParamHandle, OCI_DTYPE_PARAM,
                       &numberOfColumns, 0, OCI_ATTR_NUM_COLS,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the number of columns in table " + m_schemaName + "." + m_description->name() );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the number of columns in a table",
                                  "TableDescriptionProxy::readColumnDescription" );
  }



  for ( ub4 iColumn = 1; iColumn <= numberOfColumns; ++iColumn )
  {
    // Retrieving the column parameter
    void* temporaryPointer = 0;
    status = OCIParamGet( columnList, OCI_DTYPE_PARAM, m_sessionProperties->ociErrorHandle(),
                          &temporaryPointer, iColumn );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the parameter of a column in table " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the parameter of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }
    OCIParam* columnParameter = static_cast< OCIParam* >( temporaryPointer );

    // Retrieving the column name
    text* textPlaceHolder = 0;
    ub4 textSize = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &textPlaceHolder, &textSize,
                         OCI_ATTR_NAME,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the name of a column in table " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the name of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }
    /*
    std::ostringstream osColumnName;
    osColumnName << textPlaceHolder;
    std::string columnName = osColumnName.str();
    columnName = columnName.substr( 0, textSize );
    */
    std::string columnName( (char*)textPlaceHolder, textSize );

    // Retrieving the column type
    ub2 columnType = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &columnType, 0,
                         OCI_ATTR_DATA_TYPE,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the type of a column in table " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the type of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }

    // Retrieving the precision of the column
    ub1 precision = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &precision, 0,
                         OCI_ATTR_PRECISION,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the precision of a column in table " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the precision of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }

    // Retrieving the scale of the column
    sb1 scale = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &scale, 0,
                         OCI_ATTR_SCALE,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the scale of a column in table " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the scale of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }

    // Retrieving the data size of the column
    ub4 columnSize = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &columnSize, 0,
                         OCI_ATTR_DATA_SIZE,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the size of a column in table " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the size of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }

    // Retrieving the nullness of the column
    ub1 isNull= 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &isNull, 0,
                         OCI_ATTR_IS_NULL,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the null-ness of a column in table " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the null-ness of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }

    //std::cout << "OracleAccess::TableDescriptionProxy::readColumnDescription: name=" << columnName << ", type=" << columnType << ", precision=" << (int)precision << std::endl;

    std::string columnTypeName = "";
    if ( columnType == OCI_TYPECODE_VARCHAR2 ||
         columnType == OCI_TYPECODE_VARCHAR ||
         columnType == OCI_TYPECODE_CLOB ) {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(std::string) );
    }
    else if ( columnType == OCI_TYPECODE_DATE ) {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(coral::Date) );
    }
    else if ( columnType == OCI_TYPECODE_TIMESTAMP ) {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) );
    }
    else if ( columnType == OCI_TYPECODE_TIMESTAMP_TZ ) {
      columnTypeName = "TIMESTAMP WITH TIME ZONE";
    }
    else if ( columnType == OCI_TYPECODE_CHAR ) {
      if ( columnSize == 1 ) {
        columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(char) );
      }
      else {
        columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(std::string) );
      }
    }
    else if ( columnType == OCI_TYPECODE_NUMBER ) {
      std::ostringstream os;
      if ( scale == -127 ) { // This is a float
        os << "FLOAT(" << static_cast<int>(precision) << ")";
      }
      else {
        os << "NUMBER";
        if ( precision != 0 ) {
          os << "(" << static_cast<int>(precision);
          if ( scale != 0 ) {
            os << "," << static_cast<int>(scale);
          }
          os << ")";
        }
      }
      std::string sqlType = os.str();
      columnTypeName = m_sessionProperties->cppTypeForSqlType( sqlType );
    }
    else if ( columnType == OCI_TYPECODE_BFLOAT ) {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(float) );
    }
    else if ( columnType == OCI_TYPECODE_BDOUBLE ) {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(double) );
    }
    else if ( columnType == OCI_TYPECODE_BLOB ) {
      columnTypeName = coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) );
    }
    else {
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << "Cannot handle the type of column " << columnName << " in table "
          << m_schemaName << "." << m_description->name() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not handle the type of a column in a table",
                                    "TableDescriptionProxy::readColumnDescription" );
    }

    bool fixedSize = true;
    int size = 0;
    if ( columnTypeName == coral::AttributeSpecification::typeNameForId( typeid(std::string) ) ) {
      size = columnSize;
      if ( columnType == OCI_TYPECODE_VARCHAR2 ||
           columnType == OCI_TYPECODE_VARCHAR ||
           columnType == OCI_TYPECODE_CLOB ) fixedSize = false;
      if ( columnType == OCI_TYPECODE_CLOB ) size = 100000;
    }
    if ( columnTypeName == coral::AttributeSpecification::typeNameForId( typeid(coral::TimeStamp) ) ) {
      size = scale;
    }
    m_description->insertColumn( columnName, columnTypeName, size, fixedSize );

    if ( isNull == 0 ) {
      m_description->setNotNullConstraint( columnName );
    }
  }

  /// Construct the column proxies.
  for ( int i = 0; i < numberOfColumns; ++i )
    m_columns.push_back( new coral::OracleAccess::ColumnProxy( m_description->columnDescription( i ),
                                                               *this ) );

  m_columnDescriptionRead = true;
}


void
coral::OracleAccess::TableDescriptionProxy::refreshConstraints() const
{
  const_cast< coral::OracleAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  {
    boost::mutex::scoped_lock lock( m_mutex );
    if ( m_uniqueConstraintsRead ) return;

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
    coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
    coral::AttributeList bindData;
    bindData.extend<std::string>( "OWNER" );
    bindData.extend<std::string>( "TABLE_NAME" );
    bindData[0].data<std::string>() = m_schemaName;
    bindData[1].data<std::string>() = m_description->name();
    if ( ! statement.execute( bindData ) ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the unique constraints defined in a table",
                                    "TableDescriptionProxy::refreshConstraints" );
    }

    coral::AttributeList output;
    output.extend<std::string>( "CONSTRAINT_NAME" );
    output.extend<std::string>( "COLUMN_NAME" );
    output.extend<std::string>( "TABLESPACE_NAME" );
    statement.defineOutput( output );
    const std::string& constraintName = output[0].data<std::string>();
    const std::string& columnName = output[1].data<std::string>();
    const std::string& tableSpaceName = output[2].data<std::string>();
    std::map< std::string, std::pair< std::string, std::vector< std::string > > > constraints;
    while( statement.fetchNext() ) {
      std::map< std::string, std::pair< std::string, std::vector< std::string > > >::iterator iConstraint = constraints.find( constraintName );
      if ( iConstraint == constraints.end() ) {
        iConstraint = constraints.insert( std::make_pair( constraintName, std::make_pair( tableSpaceName, std::vector< std::string >() ) ) ).first;
      }
      iConstraint->second.second.push_back( columnName );
    }

    for ( std::map< std::string, std::pair< std::string, std::vector< std::string > > >::const_iterator iConstraint = constraints.begin();
          iConstraint != constraints.end(); ++iConstraint ) {
      m_description->setUniqueConstraint( iConstraint->second.second,
                                          iConstraint->first,
                                          true,
                                          iConstraint->second.first );
    }

    m_uniqueConstraintsRead = true;
  }

  // The indices should be refreshed as well!!
  this->refreshIndexInfo();
  this->refreshPrimaryKeyInfo();
}


void
coral::OracleAccess::TableDescriptionProxy::refreshPrimaryKeyInfo() const
{
  const_cast< coral::OracleAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );
  if ( m_primaryKeyRead ) return;

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
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  coral::AttributeList bindData;
  bindData.extend<std::string>( "OWNER" );
  bindData.extend<std::string>( "TABLE_NAME" );
  bindData[0].data<std::string>() = m_schemaName;
  bindData[1].data<std::string>() = m_description->name();
  if ( ! statement.execute( bindData ) ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the primary key information defined in a table",
                                  "TableDescriptionProxy::refreshPrimaryKeyInfo" );
  }

  coral::AttributeList output;
  output.extend<std::string>( "COLUMN_NAME" );
  output.extend<std::string>( "TABLESPACE_NAME" );
  statement.defineOutput( output );
  const std::string& columnName = output[0].data<std::string>();
  const std::string& tableSpaceName = output[1].data<std::string>();
  std::pair<std::string, std::vector<std::string> > pk = std::make_pair( std::string(""),
                                                                         std::vector<std::string>() );
  while( statement.fetchNext() ) {
    pk.first = tableSpaceName;
    pk.second.push_back( columnName );
  }

  if ( ! pk.first.empty() )
    m_description->setPrimaryKey( pk.second, pk.first );

  m_primaryKeyRead = true;
}


// A simple structure to hold the index information
namespace coral {
  namespace OracleAccess {
    class IndexInfo {
    public:
      IndexInfo( bool _unique = false,
                 std::string tsName = "" ) : unique(_unique),
                                             tableSpaceName( tsName ),
                                             columns()
      {}
      bool unique;
      std::string tableSpaceName;
      std::vector<std::string> columns;
    };
  }
}


void
coral::OracleAccess::TableDescriptionProxy::refreshIndexInfo() const
{
  const_cast< coral::OracleAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );
  if ( m_indicesRead ) return;

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

  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  coral::AttributeList bindData;
  bindData.extend<std::string>( "OWNER" );
  bindData.extend<std::string>( "TABLE_NAME" );
  bindData[0].data<std::string>() = m_schemaName;
  bindData[1].data<std::string>() = m_description->name();
  if ( ! statement.execute( bindData ) ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the index information defined in a table",
                                  "TableDescriptionProxy::refreshPrimaryKeyInfo" );
  }

  coral::AttributeList output;
  output.extend<std::string>( "INDEX_NAME" );
  output.extend<std::string>( "UNIQUENESS" );
  output.extend<std::string>( "TABLESPACE_NAME" );
  output.extend<std::string>( "COLUMN_NAME" );
  statement.defineOutput( output );
  const std::string& indexName = output[0].data<std::string>();
  const std::string& uniqueness = output[1].data<std::string>();
  const std::string& tableSpaceName = output[2].data<std::string>();
  const std::string& columnName = output[3].data<std::string>();

  // Read the index information
  std::map< std::string, coral::OracleAccess::IndexInfo > indices;
  while( statement.fetchNext() ) {
    std::map< std::string, IndexInfo >::iterator index = indices.find( indexName );
    if ( index == indices.end() ) {
      index = indices.insert( std::make_pair( indexName,
                                              coral::OracleAccess::IndexInfo( ( uniqueness == "UNIQUE") ? true : false,
                                                                              tableSpaceName ) ) ).first;
    }
    index->second.columns.push_back( columnName );
  }

  for ( std::map< std::string, coral::OracleAccess::IndexInfo >::const_iterator index = indices.begin();
        index != indices.end(); ++index ) {
    m_description->createIndex( index->first,
                                index->second.columns,
                                index->second.unique,
                                index->second.tableSpaceName );
  }

  m_indicesRead = true;
}



// A simple structure to hold the foreign key information
namespace coral {
  namespace OracleAccess {
    class ForeignKeyInfo {
    public:
      ForeignKeyInfo( std::string _tableName = "" ) : tableName(_tableName),
                                                      columns(),
                                                      refColumns()
      {}
      std::string tableName;
      std::vector<std::string> columns;
      std::vector<std::string> refColumns;
    };
  }
}




void
coral::OracleAccess::TableDescriptionProxy::refreshForeignKeyInfo() const
{
  const_cast<coral::OracleAccess::TableDescriptionProxy *>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );

  coral::AttributeList bindData;
  bindData.extend<std::string>( "OWNER" );
  bindData.extend<std::string>( "TABLE_NAME" );
  bindData[0].data<std::string>() = m_schemaName;
  bindData[1].data<std::string>() = m_description->name();

  std::auto_ptr<coral::OracleAccess::OracleStatement> pStatement;

  // Task #10775 (performance optimization for data dictionary queries)
  // Second optimization (requires read access to sys tables)
  // The selectAnyTable is initially true but is set to false if query fails
  if ( m_sessionProperties->selectAnyTable() )
  {
    std::ostringstream os;
    os << "select con.name as FK_NAME, ro.name as REF_TABLE, "
       << "col.name as FK_COL, rcol.name as REF_COL "
       << "from sys.user$ u, sys.obj$ o, sys.cdef$ cd, "
       << "sys.ccol$ cc, sys.col$ col, sys.con$ con, "
       << "sys.obj$ ro, sys.ccol$ rcc, sys.col$ rcol "
       << "where u.name=:OWNER "
       << "and o.owner#=u.user# and o.name=:TABLE_NAME and o.type#=2 "
       << "and cd.obj#=o.obj# and cd.type#=4 "
       << "and cc.con#=cd.con# "
       << "and col.obj#=cc.obj# and col.col#=cc.col# "
       << "and con.con#=cc.con# "
       << "and ro.obj#=cd.robj# "
       << "and rcc.con#=cd.rcon# and rcc.pos#=cc.pos# "
       << "and rcol.obj#=cd.robj# and rcol.col#=rcc.col# "
       << "order by cc.con#, cc.pos#"; // << need "ORDER BY FK_NAME, cc.pos#"?
    //std::cout << std::endl << "EXECUTE: " << os.str() << std::endl;
    pStatement.reset( new coral::OracleAccess::OracleStatement( m_sessionProperties, m_schemaName, os.str() ) );
    if ( ! pStatement->execute( bindData ) )
      m_sessionProperties->cannotSelectAnyTable();
  }

  // Task #10775 (performance optimization for data dictionary queries)
  // First optimization (valid for all users)
  // The selectAnyTable has been set to false if the first attempt failed
  if ( !m_sessionProperties->selectAnyTable() )
  {
    std::ostringstream os;
    os << "SELECT /* user has no SELECT ANY TABLE privileges */"
       <<  " A.CONSTRAINT_NAME AS FK_NAME,"
       <<  " D.TABLE_NAME AS REF_TABLE,"
       <<  " C.COLUMN_NAME AS FK_COL,"
       <<  " D.COLUMN_NAME AS REF_COL"
       <<  " FROM"
       <<  " ALL_CONSTRAINTS A,"
       <<  " ALL_CONS_COLUMNS C,"
       <<  " ALL_CONS_COLUMNS D"
       <<  " WHERE"
       <<  " A.OWNER=:\"OWNER\" AND A.TABLE_NAME=:\"TABLE_NAME\""
       <<  " AND A.CONSTRAINT_TYPE='R'"
       <<  " AND A.R_OWNER=:\"OWNER\"" // new but implied (A.R_OWNER=D.OWNER)
       <<  " AND C.OWNER=:\"OWNER\" AND C.TABLE_NAME=:\"TABLE_NAME\""
       <<  " AND C.CONSTRAINT_NAME=A.CONSTRAINT_NAME"
       <<  " AND D.OWNER=A.R_OWNER AND D.CONSTRAINT_NAME=A.R_CONSTRAINT_NAME"
       <<  " AND D.POSITION=C.POSITION"
       <<  " ORDER BY FK_NAME, C.POSITION";
    //std::cout << std::endl << "EXECUTE: " << os.str() << std::endl;

    pStatement.reset( new coral::OracleAccess::OracleStatement( m_sessionProperties, m_schemaName, os.str() ) );
    if ( ! pStatement->execute( bindData ) )
    {
      pStatement.reset();
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the foreign key information defined in a table",
                                    "TableDescriptionProxy::refreshForeignKeyInfo" );
    }
  }

  coral::AttributeList output;
  output.extend<std::string>( "FK_NAME" );
  output.extend<std::string>( "REF_TABLE" );
  output.extend<std::string>( "FK_COL" );
  output.extend<std::string>( "REF_COL" );
  pStatement->defineOutput( output );
  const std::string& foreignKeyName = output[0].data<std::string>();
  const std::string& referencedTableName = output[1].data<std::string>();
  const std::string& foreignKeyColumnName = output[2].data<std::string>();
  const std::string& referencedColumnName= output[3].data<std::string>();

  // Read the foreign key information
  std::map< std::string, coral::OracleAccess::ForeignKeyInfo > fkeys;
  while( pStatement->fetchNext() ) {
    //std::cout << output << std::endl;
    std::map< std::string, ForeignKeyInfo >::iterator fkey = fkeys.find( foreignKeyName );
    if ( fkey == fkeys.end() ) {
      fkey = fkeys.insert( std::make_pair( foreignKeyName,
                                           coral::OracleAccess::ForeignKeyInfo( referencedTableName ) ) ).first;
    }
    fkey->second.columns.push_back( foreignKeyColumnName );
    fkey->second.refColumns.push_back( referencedColumnName );
  }
  pStatement.reset();

  for ( std::map< std::string, coral::OracleAccess::ForeignKeyInfo >::const_iterator iKey = fkeys.begin();
        iKey != fkeys.end(); ++iKey ) {
    m_description->createForeignKey( iKey->first,
                                     iKey->second.columns,
                                     iKey->second.tableName,
                                     iKey->second.refColumns );
  }

  m_foreignKeysRead = true;
}


std::string
coral::OracleAccess::TableDescriptionProxy::tableSpaceName() const
{
  std::string tableSpaceName = m_description->tableSpaceName();
  if ( tableSpaceName.empty() ) {
    boost::mutex::scoped_lock lock( m_mutex );

    coral::OracleAccess::OracleStatement statement( m_sessionProperties,
                                                    m_schemaName,
                                                    "SELECT TABLESPACE_NAME FROM ALL_TABLES WHERE OWNER=:\"owner\" AND TABLE_NAME=:\"table_name\"" );
    coral::AttributeList bindData;
    bindData.extend<std::string>( "owner" );
    bindData.extend<std::string>( "table_name" );
    bindData[0].data<std::string>() = m_schemaName;
    bindData[1].data<std::string>() = m_description->name();
    if ( ! statement.execute( bindData ) ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the tablespace for a table",
                                    "ITableDescription::tableSpaceName" );
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


int
coral::OracleAccess::TableDescriptionProxy::numberOfColumns() const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  return m_description->numberOfColumns();
}


const coral::IColumn&
coral::OracleAccess::TableDescriptionProxy::columnDescription( int columnIndex ) const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  if ( columnIndex < 0 || columnIndex >= static_cast<int>( m_columns.size() ) )
    throw coral::InvalidColumnIndexException( m_sessionProperties->domainServiceName() );
  return *( m_columns[ columnIndex ] );
}


const coral::IColumn&
coral::OracleAccess::TableDescriptionProxy::columnDescription( const std::string& columnName ) const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  const coral::IColumn* pColumn = 0;
  for ( std::vector< coral::OracleAccess::ColumnProxy* >::const_iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == columnName ) {
      pColumn = *iColumn;
      break;
    }

  if ( pColumn == 0 )
    throw coral::InvalidColumnNameException( m_sessionProperties->domainServiceName() );
  return *pColumn;
}


bool
coral::OracleAccess::TableDescriptionProxy::hasPrimaryKey() const
{
  if ( ! m_primaryKeyRead ) this->refreshPrimaryKeyInfo();
  return m_description->hasPrimaryKey();
}


const coral::IPrimaryKey&
coral::OracleAccess::TableDescriptionProxy::primaryKey() const
{
  if ( ! m_primaryKeyRead ) this->refreshPrimaryKeyInfo();
  return m_description->primaryKey();
}


int
coral::OracleAccess::TableDescriptionProxy::numberOfIndices() const
{
  if ( ! m_indicesRead ) this->refreshIndexInfo();
  return m_description->numberOfIndices();
}


const coral::IIndex&
coral::OracleAccess::TableDescriptionProxy::index( int indexId ) const
{
  if ( ! m_indicesRead ) this->refreshIndexInfo();
  return m_description->index( indexId );
}


int
coral::OracleAccess::TableDescriptionProxy::numberOfForeignKeys() const
{
  if ( ! m_foreignKeysRead ) this->refreshForeignKeyInfo();
  return m_description->numberOfForeignKeys();
}


const coral::IForeignKey&
coral::OracleAccess::TableDescriptionProxy::foreignKey( int foreignKeyIdentifier ) const
{
  if ( ! m_foreignKeysRead ) this->refreshForeignKeyInfo();
  return m_description->foreignKey( foreignKeyIdentifier );
}


int
coral::OracleAccess::TableDescriptionProxy::numberOfUniqueConstraints() const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  this->refreshConstraints();
  boost::mutex::scoped_lock lock( m_mutex );
  return m_description->numberOfUniqueConstraints();
}


const coral::IUniqueConstraint&
coral::OracleAccess::TableDescriptionProxy::uniqueConstraint( int uniqueConstraintIdentifier ) const
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  this->refreshConstraints();
  boost::mutex::scoped_lock lock( m_mutex );
  return m_description->uniqueConstraint( uniqueConstraintIdentifier );
}


void
coral::OracleAccess::TableDescriptionProxy::insertColumn( const std::string& name,
                                                          const std::string& type,
                                                          int size,
                                                          bool fixedSize,
                                                          std::string tableSpaceName )
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );

  m_description->insertColumn( name,
                               type,
                               size,
                               fixedSize,
                               tableSpaceName );

  std::string sqltype = coral::OracleAccess::OracleTableBuilder::sqlType( m_sessionProperties,
                                                                          type,
                                                                          size,
                                                                          fixedSize );
  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" << m_description->name() << "\" ADD ( \"" << name << "\" " << sqltype << " )";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList() ) ) {
    m_description->dropColumn( name );
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not add a new column to a table",
                                  "ITableSchemaEditor::insertColumn" );
  }

  m_columns.push_back( new coral::OracleAccess::ColumnProxy( m_description->columnDescription( m_columns.size() ),
                                                             *this ) );
}


void
coral::OracleAccess::TableDescriptionProxy::dropColumn( const std::string& name )
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );

  m_description->dropColumn( name );

  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" << m_description->name() << "\" DROP COLUMN \"" << name << "\"";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList() ) ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not drop a column from a table",
                                  "ITableSchemaEditor::dropColumn" );
  }

  // Reconstruct the column proxies
  for ( std::vector< coral::OracleAccess::ColumnProxy* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn ) delete *iColumn;
  m_columns.clear();
  int numberOfColumns = m_description->numberOfColumns();
  for ( int i = 0; i < numberOfColumns; ++i )
    m_columns.push_back( new coral::OracleAccess::ColumnProxy( m_description->columnDescription( i ),
                                                               *this ) );
}


void
coral::OracleAccess::TableDescriptionProxy::renameColumn( const std::string& originalName,
                                                          const std::string& newName )
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );

  m_description->renameColumn( originalName,
                               newName );
  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" << m_description->name()
     << "\" RENAME COLUMN \"" << originalName << "\" TO \"" << newName << "\"";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList() ) ) {
    m_description->renameColumn( newName,
                                 originalName );
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not rename a column in a table",
                                  "ITableSchemaEditor::dropColumn" );
  }
}


void
coral::OracleAccess::TableDescriptionProxy::changeColumnType( const std::string& columnName,
                                                              const std::string& typeName,
                                                              int size,
                                                              bool fixedSize )
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );

  m_description->changeColumnType( columnName,
                                   typeName,
                                   size,
                                   fixedSize );

  std::string sqltype = coral::OracleAccess::OracleTableBuilder::sqlType( m_sessionProperties,
                                                                          typeName,
                                                                          size,
                                                                          fixedSize );
  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" << m_description->name() << "\" MODIFY ( \"" << columnName << "\" " << sqltype << " )";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList() ) ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not change the type of a column in a table",
                                  "ITableSchemaEditor::changeColumnType" );
  }
}


void
coral::OracleAccess::TableDescriptionProxy::setNotNullConstraint( const std::string& columnName,
                                                                  bool isNotNull )
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  boost::mutex::scoped_lock lock( m_mutex );

  m_description->setNotNullConstraint( columnName,
                                       isNotNull );

  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" << m_description->name() << "\" MODIFY ( \"" << columnName << "\" ";
  if ( isNotNull ) os << "NOT ";
  os << "NULL )";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList() ) ) {
    m_description->setNotNullConstraint( columnName,
                                         ! isNotNull );
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not change the NOT NULL constraint of a column in a table",
                                  "ITableSchemaEditor::setNotNullConstraint" );
  }

}


void
coral::OracleAccess::TableDescriptionProxy::setUniqueConstraint( const std::string& columnName,
                                                                 std::string name,
                                                                 bool isUnique,
                                                                 std::string tableSpaceName )
{
  this->setUniqueConstraint( std::vector<std::string>( 1, columnName ),
                             name,
                             isUnique,
                             tableSpaceName );
}


void
coral::OracleAccess::TableDescriptionProxy::setUniqueConstraint( const std::vector<std::string>& columnNames,
                                                                 std::string name,
                                                                 bool isUnique,
                                                                 std::string tableSpaceName )
{
  if ( ! m_columnDescriptionRead )
    const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  this->refreshConstraints();

  boost::mutex::scoped_lock lock( m_mutex );

  m_description->setUniqueConstraint( columnNames,
                                      name,
                                      isUnique,
                                      tableSpaceName );

  if ( isUnique ) {
    std::ostringstream os;
    os << "ALTER TABLE " << m_schemaName << ".\"" + m_description->name() << "\" ADD ( ";
    if ( ! name.empty() ) {
      os << "CONSTRAINT \"" << name << "\" ";
    }
    os << "UNIQUE ( ";
    for ( std::vector<std::string>::const_iterator iColumn = columnNames.begin();
          iColumn != columnNames.end(); ++iColumn ) {
      if ( iColumn != columnNames.begin() ) os << ", ";
      os << "\"" << *iColumn << "\"";
    }
    os << " ) )";
    std::string tableSpace = tableSpaceName;
    if ( tableSpace.empty() )
      tableSpace = m_sessionProperties->domainProperties().tableSpaceForIndices();
    if ( ! tableSpace.empty() )
      os << " USING INDEX TABLESPACE \"" << tableSpace << "\"";
    coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
    if ( ! statement.execute( coral::AttributeList() ) ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not create a unique constraint in the table",
                                    "ITableSchemaEditor::setUniqueConstraint" );
    }
  }
  else {
    std::ostringstream os;
    os << "ALTER TABLE " << m_schemaName << ".\"" + m_description->name() << "\" DROP ";
    if ( name.empty() ) {
      os << "UNIQUE ( \"" << columnNames[0] << "\" )";
    }
    else {
      os << "CONSTRAINT \"" << name << "\"";
    }
    coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
    if ( ! statement.execute( coral::AttributeList() ) ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not drop a unique constraint from the table",
                                    "ITableSchemaEditor::setUniqueConstraint" );
    }
  }
}


void
coral::OracleAccess::TableDescriptionProxy::setPrimaryKey( const std::string& columnName,
                                                           std::string tableSpaceName )
{
  this->setPrimaryKey( std::vector< std::string >( 1, columnName ), tableSpaceName );
}


void
coral::OracleAccess::TableDescriptionProxy::setPrimaryKey( const std::vector<std::string>& columnNames,
                                                           std::string tableSpaceName )
{
  boost::mutex::scoped_lock lock( m_mutex );

  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" + m_description->name() << "\" ADD ( CONSTRAINT \""
     << m_description->name() << "_PK\" PRIMARY KEY ( ";
  for ( std::vector<std::string>::const_iterator iColumn = columnNames.begin();
        iColumn != columnNames.end(); ++iColumn ) {
    if ( iColumn != columnNames.begin() ) os << ", ";
    os << "\"" << *iColumn << "\"";
  }
  os << " )";
  std::string tableSpace = tableSpaceName;
  if ( tableSpace.empty() )
    tableSpace = m_sessionProperties->domainProperties().tableSpaceForIndices();
  if ( ! tableSpace.empty() )
    os << " USING INDEX TABLESPACE \"" << tableSpace << "\"";
  os << " )";
  sb4 errorCode = 0;
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList(), &errorCode ) ) 
  {
    // Something went wrong, and we can't find out which exception to throw 
    // just based on the oracle error code. Read the PK info and find out.
    // FIXME what about the mutex?
    if ( ! m_columnDescriptionRead )
      const_cast<coral::OracleAccess::TableDescriptionProxy*>( this )->readColumnDescription();
    if ( !m_primaryKeyRead) refreshPrimaryKeyInfo();
    // this will throw the correct exception
    m_description->setPrimaryKey( columnNames, tableSpaceName );
    // fallback, throw general exception
    m_description->dropPrimaryKey();
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not add a primary key constraint on a table",
                                  "ITableSchemaEditor::setPrimaryKey" );
  }
  if ( m_primaryKeyRead )
  {
    m_description->setPrimaryKey( columnNames, tableSpaceName );
  }
}


void
coral::OracleAccess::TableDescriptionProxy::dropPrimaryKey()
{
  boost::mutex::scoped_lock lock( m_mutex );
  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" + m_description->name() << "\" DROP PRIMARY KEY";
  sb4 errorCode = 0;
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList(), &errorCode ) ) 
  {
    if ( errorCode == 2441 ) 
    {
      // ORA-02441:  Cannot drop nonexistent primary key 
      throw NoPrimaryKeyException( m_sessionProperties->domainServiceName(),
                                   "TableDescriptionProxy::dropPrimaryKey()");
    }
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not drop a primary key constraint from the table",
                                  "ITableSchemaEditor::dropPrimaryKey" );
  }
  if ( m_primaryKeyRead )
    m_description->dropPrimaryKey();
}


void
coral::OracleAccess::TableDescriptionProxy::createIndex( const std::string& indexName,
                                                         const std::string& columnName,
                                                         bool isUnique,
                                                         std::string tableSpaceName )
{
  this->createIndex( indexName, std::vector<std::string>( 1, columnName ), isUnique, tableSpaceName );
}


void
coral::OracleAccess::TableDescriptionProxy::createIndex( const std::string& name,
                                                         const std::vector<std::string>& columnNames,
                                                         bool isUnique,
                                                         std::string tableSpaceName )
{
  boost::mutex::scoped_lock lock( m_mutex );
  std::ostringstream os;
  os << "CREATE ";
  if ( isUnique ) os << "UNIQUE ";
  os << "INDEX " << m_schemaName << ".\"" << name << "\" ON "
     << m_schemaName << ".\"" << m_description->name() << "\" ( ";
  for ( std::vector<std::string>::const_iterator iColumn = columnNames.begin();
        iColumn != columnNames.end(); ++iColumn ) {
    if ( iColumn != columnNames.begin() ) os << ", ";
    os << "\"" << *iColumn << "\"";
  }
  os << " )";
  std::string tableSpace = tableSpaceName;
  if ( tableSpace.empty() )
    tableSpace = m_sessionProperties->domainProperties().tableSpaceForIndices();
  if ( ! tableSpace.empty() )
    os << " TABLESPACE \"" << tableSpace << "\"";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  sb4 errorCode = 0;
  if ( ! statement.execute( coral::AttributeList(), &errorCode ) ) 
  {
    if ( errorCode == 955 )
    {
      // ORA-00955: name is already used by an existing object 
      throw coral::InvalidIndexIdentifierException( m_sessionProperties->domainServiceName() );
    } 
    else if ( errorCode == 904 )
    {
      // ORA-00904: "XYZ": invalid identifier 
      throw coral::InvalidColumnNameException( m_sessionProperties->domainServiceName() );
    } 
    else if ( errorCode == 1408 )
    {
      // ORA-01408: such column list already indexed
      throw coral::UniqueConstraintAlreadyExistingException( m_sessionProperties->domainServiceName() );
    }
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not create an index on the table",
                                  "ITableSchemaEditor::createIndex" );
  }
  if ( m_indicesRead )
    m_description->createIndex( name,
                                columnNames,
                                isUnique,
                                tableSpaceName );
}


void
coral::OracleAccess::TableDescriptionProxy::dropIndex( const std::string& indexName )
{
  boost::mutex::scoped_lock lock( m_mutex );

  coral::OracleAccess::OracleStatement statement( m_sessionProperties,
                                                  m_schemaName,
                                                  "DROP INDEX " + m_schemaName + ".\"" + indexName + "\"" );
  sb4 errorCode = 0;
  if ( ! statement.execute( coral::AttributeList(), &errorCode ) )
  {
    if ( errorCode == 1418 )
    {
      //  ORA-01418: specified index does not exist
      throw coral::InvalidIndexIdentifierException( m_sessionProperties->domainServiceName() );
    }
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not drop an index from the table",
                                  "ITableSchemaEditor::dropIndex" );
  }
  if ( m_indicesRead )
    m_description->dropIndex( indexName );
}

void
coral::OracleAccess::TableDescriptionProxy::createForeignKey( const std::string& name,
                                                              const std::string& columnName,
                                                              const std::string& referencedTableName,
                                                              const std::string& referencedColumnName
#ifdef CORAL240DC
                                                              , bool onDeleteCascade
#endif
                                                            )
{
  this->createForeignKey( name,
                          std::vector< std::string >( 1, columnName ),
                          referencedTableName,
                          std::vector< std::string >( 1, referencedColumnName )
#ifdef CORAL240DC
                          , onDeleteCascade
#endif
                        );
}

void
coral::OracleAccess::TableDescriptionProxy::createForeignKey( const std::string& name,
                                                              const std::vector<std::string>& columnNames,
                                                              const std::string& referencedTableName,
                                                              const std::vector<std::string>& referencedColumnNames
#ifdef CORAL240DC
                                                              ,bool onDeleteCascade
#endif
                                                            )
{
  boost::mutex::scoped_lock lock( m_mutex );

  std::ostringstream os;
  os << "ALTER TABLE  " << m_schemaName << ".\"" + m_description->name()
     << "\" ADD ( CONSTRAINT \"" << name << "\" FOREIGN KEY ( ";
  for ( std::vector<std::string>::const_iterator iColumn = columnNames.begin();
        iColumn != columnNames.end(); ++iColumn ) {
    if ( iColumn != columnNames.begin() ) os << ", ";
    os << "\"" << *iColumn << "\"";
  }
  os << " ) REFERENCES " << m_schemaName << ".\"" << referencedTableName << "\" ( ";
  for ( std::vector<std::string>::const_iterator iColumn = referencedColumnNames.begin();
        iColumn != referencedColumnNames.end(); ++iColumn ) {
    if ( iColumn != referencedColumnNames.begin() ) os << ", ";
    os << "\"" << *iColumn << "\"";
  }
  os << " )";
#ifdef CORAL240DC
  if( onDeleteCascade )
  {
    os << " ON DELETE CASCADE";
  }
#endif
  os << " )";

  sb4 errorCode = 0;
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList(), &errorCode ) ) {
    if ( errorCode == 2275 )
    {
      // ORA-02275 such a referential constraint already exists in the table
      throw coral::InvalidForeignKeyIdentifierException( m_sessionProperties->domainServiceName(),
                                                         "TableDesciptionProxy::createForeignKey" );
    } else if ( errorCode == 904 )
    {
      // ORA-00904: invalid identifier
      // we can't decide if it is the source column, or destination column.
      // Previously in the destinatcion column case a SchemaException was
      // thrown
      throw coral::InvalidColumnNameException( m_sessionProperties->domainServiceName(),
                                               "TableDesciptionProxy::createForeignKey" );
    }
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not create a foreign key constraint on the table",
                                  "ITableSchemaEditor::createForeignKey" );
  }
  if ( m_foreignKeysRead )
  {
    m_description->createForeignKey( name,
                                     columnNames,
                                     referencedTableName,
                                     referencedColumnNames
#ifdef CORAL240DC
                                     , onDeleteCascade
#endif
                                   );
  }

}

void
coral::OracleAccess::TableDescriptionProxy::dropForeignKey( const std::string& name )
{
  boost::mutex::scoped_lock lock( m_mutex );

  std::ostringstream os;
  os << "ALTER TABLE " << m_schemaName << ".\"" + m_description->name() << "\" DROP CONSTRAINT \"" << name << "\"";

  sb4 errorCode = 0;
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList(), &errorCode ) ) 
  {
    if ( errorCode == 2443 )
    {
      // ORA-02443  Cannot drop constraint  - nonexistent constraint 
      throw coral::InvalidForeignKeyIdentifierException( m_sessionProperties->domainServiceName(),
                                                         "TableDesciptionProxy::createForeignKey" );
    }

    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not drop a foreign key constraint from the table",
                                  "ITableSchemaEditor::dropForeignKey" );
  }
  // drop foreign key from description if dropping was successful
  if ( m_foreignKeysRead )
  {
    m_description->dropForeignKey( name );
  }

}
