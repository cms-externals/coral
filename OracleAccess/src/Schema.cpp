#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include <cstring> // fix bug #58581
#include <memory>
#include <sstream>
#include "oci.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "CoralKernel/Service.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/SchemaException.h"
#ifdef CORAL240CN
#include "RelationalAccess/../src/IChangeNotification.h"
#endif
#ifdef CORAL240SQ
#include "RelationalAccess/../src/ISequenceDescription.h" // temporary
#endif

#include "Cursor.h"
#include "DomainProperties.h"
#include "OracleErrorHandler.h"
#include "OracleStatement.h"
#include "OracleTableBuilder.h"
#include "Query.h"
#include "Schema.h"
#ifdef CORAL240SQ
#include "Sequence.h"
#endif
#include "SessionProperties.h"
#include "Table.h"
#include "View.h"
#include "ViewFactory.h"

// Methods in unamed namespace are only valid in the same .cpp file
// Avoiding overlapping with global methods and static functions
namespace
{

#ifdef CORAL240CN
  ub4
  notification_callback( dvoid* pctx,
                         OCISubscription* /*psubhp*/,
                         dvoid* /*payload*/,
                         ub4 /*payloadlen*/,
                         dvoid* descriptor,
                         ub4 /*iMode*/ )
  {
    /* variable declaration */
    OCIEnv* envhp;
    OCIError* errhp;
    ub4 notify_type;
    sword status;
    /* allocate an OCI environment */
    OCIEnvCreate( (OCIEnv**)&envhp, OCI_OBJECT, 0, 0, 0, 0, 0, 0 );
    /* allocate an error handle */
    OCIHandleAlloc( (dvoid *)envhp, (dvoid **)&errhp, OCI_HTYPE_ERROR, 0, 0);
    /* get the event type */
    status = OCIAttrGet( descriptor, OCI_DTYPE_CHDES, &notify_type, NULL, OCI_ATTR_CHDES_NFYTYPE, errhp);

    coral::OracleAccess::OracleErrorHandler errorhdl( errhp );

    errorhdl.handleCase( status, "NotificationCallback: get the event type", true );

    if (notify_type != OCI_EVENT_OBJCHANGE)
    {
      OCIHandleFree((dvoid *)envhp, OCI_HTYPE_ENV);
      OCIHandleFree((dvoid *)errhp, OCI_HTYPE_ERROR);
      return 1;
    }

    /* Obtain the collection of table change descriptors */
    OCIColl* table_changes = 0;
    status = OCIAttrGet( descriptor, OCI_DTYPE_CHDES, &table_changes, NULL, OCI_ATTR_CHDES_TABLE_CHANGES, errhp);

    errorhdl.handleCase( status, "NotificationCallback: obtain the collection of table change descriptors", true );
    /* get the size of the collection */
    sb4 num_tables = 0;
    if (table_changes)
    {
      status = OCICollSize(envhp, errhp, (CONST OCIColl *)table_changes, &num_tables);
      errorhdl.handleCase( status, "NotificationCallback: get the size of the collection", true );
    }
    else
      num_tables = 0;

    boolean exist;
    dvoid* elemind = 0;
    /* For each element of the collection, extract the table name of the modified table */
    for( ub2 i = 0; i < num_tables; i++)
    {
      /* get the table description */
      dvoid** table_descp;
      status = OCICollGetElem(envhp, errhp, (OCIColl *)table_changes, i, &exist, table_descp, &elemind);

      errorhdl.handleCase( status, "NotificationCallback: get the table description", true );

      if( exist )
      {
        // get the table name
        oratext** table_name;
        status = OCIAttrGet(*table_descp, OCI_DTYPE_TABLE_CHDES, table_name, NULL, OCI_ATTR_CHDES_TABLE_NAME, errhp);
        errorhdl.handleCase( status, "NotificationCallback: get the table name", true );

        ((coral::IChangeNotification*)pctx)->event();

        printf(" not %s\n", *table_name);
      }


    }

    if (errhp)
      OCIHandleFree((dvoid *)errhp, OCI_HTYPE_ERROR);
    if (envhp)
      OCIHandleFree((dvoid *)envhp, OCI_HTYPE_ENV);

    return 0;
  }

#endif

}

coral::OracleAccess::Schema::Schema( boost::shared_ptr<const SessionProperties> sessionProperties,
                                     const std::string& schemaName )
  : m_sessionProperties( sessionProperties )
  , m_schemaName( schemaName )
  , m_tables()
  , m_tablesReadFromDataDictionary( false )
  , m_views()
  , m_viewsReadFromDataDictionary( false )
  , m_tableMutex()
  , m_viewMutex()
  , m_userSessionStarted( false ) // startUserSession must be called explicitly
{
}

//-----------------------------------------------------------------------------

coral::OracleAccess::Schema::~Schema()
{
  if ( !m_userSessionStarted ) return; // Schema was already 'deleted'
  this->reactOnEndOfTransaction();
}

//-----------------------------------------------------------------------------

std::string coral::OracleAccess::Schema::schemaName() const
{
  return m_schemaName;
}

//-----------------------------------------------------------------------------

std::set<std::string>
coral::OracleAccess::Schema::listTables() const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::listTables",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ISchema::listTables" );
  std::set<std::string> result;
  if ( ! m_tablesReadFromDataDictionary )
    this->readTablesFromDataDictionary();

  boost::mutex::scoped_lock lock( m_tableMutex );
  for ( std::map< std::string, coral::OracleAccess::Table*>::const_iterator iTable = m_tables.begin();
        iTable != m_tables.end(); ++iTable ) {
    result.insert( iTable->first );
  }

  return result;
}

//-----------------------------------------------------------------------------

bool
coral::OracleAccess::Schema::existsTable( const std::string& tableName ) const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::existsTable",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ISchema::existsTable" );

  // Check if the table is already in the local cache.
  bool isInCache = false;
  {
    boost::mutex::scoped_lock lock( m_tableMutex );
    isInCache = ( m_tables.find( tableName ) == m_tables.end() ? false : true );
  }

  if ( ! isInCache ) {

    // Check inside the database.

    // The fully qualified table name
    std::string fullTableName = m_schemaName + ".\"" + tableName + "\"";

    // Try to get the descriptor handle
    void* temporaryPointer = 0;
    sword status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(),
                                   &temporaryPointer,
                                   OCI_HTYPE_DESCRIBE, 0, 0 );
    if ( status != OCI_SUCCESS ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not allocate a describe handle",
                                    "ISchema::existsTable" );
    }

    OCIDescribe* ociDescribeHandle = static_cast< OCIDescribe* >( temporaryPointer );

    status = OCIDescribeAny( m_sessionProperties->ociSvcCtxHandle(),
                             m_sessionProperties->ociErrorHandle(),
                             const_cast<char *>( fullTableName.c_str() ),
                             ::strlen( fullTableName.c_str() ),
                             OCI_OTYPE_NAME, OCI_DEFAULT,
                             OCI_PTYPE_TABLE,
                             ociDescribeHandle );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "retrieving the describe handle of " + fullTableName + " (Schema::existsTable)" );
      OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
      if ( errorHandler.lastErrorCode() != 4043 &&
           errorHandler.lastErrorCode() != 1403 ) {
        // Errors other than 4043 or 1403: unexpected problem -> throw
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                      "Could not retrieve a describe handle",
                                      "ISchema::existsTable" );
      }
      else
      {
        // WARN that 1403 is handled as equivalent to 4043 (bug #49657)
        if ( errorHandler.lastErrorCode() == 1403 ) {
          coral::MessageStream log( m_sessionProperties->domainServiceName() );
          log << coral::Verbose << errorHandler.message() << coral::MessageStream::endmsg;
        }
        // Error 4043 or 1403: no such table -> check if it is a synonym
        std::string tableForSynonym = this->synonymForTable( tableName );
        if ( tableForSynonym.empty() || (tableForSynonym == tableName) )
          return false;
        else
          return this->existsTable( tableForSynonym );
      }
    }
    else {
      boost::mutex::scoped_lock lock( m_tableMutex );

      if ( m_tables.find( tableName ) != m_tables.end() ) { // Another thread was faster...
        OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
        return true;
      }

      // The table exists. Create the table object and insert it into the map
      coral::OracleAccess::Table* table = new coral::OracleAccess::Table( m_sessionProperties,
                                                                          m_schemaName,
                                                                          tableName,
                                                                          ociDescribeHandle );
      m_tables.insert( std::make_pair( tableName, table ) );
      return true;
    }
  }
  else
    return true;
}

//-----------------------------------------------------------------------------

std::string
coral::OracleAccess::Schema::synonymForTable( const std::string& tableName ) const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::synonymForTable",
                            m_sessionProperties->domainServiceName() );
  // The fully qualified table name
  std::string fullTableName = m_schemaName + ".\"" + tableName + "\"";
  // Try to get the descriptor handle
  void* temporaryPointer = 0;
  sword status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(),
                                 &temporaryPointer,
                                 OCI_HTYPE_DESCRIBE, 0, 0 );
  if ( status != OCI_SUCCESS ) {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not allocate a describe handle",
                                  "OracleAccess::Schema::synonymForTable" );
  }

  OCIDescribe* ociDescribeHandle = static_cast< OCIDescribe* >( temporaryPointer );

  status = OCIDescribeAny( m_sessionProperties->ociSvcCtxHandle(),
                           m_sessionProperties->ociErrorHandle(),
                           const_cast<char *>( fullTableName.c_str() ),
                           ::strlen( fullTableName.c_str() ),
                           OCI_OTYPE_NAME, OCI_DEFAULT,
                           OCI_PTYPE_SYN,
                           ociDescribeHandle );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "retrieving the describe handle of " + fullTableName + " (Schema::synonymForTable)" );
    OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
    if ( errorHandler.lastErrorCode() != 4043 &&
         errorHandler.lastErrorCode() != 1403 ) {
      // Errors other than 4043 or 1403: unexpected problem -> throw
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not retrieve a describe handle",
                                    "OracleAccess::Schema::synonymForTable" );
    }
    else {
      // WARN that 1403 is handled as equivalent to 4043 (bug #49657)
      if ( errorHandler.lastErrorCode() == 1403 ) {
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Verbose << errorHandler.message() << coral::MessageStream::endmsg;
      }
      // Error 4043 or 1403: not a synonym -> return ""
      return "";
    }
  }

  // Retrieve the corresponding table name.
  OCIParam* ociParamHandle = 0;
  status = OCIAttrGet( ociDescribeHandle, OCI_HTYPE_DESCRIBE,
                       &ociParamHandle, 0, OCI_ATTR_PARAM,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the describe parameter for synonym " + fullTableName );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the OCI describe parameter for synonym " + fullTableName,
                                  "OracleAccess::Schema::synonymForTable" );
  }

  // Retrieving the table name
  text* textPlaceHolder = 0;
  ub4 textSize = 0;
  status = OCIAttrGet( ociParamHandle, OCI_DTYPE_PARAM,
                       &textPlaceHolder, &textSize,
                       OCI_ATTR_NAME,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the name of the object referenced by a synonym" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not retrieve the name of the object referenced by a synonym",
                                  "OracleAccess::Schema::synonymForTable" );
  }

  std::ostringstream osName;
  osName << textPlaceHolder;
  std::string sName = osName.str();

  OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
  return sName.substr( 0, textSize );
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::dropTable( const std::string& tableName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::dropTable",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::dropTable" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::dropTable" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::dropTable" );

  if ( ! this->existsTable( tableName ) ) {
    throw coral::TableNotExistingException( m_sessionProperties->domainServiceName(),
                                            tableName,
                                            "ISchema::dropTable" );
  }
  std::string dropTableCmd = "DROP TABLE " + m_schemaName + ".\"" + tableName + "\"";
  // Temporary workaround for POOL bug #61888
  // Eventually extend API to support dropTableCascade (task #14095)
  if ( ::getenv( "CORAL_ORA_DROP_TABLE_CASCADE_CONSTRAINTS" ) ) dropTableCmd += " CASCADE CONSTRAINTS";
  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, dropTableCmd );
  if ( ! statement.execute( coral::AttributeList() ) )
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not drop a table",
                                  "ISchema::dropTable" );

  boost::mutex::scoped_lock lock( m_tableMutex );
  std::map< std::string, coral::OracleAccess::Table* >::iterator iTable = m_tables.find( tableName );
  if ( iTable->second ) delete iTable->second;
  m_tables.erase( iTable );
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::dropIfExistsTable( const std::string& tableName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::dropIfExistsTable",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::dropIfExistsTable" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::dropIfExistsTable" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::dropIfExistsTable" );
  if ( this->existsTable( tableName ) )
    this->dropTable( tableName );
}

//-----------------------------------------------------------------------------

coral::ITable&
coral::OracleAccess::Schema::createTable( const coral::ITableDescription& description )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::createTable",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::createTable" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::createTable" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::createTable" );

  // Get the name of the table
  std::string tableName = description.name();

  // Check if a table with this name already exists
  if ( this->existsTable( tableName ) )
    throw coral::TableAlreadyExistingException( m_sessionProperties->domainServiceName(),
                                                tableName );

  // Construct and execute the statement creating the table
  coral::OracleAccess::OracleTableBuilder builder( description,
                                                   m_sessionProperties,
                                                   m_schemaName );
  coral::OracleAccess::OracleStatement statement( m_sessionProperties,
                                                  m_schemaName,
                                                  builder.statement() );
  if ( ! statement.execute( coral::AttributeList() ) )
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not create a new table",
                                  "ISchema::createTable" );

  // Get the table handle
  coral::ITable& table = this->tableHandle( tableName );

  // Use now the schema editor to add the indices
  coral::ITableSchemaEditor& editor = table.schemaEditor();
  for ( int i = 0; i < description.numberOfIndices(); ++i ) {
    const coral::IIndex& index = description.index( i );
    editor.createIndex( index.name(), index.columnNames(), index.isUnique(), index.tableSpaceName() );
  }

  return table;
}

//-----------------------------------------------------------------------------

coral::ITable&
coral::OracleAccess::Schema::tableHandle( const std::string& tableName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::tableHandle",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ISchema::tableHandle" );

  // Check if the table is already in the local cache.
  std::map< std::string, coral::OracleAccess::Table* >::iterator iTable;
  bool needDescription = false;
  {
    boost::mutex::scoped_lock lock( m_tableMutex );
    iTable = m_tables.find( tableName );
    needDescription = ( iTable == m_tables.end() || iTable->second == 0 );
  }

  if ( needDescription ) {

    // The fully qualified table name
    const std::string fullTableName = m_schemaName + ".\"" + tableName + "\"";

    // Try to get the descriptor handle
    void* temporaryPointer = 0;
    sword status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(),
                                   &temporaryPointer,
                                   OCI_HTYPE_DESCRIBE, 0, 0 );
    if ( status != OCI_SUCCESS ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not allocate a describe handle",
                                    "ISchema::tableHandle" );
    }

    OCIDescribe* ociDescribeHandle = static_cast< OCIDescribe* >( temporaryPointer );

    {
      //boost::mutex::scoped_lock lock( m_sessionProperties->connectionProperties().connectionMutex() );
      status = OCIDescribeAny( m_sessionProperties->ociSvcCtxHandle(),
                               m_sessionProperties->ociErrorHandle(),
                               const_cast<char *>( fullTableName.c_str() ),
                               ::strlen( fullTableName.c_str() ),
                               OCI_OTYPE_NAME, OCI_DEFAULT,
                               OCI_PTYPE_TABLE,
                               ociDescribeHandle );
    }
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "retrieving the describe handle of " + fullTableName + " (Schema::tableHandle)" );
      OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
      if ( errorHandler.lastErrorCode() != 4043 &&
           errorHandler.lastErrorCode() != 1403 ) {
        // Errors other than 4043 or 1403: unexpected problem -> throw
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                      "Could not retrieve a describe handle",
                                      "ISchema::tableHandle" );
      }
      else {
        // WARN that 1403 is handled as equivalent to 4043 (bug #49657)
        if ( errorHandler.lastErrorCode() == 1403 ) {
          coral::MessageStream log( m_sessionProperties->domainServiceName() );
          log << coral::Verbose << errorHandler.message() << coral::MessageStream::endmsg;
        }
        // Error 4043 or 1403: no such table -> check if it is a synonym
        std::string tableForSynonym = this->synonymForTable( tableName );
        if ( tableForSynonym.empty() )
          throw coral::TableNotExistingException( m_sessionProperties->domainServiceName(),
                                                  fullTableName );
        else
          return this->tableHandle( tableForSynonym );
      }
    }

    boost::mutex::scoped_lock lock( m_tableMutex );
    iTable = m_tables.find( tableName );
    if ( iTable != m_tables.end() && iTable->second != 0 ) { // Another thread was faster...
      OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
    }
    else {
      // The table exists. Create the table object and insert it into the map
      coral::OracleAccess::Table* table = new coral::OracleAccess::Table( m_sessionProperties,
                                                                          m_schemaName,
                                                                          tableName,
                                                                          ociDescribeHandle );
      if ( iTable == m_tables.end() )
        iTable = m_tables.insert( std::make_pair( tableName, table ) ).first;
      else
        iTable->second = table;
    }
    return *( iTable->second );
  }

  return *( iTable->second );
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::truncateTable( const std::string& tableName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::truncateTable",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::truncateTable" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::truncateTable" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::truncateTable" );

  if ( ! this->existsTable( tableName ) )
    throw coral::TableNotExistingException( m_sessionProperties->domainServiceName(),
                                            tableName, "truncateTable" );

  coral::OracleAccess::OracleStatement statement( m_sessionProperties,
                                                  m_schemaName,
                                                  "TRUNCATE TABLE " + m_schemaName + ".\"" + tableName + "\"" );

  if ( ! statement.execute( coral::AttributeList() ) )
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not truncate table \"" + tableName + "\"",
                                  "ISchema::truncateTable" );
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::callProcedure( const std::string& procedureName,
                                            const coral::AttributeList& inputArguments )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::callProcedure",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::callProcedure" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::callProcedure" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::callProcedure" );
  std::ostringstream os;
  std::string sqlProcName = procedureName;
  size_t dotPos = sqlProcName.find(".");

  if (dotPos != std::string::npos) sqlProcName.replace(dotPos, 1, "\".\"");

  os << "CALL " << m_schemaName << ".\"" << sqlProcName << "\" ( ";

  for ( coral::AttributeList::const_iterator iAttribute = inputArguments.begin();
        iAttribute != inputArguments.end(); ++iAttribute ) {
    if ( iAttribute != inputArguments.begin() ) os << ", ";
    os << ":\"" << iAttribute->specification().name() << "\"";
  }

  os << " )";

  coral::OracleAccess::OracleStatement statement( m_sessionProperties, m_schemaName, os.str() );

  if ( ! statement.execute( inputArguments ) )
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Error calling \"" + sqlProcName + "\"",
                                  "ISchema::callProcedure" );
}

//-----------------------------------------------------------------------------

coral::IQuery*
coral::OracleAccess::Schema::newQuery() const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::newQuery",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ISchema::newQuery" );

  return new coral::OracleAccess::Query( m_sessionProperties, m_schemaName );
}

//-----------------------------------------------------------------------------

coral::IViewFactory*
coral::OracleAccess::Schema::viewFactory()
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::viewFactory",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::viewFactory" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::viewFactory" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::viewFactory" );

  return new coral::OracleAccess::ViewFactory( m_sessionProperties, m_schemaName );
}

//-----------------------------------------------------------------------------

bool
coral::OracleAccess::Schema::existsView( const std::string& viewName ) const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::existsView",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ISchema::existsView" );

  // Check if the view is already in the local cache.
  bool isInCache = false;
  {
    boost::mutex::scoped_lock lock( m_tableMutex );
    isInCache = ( m_views.find( viewName ) != m_views.end() );
  }

  if ( ! isInCache ) {

    // Check inside the database.

    // The fully qualified view name
    std::string fullViewName = m_schemaName + ".\"" + viewName + "\"";

    // Try to get the descriptor handle
    void* temporaryPointer = 0;
    sword status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(),
                                   &temporaryPointer,
                                   OCI_HTYPE_DESCRIBE, 0, 0 );
    if ( status != OCI_SUCCESS ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not allocate a describe handle",
                                    "ISchema::existsView" );
    }

    OCIDescribe* ociDescribeHandle = static_cast< OCIDescribe* >( temporaryPointer );

    status = OCIDescribeAny( m_sessionProperties->ociSvcCtxHandle(),
                             m_sessionProperties->ociErrorHandle(),
                             const_cast<char *>( fullViewName.c_str() ),
                             ::strlen( fullViewName.c_str() ),
                             OCI_OTYPE_NAME, OCI_DEFAULT,
                             OCI_PTYPE_VIEW,
                             ociDescribeHandle );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "retrieving the describe handle of " + fullViewName + " (Schema::existsView)" );
      OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
      if ( errorHandler.lastErrorCode() != 4043 &&
           errorHandler.lastErrorCode() != 1403 ) {
        // Errors other than 4043 or 1403: unexpected problem -> throw
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                      "Could not retrieve a describe handle",
                                      "ISchema::existsView" );
      }
      else
      {
        // WARN that 1403 is handled as equivalent to 4043 (bug #49657)
        if ( errorHandler.lastErrorCode() == 1403 )
        {
          coral::MessageStream log( m_sessionProperties->domainServiceName() );
          log << coral::Verbose << errorHandler.message() << coral::MessageStream::endmsg;
        }
        // Error 4043 or 1403: no such table -> check if it is a synonym
        std::string viewForSynonym = this->synonymForTable( viewName );
        if ( viewForSynonym.empty() || (viewForSynonym == viewName) )
          return false;
        else
          return this->existsView( viewForSynonym );
      }
    }
    else {
      boost::mutex::scoped_lock lock( m_viewMutex );
      if ( m_views.find( viewName ) != m_views.end() ) { // Another thread was faster...
        OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
        return true;
      }

      // The view exists. Create the view object and insert it into the map
      coral::OracleAccess::View* view = new coral::OracleAccess::View( m_sessionProperties,
                                                                       m_schemaName,
                                                                       viewName,
                                                                       ociDescribeHandle );
      m_views.insert( std::make_pair( viewName, view ) );
      return true;
    }
  }
  else
    return true;
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::dropView( const std::string& viewName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::dropView",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::dropView" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::dropView" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::dropView" );

  if ( ! this->existsView( viewName ) ) {
    throw coral::ViewNotExistingException( m_sessionProperties->domainServiceName(),
                                           viewName,
                                           "ISchema::dropView" );
  }
  coral::OracleAccess::OracleStatement statement( m_sessionProperties,
                                                  m_schemaName,
                                                  "DROP VIEW " + m_schemaName + ".\"" + viewName + "\"" );

  if ( ! statement.execute( coral::AttributeList() ) )
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Could not drop a view",
                                  "ISchema::dropView" );

  boost::mutex::scoped_lock lock( m_viewMutex );
  std::map< std::string, coral::OracleAccess::View* >::iterator iView = m_views.find( viewName );
  if ( iView->second ) delete iView->second;
  m_views.erase( iView );
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::dropIfExistsView( const std::string& viewName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::dropIfExistsView",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(), "ISchema::dropIfExistsView" );
  if ( m_sessionProperties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties->domainServiceName(), "ISchema::dropIfExistsView" );
  if ( m_sessionProperties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_sessionProperties->domainServiceName(), "ISchema::dropIfExistsView" );

  if ( this->existsView( viewName ) )
    this->dropView( viewName );
}

//-----------------------------------------------------------------------------

coral::IView&
coral::OracleAccess::Schema::viewHandle( const std::string& viewName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::viewHandle",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ISchema::viewHandle" );

  // Check if the view is already in the local cache.
  std::map< std::string, coral::OracleAccess::View* >::iterator iView;
  bool needDescription = false;
  {
    boost::mutex::scoped_lock lock( m_viewMutex );
    iView = m_views.find( viewName );
    needDescription = ( iView == m_views.end() || iView->second == 0 );
  }

  if ( needDescription ) {

    // The fully qualified view name
    const std::string fullViewName = m_schemaName + ".\"" + viewName + "\"";

    // Try to get the descriptor handle
    void* temporaryPointer = 0;
    sword status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(),
                                   &temporaryPointer,
                                   OCI_HTYPE_DESCRIBE, 0, 0 );
    if ( status != OCI_SUCCESS ) {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Could not allocate a describe handle",
                                    "ISchema::viewHandle" );
    }

    OCIDescribe* ociDescribeHandle = static_cast< OCIDescribe* >( temporaryPointer );

    status = OCIDescribeAny( m_sessionProperties->ociSvcCtxHandle(),
                             m_sessionProperties->ociErrorHandle(),
                             const_cast<char *>( fullViewName.c_str() ),
                             ::strlen( fullViewName.c_str() ),
                             OCI_OTYPE_NAME, OCI_DEFAULT,
                             OCI_PTYPE_VIEW,
                             ociDescribeHandle );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "retrieving the describe handle of " + fullViewName );
      OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
      if ( errorHandler.lastErrorCode() != 4043 &&
           errorHandler.lastErrorCode() != 1403 ) {
        // Errors other than 4043 or 1403: unexpected problem -> throw
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                      "Could not retrieve a describe handle",
                                      "ISchema::viewHandle" );
      }
      else {
        // WARN that 1403 is handled as equivalent to 4043 (bug #49657)
        if ( errorHandler.lastErrorCode() == 1403 ) {
          coral::MessageStream log( m_sessionProperties->domainServiceName() );
          log << coral::Verbose << errorHandler.message() << coral::MessageStream::endmsg;
        }
        // Error 4043 or 1403: no such view -> throw ViewNotExistingException
        throw coral::ViewNotExistingException( m_sessionProperties->domainServiceName(),
                                               fullViewName );
      }
    }

    boost::mutex::scoped_lock lock( m_viewMutex );
    iView = m_views.find( viewName );
    if ( iView != m_views.end() && iView->second != 0 ) { // Another thread was faster...
      OCIHandleFree( ociDescribeHandle, OCI_HTYPE_DESCRIBE );
    }
    else {
      // The view exists. Create the view object and insert it into the map
      coral::OracleAccess::View* view = new coral::OracleAccess::View( m_sessionProperties,
                                                                       m_schemaName,
                                                                       viewName,
                                                                       ociDescribeHandle );
      if ( iView == m_views.end() )
        iView = m_views.insert( std::make_pair( viewName, view ) ).first;
      else
        iView->second = view;
    }
    return *( iView->second );
  }

  return *( iView->second );
}

//-----------------------------------------------------------------------------

std::set<std::string>
coral::OracleAccess::Schema::listViews() const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::listViews",
                            m_sessionProperties->domainServiceName() );
  if ( ! m_sessionProperties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_sessionProperties->domainServiceName(),
                                                "ISchema::listViews" );
  std::set<std::string> result;
  if ( ! m_viewsReadFromDataDictionary )
    this->readViewsFromDataDictionary();

  boost::mutex::scoped_lock lock( m_viewMutex );
  for ( std::map< std::string, coral::OracleAccess::View*>::const_iterator iView = m_views.begin();
        iView != m_views.end(); ++iView ) {
    result.insert( iView->first );
  }

  return result;
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::reactOnEndOfTransaction()
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::reactOnEndOfTransaction",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock( m_tableMutex );
  for ( std::map< std::string, coral::OracleAccess::Table* >::iterator iTable = m_tables.begin();
        iTable != m_tables.end(); ++iTable )
    if ( iTable->second )
      delete iTable->second;

  m_tables.clear();

  m_tablesReadFromDataDictionary = false;

  boost::mutex::scoped_lock lockViews( m_viewMutex );
  for ( std::map< std::string, coral::OracleAccess::View* >::iterator iView = m_views.begin();
        iView != m_views.end(); ++iView )
    if ( iView->second )
      delete iView->second;

  m_views.clear();

  m_viewsReadFromDataDictionary = false;

#ifdef CORAL240SQ
  std::map< std::string, coral::OracleAccess::Sequence* >::iterator i;
  for(i = m_sequences.begin(); i != m_sequences.end(); ++i )
  {
    delete i->second;
  }
  m_sequences.clear();
#endif

}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::readTablesFromDataDictionary() const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::readTablesFromDataDictionary",
                            m_sessionProperties->domainServiceName() );

  coral::AttributeList bindData;
  bindData.extend<std::string>( "OWNER" );
  bindData.begin()->data<std::string>() = m_schemaName;

  std::auto_ptr<coral::OracleAccess::OracleStatement> pStatement;

  // Task #10775 (performance optimization for data dictionary queries)
  // Optimization (requires read access to sys tables)
  // The selectAnyTable is initially true but is set to false if query fails
  if ( m_sessionProperties->selectAnyTable() )
  {
    std::ostringstream os;
    os << "select /*+ INDEX(o I_OBJ2) */ o.name as TABLE_NAME "
       << "from sys.user$ u, sys.obj$ o "
       << "where u.name=:OWNER "
       << "and o.owner#=u.user# and o.type#=2";
    //std::cout << std::endl << "EXECUTE: " << os.str() << std::endl;
    pStatement.reset( new coral::OracleAccess::OracleStatement( m_sessionProperties, m_schemaName, os.str() ) );
    pStatement->setNumberOfPrefetchedRows( 100 );
    if ( ! pStatement->execute( bindData ) )
      m_sessionProperties->cannotSelectAnyTable();
  }

  // Task #10775 (performance optimization for data dictionary queries)
  // Query on ALL_TABLES (valid for all users)
  // The selectAnyTable has been set to false if the first attempt failed
  if ( !m_sessionProperties->selectAnyTable() )
  {
    std::ostringstream os;
    os << "SELECT /* user has no SELECT ANY TABLE privileges */ "
       << "TABLE_NAME FROM ALL_TABLES WHERE OWNER=:OWNER";
    //std::cout << std::endl << "EXECUTE: " << os.str() << std::endl;

    pStatement.reset( new coral::OracleAccess::OracleStatement( m_sessionProperties, m_schemaName, os.str() ) );
    pStatement->setNumberOfPrefetchedRows( 100 );
    if ( ! pStatement->execute( bindData ) )
    {
      pStatement.reset();
      throw coral::SchemaException
        ( m_sessionProperties->domainServiceName(),
          "Could not retrieve the list of tables in a schema",
          "Schema::readTablesFromDataDictionary" );
    }
  }

  boost::mutex::scoped_lock lock( m_tableMutex );

  coral::AttributeList output;
  output.extend<std::string>( "TABLE_NAME" );
  const std::string& tableName = output.begin()->data<std::string>();
  pStatement->defineOutput( output );

  while( pStatement->fetchNext() ) {
    if ( m_tables.find( tableName ) == m_tables.end() ) {
      m_tables[tableName] = 0;
    }
  }
  m_tablesReadFromDataDictionary = true;
  pStatement.reset();
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::readViewsFromDataDictionary() const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::readViewsFromDataDictionary",
                            m_sessionProperties->domainServiceName() );
  coral::OracleAccess::OracleStatement statement( m_sessionProperties,
                                                  m_schemaName,
                                                  "SELECT VIEW_NAME FROM ALL_VIEWS WHERE OWNER=:\"owner\"" );
  statement.setNumberOfPrefetchedRows( 100 );
  coral::AttributeList bindData;
  bindData.extend<std::string>( "owner" );
  bindData.begin()->data<std::string>() = m_schemaName;
  statement.execute( bindData );
  coral::AttributeList output;
  output.extend<std::string>( "VIEW_NAME" );
  const std::string& viewName = output.begin()->data<std::string>();
  statement.defineOutput( output );

  boost::mutex::scoped_lock lock( m_viewMutex );

  while( statement.fetchNext() ) {
    if ( m_views.find( viewName ) == m_views.end() ) {
      m_views[viewName] = 0;
    }
  }
  m_viewsReadFromDataDictionary = true;
}

//-----------------------------------------------------------------------------

#ifdef CORAL240CN
void
coral::OracleAccess::Schema::registerNotification( const std::string& tableName, coral::IChangeNotification& callback )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::registerNotification",
                            m_sessionProperties->domainServiceName() );
  OracleErrorHandler errorhdl( m_sessionProperties->ociErrorHandle() );

  coral::MessageStream log( m_sessionProperties->domainServiceName() );

  log << coral::Debug << "Try to register NotificationClass on table [" << tableName << "]" << coral::MessageStream::endmsg;

  sword status;
  ub4 port = 7700;
  /* set the port number for subscriptions */
  status = OCIAttrSet( m_sessionProperties->ociEnvHandle(),
                       OCI_HTYPE_ENV,
                       (dvoid *)&port,
                       0,
                       OCI_ATTR_SUBSCR_PORTNO,
                       m_sessionProperties->ociErrorHandle() );

  errorhdl.handleCase( status, "registerNotification: set the port number for subscriptions", true );

  static ub4 nspace = OCI_SUBSCR_NAMESPACE_DBCHANGE;
  static ub4 rowids = TRUE;
  //static ub4 recpto = OCI_SUBSCR_PROTO_OCI;
  OCISubscription* subscrhp = 0;
  /* allocate subscription handler */
  status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(), (void**)&subscrhp, OCI_HTYPE_SUBSCRIPTION, 0, 0 );
  errorhdl.handleCase( status, "registerNotification: allocate subscription handler", true );
  /* set the namespace to DBCHANGE */
  status = OCIAttrSet( subscrhp, OCI_HTYPE_SUBSCRIPTION, (void*)&nspace, sizeof(ub4), OCI_ATTR_SUBSCR_NAMESPACE, m_sessionProperties->ociErrorHandle());
  errorhdl.handleCase( status, "registerNotification: set the namespace to DBCHANGE", true );
  /* set the callback function */
  status = OCIAttrSet( subscrhp, OCI_HTYPE_SUBSCRIPTION, (void*)notification_callback, 0, OCI_ATTR_SUBSCR_CALLBACK, m_sessionProperties->ociErrorHandle());
  errorhdl.handleCase( status, "registerNotification: set the callback function", true );
  /* set context */
  status = OCIAttrSet( subscrhp, OCI_HTYPE_SUBSCRIPTION, (void*)&callback, 0, OCI_ATTR_SUBSCR_CTX, m_sessionProperties->ociErrorHandle());
  errorhdl.handleCase( status, "registerNotification: set context", true );
  /* allow extraction of rowid information */
  status = OCIAttrSet( subscrhp, OCI_HTYPE_SUBSCRIPTION, (void*)&rowids, sizeof(ub4), OCI_ATTR_CHNF_ROWIDS, m_sessionProperties->ociErrorHandle());
  errorhdl.handleCase( status, "registerNotification: allow extraction of rowid information", true );
  /* register the notification callback */
  status = OCISubscriptionRegister( m_sessionProperties->ociSvcCtxHandle(), &subscrhp, 1, m_sessionProperties->ociErrorHandle(), OCI_DEFAULT );
  errorhdl.handleCase( status, "registerNotification: register the notification callback", true );
  /* register a query to the notification */

  OCIStmt* stmthp;
  dvoid* tmp;
  OCIDefine* defnp1 = (OCIDefine *)0;
  int mgr_id = 0;
  /* allocate a statement handle */
  status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(), (dvoid**)&stmthp, OCI_HTYPE_STMT, 52, (dvoid**)&tmp );
  errorhdl.handleCase( status, "registerNotification: allocate a statement handle", true );

  /* create the correct table name with the schema name */
  std::string query1("SELECT * FROM ");
  query1.append(this->schemaName());
  query1.append(".\"");
  query1.append(tableName);
  query1.append("\"");

  const char* query_c = query1.c_str();

  log << coral::Debug << "Try to register on QUERY [" << query1 << "]" << coral::MessageStream::endmsg;

  status = OCIStmtPrepare( stmthp, m_sessionProperties->ociErrorHandle(), reinterpret_cast< CONST text* >( query_c ), strlen(query_c), OCI_V7_SYNTAX, OCI_DEFAULT );
  errorhdl.handleCase( status, "registerNotification: OCIStmtPrepare", true );

  status = OCIDefineByPos( stmthp, &defnp1, m_sessionProperties->ociErrorHandle(), 1, (dvoid*)&mgr_id, sizeof(mgr_id), SQLT_INT, (dvoid*)0, 0, 0, OCI_DEFAULT );
  errorhdl.handleCase( status, "registerNotification: OCIDefineByPos", true );

  status = OCIAttrSet( stmthp, OCI_HTYPE_STMT, subscrhp, 0, OCI_ATTR_CHNF_REGHANDLE, m_sessionProperties->ociErrorHandle() );
  errorhdl.handleCase( status, "registerNotification: OCIAttrSet", true );

  status = OCIStmtExecute( m_sessionProperties->ociSvcCtxHandle(), stmthp, m_sessionProperties->ociErrorHandle(), 1, 0, NULL, NULL, OCI_DEFAULT );
  errorhdl.handleCase( status, "registerNotification: OCIStmtExecute", true );

  log << coral::Info << "Register successful of NotificationClass on table [" << tableName << "]" << coral::MessageStream::endmsg;

}

#endif
//-----------------------------------------------------------------------------

#ifdef CORAL240SQ

//-----------------------------------------------------------------------------

bool
coral::OracleAccess::Schema::validateSequence( const std::string& sequenceName ) const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::validateSequence",
                            m_sessionProperties->domainServiceName() );
  // try to find the sequence in the map first
  std::map< std::string, coral::OracleAccess::Sequence* >::iterator i = m_sequences.find( sequenceName );
  if( i == m_sequences.end() )
  {
    // Not found
    coral::OracleAccess::Sequence* sequence = coral::OracleAccess::Sequence::checkIfExists( m_sessionProperties, m_schemaName, sequenceName );
    if( sequence )
    {
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Verbose << "Found sequence [" << sequenceName << "] in database" << coral::MessageStream::endmsg;
      // Sequence exists in the database
      // Store the object in the map
      m_sequences.insert(std::pair< std::string, coral::OracleAccess::Sequence* >( sequenceName, sequence ));
      // return true
      return true;
    }
  }
  else
  {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    // Found in the map
    if( coral::OracleAccess::Sequence::checkIfStillExists( m_sessionProperties, m_schemaName, sequenceName ) )
    {
      log << coral::Verbose << "Found sequence [" << sequenceName << "] in cache" << coral::MessageStream::endmsg;
      // Everything is fine
      return true;
    }
    log << coral::Verbose << "Sequence [" << sequenceName << "] doesn't exists anymore" << coral::MessageStream::endmsg;
    // Sequence was deleted from the database
    // Drop the entry for the Sequence
    delete i->second;
    // Erase
    m_sequences.erase(i);
  }
  return false;
}

//-----------------------------------------------------------------------------

coral::ISequence&
coral::OracleAccess::Schema::createSequence( const coral::ISequenceDescription& description )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::createSequence",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock( m_sequenceMutex );

  if( validateSequence( description.name() ) )
  {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Sequence already exists " + description.name(),
                                  "Schema::createSequence" );
  }
  // Continue with the creation process
  std::stringstream stmstream;

  stmstream << "CREATE SEQUENCE " << description.name() << " START WITH " << description.startValue();
  // Add the specific order types, if default nothing will be added to the statement
  if( description.isOrdered() == coral::Sequence::Order )
    stmstream << " ORDER";
  else if( description.isOrdered() == coral::Sequence::NoOrder )
    stmstream << " NOORDER";

  //statement << " NOCACHE";

  // Create the sequence via an SQL statement
  {
    std::auto_ptr<OracleStatement> statement( new OracleStatement( m_sessionProperties, m_schemaName, stmstream.str() ) );
    if ( !( statement->execute( coral::AttributeList() ) ) )
      throw coral::SchemaException( m_sessionProperties->domainServiceName(), "Can't create sequence in the database " + description.name(), "Schema::createSequence" );
  }

  coral::OracleAccess::Sequence* sequence =
    new coral::OracleAccess::Sequence( m_sessionProperties, m_schemaName, description );
  // The return iterator (TODO: what is this??? bug #90995)
  std::pair< std::map< std::string, coral::OracleAccess::Sequence* >::iterator, bool> ret;
  // Add a new Sequence to the map
  ret = m_sequences.insert( std::pair< std::string, coral::OracleAccess::Sequence* >( description.name(), sequence ) );
  if( !ret.second ) // TODO: what is this??? bug #90995
  {
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Can't add sequence to map " + description.name(),
                                  "Schema::createSequence" );
  }
  return *sequence;
}

//-----------------------------------------------------------------------------

bool
coral::OracleAccess::Schema::existsSequence( const std::string& sequenceName ) const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::existsSequence",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock( m_sequenceMutex );

  return validateSequence( sequenceName );
}

//-----------------------------------------------------------------------------

coral::ISequence&
coral::OracleAccess::Schema::sequenceHandle( const std::string& sequenceName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::sequenceHandle",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock( m_sequenceMutex );

  if( validateSequence( sequenceName ) )
  {
    std::map< std::string, coral::OracleAccess::Sequence* >::iterator i = m_sequences.find( sequenceName );
    return *(i->second);
  }
  throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                "Could not retrieve the sequence " + sequenceName,
                                "Schema::sequenceHandle" );
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::dropSequence( const std::string& sequenceName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::dropSequence",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock( m_sequenceMutex );

  if( validateSequence( sequenceName ) )
  {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Verbose << "Try to drop sequence [" << sequenceName << "]" << coral::MessageStream::endmsg;

    std::map< std::string, coral::OracleAccess::Sequence* >::iterator i = m_sequences.find( sequenceName );
    if( i == m_sequences.end() )
    {
      // This should not happen
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "CRITICAL Sequence " + sequenceName + " error",
                                    "Schema::dropSequence" );
    }
    // Drop the sequence
    // First create the sql statement
    std::auto_ptr<OracleStatement> statement( new OracleStatement( m_sessionProperties, m_schemaName, "DROP SEQUENCE " + i->second->description().name() ) );
    // Execute
    if( !( statement->execute( coral::AttributeList() ) ) )
    {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Can't drop sequence from database " + sequenceName,
                                    "Schema::dropSequence" );
    }
    // Clean up
    delete i->second; // AV what is this?
    m_sequences.erase(i);
  }
  else
    throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                  "Sequence " + sequenceName + " doesn't exists",
                                  "Schema::dropSequence" );
}

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::dropIfExistsSequence( const std::string& sequenceName )
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::dropIfExistsSequence",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock( m_sequenceMutex );

  if( validateSequence( sequenceName ) )
  {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Verbose << "Try to drop sequence [" << sequenceName << "]" << coral::MessageStream::endmsg;

    std::map< std::string, coral::OracleAccess::Sequence* >::iterator i = m_sequences.find( sequenceName );
    // Drop the sequence
    // First create the sql statement
    std::auto_ptr<OracleStatement> statement( new OracleStatement( m_sessionProperties, m_schemaName, "DROP SEQUENCE " + i->second->description().name() ) );
    // Execute
    if( !( statement->execute( coral::AttributeList() ) ) )
    {
      throw coral::SchemaException( m_sessionProperties->domainServiceName(),
                                    "Can't drop sequence from database " + sequenceName,
                                    "Schema::dropIfExistsSequence" );
    }
    // Clean up
    delete i->second; // AV what is this?
    m_sequences.erase(i);
  }
}

//-----------------------------------------------------------------------------

std::set<std::string>
coral::OracleAccess::Schema::listSequences() const
{
  if ( !m_userSessionStarted ) // This schema was "deleted" (bug #80178)
    throw SessionException( "User session has already ended",
                            "Schema::listSequences",
                            m_sessionProperties->domainServiceName() );
  // Local list
  std::set<std::string> list;
  // Lock the sequences
  boost::mutex::scoped_lock lock( m_sequenceMutex );

  // Execute a SQL statement to retrieve all registered sequences
  /*
    SELECT sequence_name FROM all_sequences;
  */
  std::auto_ptr<OracleStatement> statement( new OracleStatement( m_sessionProperties, m_schemaName, "SELECT sequence_name FROM all_sequences" ) );
  /* empty bind data here */
  coral::AttributeList binddata;
  if( statement->execute( binddata ) )
  {
    // Prepare the buffer
    coral::AttributeList res;
    res.extend("SEQUENCE_NAME", typeid(std::string));
    statement->defineOutput( res );
    // Create a cursor with the statement
    Cursor* cursor = new Cursor( statement, res );
    while(cursor->next())
    {
      const coral::AttributeList& row = cursor->currentRow();
      list.insert( row[0].data<std::string>() );
    }
    delete cursor;
  }
  // Return the results
  return list;
}

#endif

//-----------------------------------------------------------------------------

void
coral::OracleAccess::Schema::startUserSession()
{
  // Sanity check: 'creating' an already 'created' schema?
  if ( m_userSessionStarted )
    throw SessionException( "PANIC! User session has already started",
                            "Schema::startUserSession",
                            m_sessionProperties->domainServiceName() );
  m_userSessionStarted = true;
}


void
coral::OracleAccess::Schema::endUserSession()
{
  // Sanity check: 'deleting' an already 'deleted' schema?
  if ( !m_userSessionStarted )
    throw SessionException( "PANIC! User session has already ended",
                            "Schema::endUserSession",
                            m_sessionProperties->domainServiceName() );
  this->reactOnEndOfTransaction();
  m_userSessionStarted = false;
}
