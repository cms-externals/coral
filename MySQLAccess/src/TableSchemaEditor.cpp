// $Id: TableSchemaEditor.cpp,v 1.25 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <cstdio>
#include <sstream>
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "CoralBase/boost_thread_headers.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

#include "ColumnConstraint.h"
#include "ColumnDefinition.h"
#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "SchemaProperties.h"
#include "SessionProperties.h"
#include "TableSchemaEditor.h"
#include "TableSchemaLoader.h"

namespace coral
{
  namespace MySQLAccess
  {
    TableSchemaEditor::TableSchemaEditor( const std::string& tableName, const ISessionProperties& sessionProperties, ISchemaProperties& schemaProperties, bool refresh /*=true*/ )
      : m_tableName( tableName )
      , m_schemaName( sessionProperties.schemaName() )
      , m_sessionProperties( sessionProperties )
      , m_schemaProperties( schemaProperties )
      , m_description( (*(schemaProperties.tableDescriptionRegistry().find( m_tableName ))).second )
      , m_tableSchemaLoader( 0 )
    {
      m_tableSchemaLoader = new TableSchemaLoader( *m_description, sessionProperties, schemaProperties );

      if( refresh )
        m_tableSchemaLoader->refreshTableDescription();
    }

    TableSchemaEditor::~TableSchemaEditor()
    {
      if( m_tableSchemaLoader )
      {
        delete m_tableSchemaLoader;
        m_tableSchemaLoader = 0;
      }
    }

    void TableSchemaEditor::insertColumn( const std::string& name, const std::string& type, int size /*= 0*/, bool fixedSize /*= true*/, std::string /*tableSpaceName = ""*/ )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaProperties.schemaName() + "\".\"" + m_tableName + "\" ADD (";
      sql += this->buildColumn( name, type, size, fixedSize );
      sql += " )";


      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Inserting column  \"" + name + "\" into table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::dropColumn( const std::string& name )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" DROP COLUMN \"" + name + "\"";

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Dropping column \"" + name + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::renameColumn( const std::string& originalName, const std::string& newName )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      // ALTER TABLE CHANGE is Oracle extension supported as well by MySQL
      // However MySQL requires that column type is present apparently due to missing
      // data dictionary which Oracle can use to deduce column type.
      // It may work in MySQL 5.0, to be checked

      // Get original column SQL type
      const coral::ITableDescription& descr  = m_schemaProperties.tableDescription( m_tableName );
      const coral::IColumn&           column = descr.columnDescription( originalName );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" CHANGE COLUMN \"" + originalName + "\" " + this->buildColumn( newName, column.type(), column.size(), column.isSizeFixed() );

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Renaming column \"" + originalName + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::changeColumnType( const std::string& columnName, const std::string& typeName, int size /*= 0*/, bool fixedSize /*= true*/ )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" MODIFY " + this->buildColumn( columnName, typeName, size, fixedSize );

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Changing column type of \"" + columnName + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::setNotNullConstraint( const std::string& columnName, bool isNotNull /*= true*/ )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      // ALTER TABLE MODIFY is Oracle extension supported as well by MySQL
      // However MySQL requires that column type is present apparently due to missing
      // data dictionary which Oracle can use to deduce column type.
      // It may work in MySQL 5.0, to be checked

      // Get original column SQL type
      const coral::ITableDescription& descr  = m_schemaProperties.tableDescription( m_tableName );
      const coral::IColumn&           column = descr.columnDescription( columnName );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" MODIFY " + this->buildColumn( columnName, column.type(), column.size(), column.isSizeFixed() );

      if ( isNotNull )
        sql += " NOT ";

      sql += " NULL ";

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Setting NOT NULL constraint for column \"" + columnName + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::setUniqueConstraint( const std::string& columnName, std::string name /*= ""*/, bool isUnique /*= true*/, std::string tableSpaceName /*= ""*/ )
    {
      this->setUniqueConstraint( std::vector<std::string>( 1, columnName ), name, isUnique, tableSpaceName );
    }

    void TableSchemaEditor::setUniqueConstraint( const std::vector<std::string>& columnNames, std::string name /*= ""*/, bool isUnique /*= true*/, std::string /*tableSpaceName = ""*/ )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" ";

      if ( isUnique )
      {
        sql += " ADD " + this->buildConstraint( coral::MySQLAccess::Unique, columnNames, name, columnNames, name, isUnique );
      }
      else
      {
        sql += "DROP INDEX \"" + name + "\" ";
      }

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Setting unique constraint \"" + name + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::setPrimaryKey( const std::string& columnName, std::string tableSpaceName /*= ""*/ )
    {
      this->setPrimaryKey( std::vector<std::string>( 1, columnName ), tableSpaceName );
    }

    void TableSchemaEditor::setPrimaryKey( const std::vector<std::string>& columnNames, std::string /*tableSpaceName = ""*/ )
    {
      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" ADD " + this->buildConstraint( coral::MySQLAccess::Primary, columnNames, m_tableName+"_PK", columnNames, "PRIMARY", true );

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Setting primary key constraint in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::dropPrimaryKey()
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" DROP PRIMARY KEY";

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Dropping primary key constraint in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    void TableSchemaEditor::createIndex( const std::string& indexName, const std::string& columnName, bool isUnique /*= false*/, std::string tableSpaceName /*= ""*/ )
    {
      this->createIndex( indexName, std::vector<std::string>( 1, columnName ), isUnique, tableSpaceName );
    }

    void TableSchemaEditor::createIndex( const std::string& name, const std::vector<std::string>& columnNames, bool isUnique /*= false*/, std::string /*tableSpaceName = ""*/ )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::stringstream sql;

      sql << "CREATE ";

      if ( isUnique )
        sql << "UNIQUE ";

      sql << "INDEX \"" << name << "\" ON \"" << m_schemaName << "\".\"" << m_tableName << "\"(";

      for ( size_t iColumn = 0; iColumn < columnNames.size(); ++iColumn )
      {
        if ( iColumn > 0 )
          sql << ",";

        sql << "\"" + columnNames[iColumn] + "\"";

        // FIXME - MySQL required workaround for CLOB types in case they are used as unique key, will be done properly in future CORAL releases
        // FIXME - as CORAL provides required interface changes to support index prefixes for all types of constraints
        const coral::IColumn& column = m_schemaProperties.tableDescription(m_tableName).columnDescription( columnNames[iColumn] );
        if( *(coral::AttributeSpecification::typeIdForName( column.type() )) == typeid( std::string ) && column.size() > 255 )
        { /* MySQL can store it as CLOB only */
          sql << "(" <<  checkEnvClobPrefixLength() << ")";
        }
      }
      sql << ")";

      std::string sqlstr = sql.str();

      if( mysql_query( m_sessionProperties.connectionHandle(), sqlstr.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Creating index \"" + name + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }


    void TableSchemaEditor::dropIndex( const std::string& indexName )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "DROP INDEX \"" + indexName + "\"";

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Dropping index \"" + indexName + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

#ifdef CORAL240DC
    void TableSchemaEditor::createForeignKey( const std::string& name, const std::string& columnName, const std::string& referencedTableName, const std::string& referencedColumnName, bool onDeleteCascade )
    {
      this->createForeignKey( name, std::vector<std::string>( 1, columnName ), referencedTableName, std::vector<std::string>( 1, referencedColumnName ) , onDeleteCascade );
    }
#else
    void TableSchemaEditor::createForeignKey( const std::string& name, const std::string& columnName, const std::string& referencedTableName, const std::string& referencedColumnName )
    {
      this->createForeignKey( name, std::vector<std::string>( 1, columnName ), referencedTableName, std::vector<std::string>( 1, referencedColumnName ) );
    }
#endif

#ifdef CORAL240DC
    void TableSchemaEditor::createForeignKey( const std::string& name, const std::vector<std::string>& columnNames, const std::string& referencedTableName, const std::vector<std::string>& referencedColumnNames, bool /*onDeleteCascade*/ )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" ADD " + this->buildConstraint( coral::MySQLAccess::Foreign, columnNames, name, referencedColumnNames, referencedTableName, false );

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Creating foreign key constraint \"" + name + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }
#else
    void TableSchemaEditor::createForeignKey( const std::string& name, const std::vector<std::string>& columnNames, const std::string& referencedTableName, const std::vector<std::string>& referencedColumnNames )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" ADD " + this->buildConstraint( coral::MySQLAccess::Foreign, columnNames, name, referencedColumnNames, referencedTableName, false );

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Creating foreign key constraint \"" + name + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }
#endif

    void TableSchemaEditor::dropForeignKey( const std::string& name )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      std::string sql = "ALTER TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\" DROP FOREIGN KEY \""+ name + "\"";

      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Debug << "Alter Table DDL: " << sql << std::endl << coral::MessageStream::endmsg;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Dropping foreign key constraint \"" + name + "\" in table " + m_schemaName + "." + m_tableName));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaEditor", m_sessionProperties.domainProperties().service()->name() );
      }
    }

    std::string TableSchemaEditor::buildColumn( const std::string& name, const std::string& type, int size /*= 0*/, bool fixedSize /*= true*/ )
    {
      ColumnDefinition coldef;

      coldef.name     = name;
      coldef.type     = m_sessionProperties.typeConverter().sqlTypeForCppType( type );
      coldef.length   = size;
      coldef.fixed    = fixedSize;

      if( *(coral::AttributeSpecification::typeIdForName(type)) == typeid( std::string ) )
        coldef.domain = coral::MySQLAccess::String;
      else if( *(coral::AttributeSpecification::typeIdForName( type )) == typeid( coral::Blob ) )
        coldef.domain = coral::MySQLAccess::Blob;
      else
        coldef.domain = coral::MySQLAccess::Numeric;

      return( coldef.sql() );
    }

    std::string TableSchemaEditor::buildConstraint( ConstraintType ct,
                                                    const std::vector<std::string>& columnNames, const std::string& name,
                                                    const std::vector<std::string>& refColumnNames, const std::string& refTableName,
                                                    bool /*isUnique*/ )
    {
      ColumnConstraint colcons;

      colcons.type     = ct;
      colcons.name     = name;

      // FIXME - MySQL required workaround for CLOB types in case they are used as unique key, will be done properly in future CORAL releases
      // FIXME - as CORAL provides required interface changes to support index prefixes for all types of constraints
      size_t numColumns                         = columnNames.size();
      for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
      {
        const coral::IColumn& column = m_schemaProperties.tableDescription(m_tableName).columnDescription( columnNames[iColumn] );
        if( *(coral::AttributeSpecification::typeIdForName( column.type() )) == typeid( std::string ) && column.size() > 255 )
        { /* MySQL can store it as CLOB only */
          colcons.keys.push_back( IndexedColumn( column.name(), checkEnvClobPrefixLength() ) );
        }
        else
          // FIXME - TODO index prefix in future CORAL releases
          colcons.keys.push_back( IndexedColumn( column.name(), 0 /* Means don't use index prefix */ ) );  // FIXME
      }

      if( ct == coral::MySQLAccess::Foreign )
      {
        colcons.refColumns = refColumnNames;
        colcons.refTable   = refTableName;
      }

      return( colcons.sql() );
    }

    unsigned int TableSchemaEditor::checkEnvClobPrefixLength() const
    {
      unsigned int clobPrefixLength = 32;
      const char* clobPrefixEnvSetting = getenv( "CORAL_MYSQL_CLOB_PREFIX_LEN" );
      if( clobPrefixEnvSetting != 0 )
        // FIXME - max. allowed prefix len in MySQL 4.0, for 4.1 & later it can be longer
        if( (sscanf( clobPrefixEnvSetting, "%ud", &clobPrefixLength ) != 1) || clobPrefixLength > 255 ) // FIXME
          clobPrefixLength = 32;
      return clobPrefixLength;
    }
  }
}
