#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include "oci.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/TableDescription.h"

#include "DomainProperties.h"
#include "OracleErrorHandler.h"
#include "OracleStatement.h"
#include "PrivilegeManager.h"
#include "SessionProperties.h"
#include "View.h"


coral::OracleAccess::View::View( boost::shared_ptr<const SessionProperties> properties,
                                 const std::string& schemaName,
                                 const std::string& viewName,
                                 OCIDescribe* descriptor ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
  m_ociDescribeHandle( descriptor ),
  m_definitionString( "" ),
  m_description( new coral::TableDescription( m_sessionProperties->domainServiceName() ) ),
  m_privilegeManager( new coral::OracleAccess::PrivilegeManager( m_sessionProperties,
                                                                 m_schemaName,
                                                                 m_schemaName + ".\"" + viewName + "\"" ) )
{
  m_description->setName( viewName );
}


coral::ITablePrivilegeManager&
coral::OracleAccess::View::privilegeManager()
{
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "IView::privilegeManager" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "IView::privilegeManager" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "IView::privilegeManager" );
  return *m_privilegeManager;
}

const coral::ITableDescription&
coral::OracleAccess::View::description() const
{
  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::OracleAccess::View*>( this )->refreshDescription();
  return *m_description;
}


coral::OracleAccess::View::~View()
{
  delete m_privilegeManager;
  delete m_description;
  OCIHandleFree( m_ociDescribeHandle, OCI_HTYPE_DESCRIBE );
}


std::string
coral::OracleAccess::View::name() const
{
  return m_description->name();
}


std::string
coral::OracleAccess::View::definition() const
{
  if ( m_definitionString.empty() )
    const_cast<coral::OracleAccess::View*>( this )->refreshDefinition();
  return m_definitionString;
}


int
coral::OracleAccess::View::numberOfColumns() const
{
  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::OracleAccess::View*>( this )->refreshDescription();
  return m_description->numberOfColumns();
}


const coral::IColumn&
coral::OracleAccess::View::column( int index ) const
{
  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::OracleAccess::View*>( this )->refreshDescription();
  return m_description->columnDescription( index );
}


void
coral::OracleAccess::View::refreshDescription()
{
  // Retrieving the describe parameter handle
  OCIParam* ociParamHandle = 0;
  sword status = OCIAttrGet( m_ociDescribeHandle, OCI_HTYPE_DESCRIBE,
                             &ociParamHandle, 0, OCI_ATTR_PARAM,
                             m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the describe parameter for a view" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the OCI describe parameter for a view",
                                  "View::refreshDescription" );
  }

  // Retrieving the column list
  OCIParam* columnList = 0;
  status = OCIAttrGet( ociParamHandle, OCI_DTYPE_PARAM,
                       &columnList, 0, OCI_ATTR_LIST_COLUMNS,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the column list for a view" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the column list for a view",
                                  "View::refreshDescription" );
  }


  // Retrieving the number of columns
  ub2 numberOfColumns = 0;
  status = OCIAttrGet( ociParamHandle, OCI_DTYPE_PARAM,
                       &numberOfColumns, 0, OCI_ATTR_NUM_COLS,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the number of columns in view " + m_schemaName + "." + m_description->name() );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the number of columns in a view",
                                  "View::refreshDescription" );
  }



  for ( ub4 iColumn = 1; iColumn <= numberOfColumns; ++iColumn )
  {
    // Retrieving the column parameter
    void* temporaryPointer = 0;
    status = OCIParamGet( columnList, OCI_DTYPE_PARAM, m_sessionProperties->ociErrorHandle(),
                          &temporaryPointer, iColumn );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the parameter of a column in view " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the parameter of a column in a view",
                                    "View::refreshDescription" );
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
      errorHandler.handleCase( status, "Retrieving the name of a column in view " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the name of a column in a view",
                                    "View::refreshDescription" );
    }
    std::ostringstream osColumnName;
    osColumnName << textPlaceHolder;
    std::string columnName = osColumnName.str();
    columnName = columnName.substr( 0, textSize );

    // Retrieving the column type
    ub2 columnType = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &columnType, 0,
                         OCI_ATTR_DATA_TYPE,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the type of a column in view " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the type of a column in a view",
                                    "View::refreshDescription" );
    }

    // Retrieving the precision of the column
    ub1 precision = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &precision, 0,
                         OCI_ATTR_PRECISION,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the precision of a column in view " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the precision of a column in a view",
                                    "View::refreshDescription" );
    }

    // Retrieving the scale of the column
    sb1 scale = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &scale, 0,
                         OCI_ATTR_SCALE,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the scale of a column in view " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the scale of a column in a view",
                                    "View::refreshDescription" );
    }

    // Retrieving the data size of the column
    ub4 columnSize = 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &columnSize, 0,
                         OCI_ATTR_DATA_SIZE,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the size of a column in view " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the size of a column in a view",
                                    "View::refreshDescription" );
    }

    // Retrieving the nullness of the column
    ub1 isNull= 0;
    status = OCIAttrGet( columnParameter, OCI_DTYPE_PARAM,
                         &isNull, 0,
                         OCI_ATTR_IS_NULL,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the null-ness of a column in view " + m_schemaName + "." + m_description->name() );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve the null-ness of a column in a view",
                                    "View::refreshDescription" );
    }

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
      log << coral::Error << "Cannot handle the type of column " << columnName << " in view "
          << m_schemaName << "." << m_description->name() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not handle the type of a column in a view",
                                    "View::refreshDescription" );
    }

    bool fixedSize = true;
    int size = 0;
    if ( columnTypeName == coral::AttributeSpecification::typeNameForId( typeid(std::string) ) ) {
      size = columnSize;
      if ( columnType == OCI_TYPECODE_VARCHAR2 ||
           columnType == OCI_TYPECODE_VARCHAR ||
           columnType == OCI_TYPECODE_CLOB ) fixedSize = false;
    }
    m_description->insertColumn( columnName, columnTypeName, size, fixedSize );

    if ( isNull == 0 ) {
      m_description->setNotNullConstraint( columnName );
    }
  }
}


void
coral::OracleAccess::View::refreshDefinition()
{
  std::ostringstream os;
  os << "SELECT TEXT FROM ALL_VIEWS WHERE VIEW_NAME='"
     << this->description().name()
     << "' AND OWNER='" << m_schemaName << "'";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );
  if ( ! statement.execute( coral::AttributeList() ) )
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not read the definition of a view",
                                  "View::refreshDefinition" );

  coral::AttributeList* output = new coral::AttributeList;
  output->extend( "TEXT", typeid(std::string) );
  output->begin()->data<std::string>() = "";
  statement.defineOutput( *output );
  statement.fetchNext();
  m_definitionString = output->begin()->data<std::string>();
  delete output;
  if ( m_definitionString.empty() ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not read the definition of a view",
                                  "View::refreshDefinition" );
  }

  // Remove the parentheses
  while ( m_definitionString[0] == ' ' ||
          m_definitionString[0] == '\t' ||
          m_definitionString[0] == '\n' ||
          m_definitionString[0] == '\r' ) {
    m_definitionString = m_definitionString.substr( 1 );
  }
  std::string::size_type iPos = m_definitionString.size() - 1;
  while ( m_definitionString[iPos] == ' ' ||
          m_definitionString[iPos] == '\t' ||
          m_definitionString[iPos] == '\n' ||
          m_definitionString[iPos] == '\r' ) {
    m_definitionString = m_definitionString.substr( 0, iPos );
    --iPos;
  }

  if ( m_definitionString[0] == '(' && m_definitionString[iPos] == ')' ) {
    m_definitionString = m_definitionString.substr( 1, iPos - 1 );

    while ( m_definitionString[0] == ' ' ||
            m_definitionString[0] == '\t' ||
            m_definitionString[0] == '\n' ||
            m_definitionString[0] == '\r' ) {
      m_definitionString = m_definitionString.substr( 1 );
    }
    iPos = m_definitionString.size() - 1;
    while ( m_definitionString[iPos] == ' ' ||
            m_definitionString[iPos] == '\t' ||
            m_definitionString[iPos] == '\n' ||
            m_definitionString[iPos] == '\r' ) {
      m_definitionString = m_definitionString.substr( 0, iPos );
      --iPos;
    }
  }

}
