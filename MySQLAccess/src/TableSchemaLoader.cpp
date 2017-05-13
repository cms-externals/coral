// $Id: TableSchemaLoader.cpp,v 1.27 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <iostream>
#include <sstream>
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/StringOps.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/TableDescription.h"

#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "SchemaProperties.h"
#include "SessionProperties.h"
#include "TableSchemaLoader.h"

namespace coral
{
  namespace MySQLAccess
  {
    TableSchemaLoader::TableSchemaLoader( coral::TableDescription& description, const ISessionProperties& sessionProperties, ISchemaProperties& schemaProperties )
      : m_tableName( description.name() )
      , m_schemaName( schemaProperties.schemaName() )
      , m_sessionProperties( sessionProperties )
      , m_schemaProperties( schemaProperties )
      , m_description( description )
      , m_columnsFresh( false )

    {
    }

    TableSchemaLoader::~TableSchemaLoader()
    {
    }

    void TableSchemaLoader::refreshTableDescription()
    {
      this->loadTableColumnsDescriptions();
      this->loadIndexDescriptions();
      this->loadForeignKeyDescriptions();
    }

    void TableSchemaLoader::loadTableColumnsDescriptions()
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      MYSQL_RES* result = 0;

      std::string fields_sql = "SHOW COLUMNS FROM \"" + m_schemaName + "\".\"" + m_tableName + "\"";

      if( mysql_query( m_sessionProperties.connectionHandle(), fields_sql.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Loading table schema for \"" + m_schemaName + "\".\"" + m_tableName + "\""));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaLoader", m_sessionProperties.domainProperties().service()->name() );
      }

      if( (result = mysql_store_result( m_sessionProperties.connectionHandle() )) != 0 )
      {
        // mysql> show columns from "test"."T";
        // +-------+---------------------+------+-----+---------+-------+
        // | Field | Type                | Null | Key | Default | Extra |
        // +-------+---------------------+------+-----+---------+-------+


        // We need to find out the following:
        std::string name;
        std::string cpptype;
        std::string sqltype;
        std::string isPk;
        std::vector<std::string> pkcolumns;
        std::string nullFlag;

        MYSQL_ROW row;

        while( (row = mysql_fetch_row(result)) )
        {
          name                    = row[0]; // Column name
          sqltype                 = row[1]; // Column SQL type
          nullFlag                = row[2]; // Is NULL?
          bool notNull            = ((nullFlag == "NO") ? true : false );
          isPk                    = row[3];

          bool fixedSize          = true;
          unsigned int length     = 0;
          //unsigned int precision  = 0;

          //std::cout << " Column " << name << " type " << sqltype << " is " << row[2] << " we detected " << ( notNull ? " NOT NULL" : " default NULL" ) << std::endl;

          // Test for fixed/variable column size
          if( sqltype.find( "var" ) != std::string::npos ||
              sqltype.find( "lob" ) != std::string::npos ||
              sqltype.find( "text" ) != std::string::npos )
          {

            //            std::cout << "Detected variable column " << name
            //                      << " with the SQL type " << sqltype
            //        << " from table " << m_schemaName << "." << m_tableName << std::endl;

            fixedSize = false;

            // Test for CLOB like SQL types and set the column sizes to generate proper table DDL
            if( sqltype == "tinytext" )
              length = 256;
            else if( sqltype == "text" )
              length = 65535;
            else if( sqltype == "mediumtext" )
              length = 16777215;
            else if( sqltype == "longtext" )
              length = 16777216;
            else
              length = 0;  // Should not happen, sorry we set it to default
          }

          // Figure out the column (create) length  if specified
          size_t lenb = std::string::npos;
          size_t lene = std::string::npos;
          size_t penb = std::string::npos;
          //size_t pene = std::string::npos;

          if( (lenb = sqltype.find('(')) != std::string::npos )
          {
            lenb+=1;

            if( (penb = sqltype.find(',')) != std::string::npos )
            {
              lene=penb;
              penb+=1;
              //pene = sqltype.find(')');
            }
            else
            {
              lene = sqltype.find(')');
            }
          }

          if( lene != std::string::npos )
            length    = ::atol( sqltype.substr( lenb, lene - lenb ).c_str() );

          //if( pene != std::string::npos )
          //  precision = ::atol( sqltype.substr( penb, pene - penb ).c_str() );

          // Deduce C++ type from SQL type
          std::string sqlType( sqltype ); std::transform(sqlType.begin(), sqlType.end(), sqlType.begin(), (int (*)(int))toupper);
          cpptype = m_sessionProperties.typeConverter().cppTypeForSqlType( sqlType );

          // Insert the new row metadata into description
          m_description.insertColumn( name, cpptype, length, fixedSize );

          if( notNull )
            m_description.setNotNullConstraint( name );

          if( isPk == "PRI" )
            pkcolumns.push_back( name );
        }

        mysql_free_result( result );

        // If any primary key defined set it now
        if( !pkcolumns.empty() )
          m_description.setPrimaryKey( pkcolumns );
      }
    } // End of loadColumnDescriptions()

    void TableSchemaLoader::loadIndexDescriptions()
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      ErrorHandler errorHandler;

      /*show index from ctmpk;
        +-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+
        | Table | Non_unique | Key_name | Seq_in_index | Column_name | Collation | Cardinality | Sub_part | Packed | Null | Index_type | Comment |
        +-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+
        | ctmpk |          0 | PRIMARY  |            1 | id          | A         |           3 |     NULL | NULL   |      | BTREE      |         |
        | ctmpk |          0 | PRIMARY  |            2 | uid         | A         |           3 |     NULL | NULL   |      | BTREE      |         |
        +-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+
      */
      std::string sql = "SHOW INDEX FROM \"" + m_schemaName + "\".\"" + m_tableName + "\"";

      MYSQL_RES* result = 0;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str()) )
      {
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Getting index list for " + m_schemaName + "." + m_tableName ));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaLoader", m_sessionProperties.domainProperties().service()->name() );
      }

      //       FIXME - to provide complete error checking
      //       if( (result = mysql_store_result( m_sessionProperties.connectionHandle() )) == 0 );
      //       {
      //         MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Getting result set of index list query for " + m_schemaName + "." + m_tableName ));
      //         throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaLoader", m_sessionProperties.domainProperties().service()->name() );
      //       }
      result = mysql_store_result( m_sessionProperties.connectionHandle() );

      std::string lastIndex;
      std::vector<std::string> columns;
      unsigned seq;
      // Set the default value for nonUnique
      bool nonUnique = false;

      MYSQL_ROW row;
      while( (row = mysql_fetch_row(result)) )
      {
        std::istringstream is(row[3]) ; is  >> seq;

        if( seq == 1 ) // First column in the index definition
        {
          if( lastIndex.empty() )
          {
            lastIndex = row[2];
            std::istringstream isb(row[1]); isb >> nonUnique;
          }
          else // There is already an index scanned so register it into description
          {
            try
            {
              if ( lastIndex != "PRIMARY" )
                m_description.createIndex( lastIndex, columns, !nonUnique );
            }
            catch ( coral::UniqueConstraintAlreadyExistingException& )
            {
              /* The primary key is already defined */
            }

            lastIndex = row[2];
            std::istringstream isb(row[1]); isb >> nonUnique;
            columns.clear();
          }
        }
        columns.push_back( row[4] ); // Collect column name
        std::istringstream isb(row[1]); isb >> nonUnique;
      }

      if( !columns.empty() )
      {
        try
        {
          if( lastIndex != "PRIMARY" )
            m_description.createIndex( lastIndex, columns, !nonUnique );
        }
        catch ( coral::UniqueConstraintAlreadyExistingException& )
        { /* The primary key is already defined */ }
      }

      mysql_free_result( result );
    }

    void TableSchemaLoader::loadForeignKeyDescriptions()
    {
      std::string tableDDL;

      this->retrieveTableDDL( tableDDL );

      ServerRevision sr = m_sessionProperties.serverRevision();

      FKConstraints fkc;

      if( sr.major == 5 || ( sr.major == 4 && sr.minor == 1 ) )
        fkc = this->parseFKConstraints( tableDDL );  // MySQL 4.1.x or 5.0.x
      else
        fkc = this->parseFKConstraints40( tableDDL );  // Try MySQL 3.x.x or 4.0.x

      if( ! fkc.empty() )
      {
        for( FKConstraints::const_iterator fki = fkc.begin(); fki != fkc.end(); ++fki )
        {
          //       std::cout << (*fki).FKCname << " fkcs: " << (*fki).FKcolumns.size() << " ";

          //       for( size_t i = 0; i < (*fki).FKcolumns.size(); i++ )
          //         std::cout << ">>" << (*fki).FKcolumns[i] << "<< ";

          //       std::cout << (*fki).PKtable << " pkcs: " << (*fki).PKcolumns.size() << " ";

          //       for( size_t i = 0; i < (*fki).PKcolumns.size(); i++ )
          //         std::cout << ">>" << (*fki).PKcolumns[i] << "<< ";

          //       std::cout << std::endl;

          m_description.createForeignKey( (*fki).FKCname, (*fki).FKcolumns, (*fki).PKtable, (*fki).PKcolumns );
        }
      }
    }

    void TableSchemaLoader::retrieveTableDDL( std::string& tableDDL )
    {
      boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

      ErrorHandler errorHandler;

      std::string sql = "SHOW CREATE TABLE \"" + m_schemaName + "\".\"" + m_tableName + "\"";

      MYSQL_RES* result = 0;

      if( mysql_query( m_sessionProperties.connectionHandle(), sql.c_str()) )
      {
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Getting table DDL for " + m_schemaName + "." + m_tableName ));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaLoader", m_sessionProperties.domainProperties().service()->name() );
      }

      //       FIXME - to provide complete error checking
      //       if( (result = mysql_store_result( m_sessionProperties.connectionHandle() )) == 0 );
      //       {
      //         MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Getting result set of table DDL query for " + m_schemaName + "." + m_tableName ));
      //         throw coral::SessionException( errorHandler.message(), "MySQLAccess::TableSchemaLoader", m_sessionProperties.domainProperties().service()->name() );
      //       }

      result = mysql_store_result( m_sessionProperties.connectionHandle() );

      MYSQL_ROW row;

      while( (row = mysql_fetch_row(result)) )
      {
        tableDDL = row[1]; // row[0] - table name, row[1] - table DDL
      }

      mysql_free_result( result );
    }

    FKConstraints TableSchemaLoader::parseFKConstraints( const std::string& tableDDL )
    {
      FKConstraints c;

      if( tableDDL.empty() )
        return c;

      coral::StringList sl = coral::StringOps::split ( tableDDL, ' ', coral::StringOps::TrimEmpty );

      size_t sll = sl.size();

      bool collectFk = false;
      bool collectPk = false;

      std::vector<std::string> vc, vpkt, vfkc, vpkc;
      std::string sfkc, pkc;
      size_t cidx = 0;

      /*
        CREATE TABLE "T_1" (
        "ID" int(11) NOT NULL default '0',         "x" float NOT NULL default '0',         "Y" double default NULL,         "Z" double default NULL,
        "b" tinyint(1) default NULL,         "d" date default NULL,         "data1" varchar(123) default NULL,         "data2" text,
        "ID_FK" int(11) default NULL,         "cc_FK" varchar(255) character set latin1 collate latin1_bin default NULL,
        PRIMARY KEY  ("ID"),         UNIQUE KEY "Y" ("Y"),         UNIQUE KEY "C_T1" ("x","Z"),
        KEY "T_1_FK" ("ID_FK"),         KEY "T_1_FK_composite" ("ID_FK","cc_FK"),

        CONSTRAINT "T_1_FK" FOREIGN KEY ("ID_FK") REFERENCES "T_0" ("ID"),
        CONSTRAINT "T_1_FK_composite" FOREIGN KEY ("ID_FK", "cc_FK") REFERENCES "T_0" ("ID", "cc")
        )
      */

      for( size_t i = 0; i < sll; i++ )
      {
        if( sl[i] == "CONSTRAINT" )
        {
          vc.push_back( coral::StringOps::remove( sl[i+1] , '"' ) );

          cidx++;

          if( collectPk ) // Primary key(s) from the previous FK constraint if any
          {
            vpkc.push_back( pkc );
            collectPk = false;
            pkc = "";
          }
        }
        if( sl[i] == "FOREIGN" && sl[i+1] == "KEY" )
        {
          collectFk = true;
          i+=2;
        }
        if( sl[i] == "REFERENCES" )
        {
          vpkt.push_back( std::string( coral::StringOps::remove( sl[i+1], '"' )) );

          // List of foreign key columns complete
          collectFk = false;
          vfkc.push_back( sfkc );
          sfkc = "";

          // Start collecting referenced primary key columns
          collectPk = true;
          pkc = "";
          i++;
          i++;
        }
        if( collectFk )
        {
          sfkc += sl[i];
          continue;
        }
        if( collectPk )
        {
          pkc += sl[i];
          if( sl[i].find( "\"),\n") != std::string::npos || sl[i].find( "\")\n" ) == 0 )
          {
            vpkc.push_back( pkc );
            pkc = "";
            collectPk = false;
            continue;
          }
          continue;
        }
        if( sl[i] == "ON" || sl[i] == "TYPE=InnoDB" )
        {
          // FIXME! For the case when ON ... CASCADE is missing
          if( collectPk )
          {
            // List of referenced primary key columns complete
            vfkc.push_back( sfkc );
            vpkc.push_back( pkc );
            collectPk = false;
            sfkc = "";
            pkc = "";
          }
        }
      }
      if( collectPk )
      {
        vpkc.push_back( pkc );
      }

      for( size_t pidx = 0; pidx < cidx; pidx++ )
      {
        ForeignKeyConstraint fkc;

        fkc.FKCname = vc[pidx];
        fkc.PKtable = vpkt[pidx];

        // Extract FK column names
        // First remove braces (,);
        std::string fkcstr = coral::StringOps::remove( vfkc[pidx], '(' ); fkcstr = coral::StringOps::remove( fkcstr    , ')' );

        // Next remove comma ','
        fkcstr = coral::StringOps::remove( fkcstr    , ',' );
        fkcstr = coral::StringOps::remove( fkcstr    , '\n' );


        // Finaly split by MySQL (back)quotes "`"
        coral::StringList fkcsl = coral::StringOps::split( fkcstr, '"', coral::StringOps::TrimEmpty );
        for( size_t fki = 0; fki < fkcsl.size(); fki++ )
        {
          fkc.FKcolumns.push_back( fkcsl[fki] );
        }

        // Extract referenced PK column names
        // First remove braces;
        std::string pkcstr = coral::StringOps::remove( vpkc[pidx], '(' ); pkcstr = coral::StringOps::remove( pkcstr    , ')' );

        // Next remove ','
        pkcstr = coral::StringOps::remove( pkcstr    , ',' );
        // Remove new line if any
        pkcstr = coral::StringOps::remove( pkcstr    , '\n' );

        // Finaly split by ANSI double quotes
        coral::StringList pkcsl = coral::StringOps::split( pkcstr, '"', coral::StringOps::TrimEmpty );
        for( size_t pki = 0; pki < pkcsl.size(); pki++ )
        {
          fkc.PKcolumns.push_back( pkcsl[pki] );
        }

        c.push_back( fkc );
      }

      return c;
    }

    FKConstraints TableSchemaLoader::parseFKConstraints40( const std::string& tableDDL )
    {
      FKConstraints c;

      if( tableDDL.empty() )
        return c;

      coral::StringList sl = coral::StringOps::split ( tableDDL, ' ', coral::StringOps::TrimEmpty );

      size_t sll = sl.size();

      bool collectFk = false;
      bool collectPk = false;

      std::vector<std::string> vc, vpkt, vfkc, vpkc;
      std::string sfkc, pkc;
      size_t cidx = 0;

      for( size_t i = 0; i < sll; i++ )
      {
        if( sl[i] == "CONSTRAINT" )
        {
          vc.push_back(coral::StringOps::remove( sl[i+1], '`' ));
          cidx++;
          // AV 10-01-2006
          if( collectPk )
          {
            vfkc.push_back( sfkc );
            vpkc.push_back( pkc );
            collectPk = false;
            sfkc = "";
            pkc = "";
          }
        }
        if( sl[i] == "FOREIGN" && sl[i+1] == "KEY" )
        {
          collectFk = true;
          i+=2;
        }
        if( sl[i] == "REFERENCES" )
        {
          vpkt.push_back( std::string( coral::StringOps::remove( sl[i+1], '`' )) );

          // List of foreign key columns complete
          collectFk = false;

          // Start collecting referenced primary key columns
          collectPk = true;
          i+=2;
        }
        if( collectFk )
        {
          sfkc += sl[i];
          continue;
        }
        if( sl[i] == "ON" || sl[i] == "TYPE=InnoDB" )
        {
          // FIXME! For the case when ON ... CASCADE is missing
          if( collectPk )
          {
            // List of referenced primary key columns complete
            vfkc.push_back( sfkc );
            vpkc.push_back( pkc );
            collectPk = false;
            sfkc = "";
            pkc = "";
          }
        }
        if( collectPk )
        {
          pkc += sl[i];
          continue;
        }
      }

      for( size_t pidx = 0; pidx < cidx; pidx++ )
      {
        ForeignKeyConstraint fkc;

        fkc.FKCname = vc[pidx];
        fkc.PKtable = vpkt[pidx];

        // Extract FK column names
        // First remove braces (,);
        std::string
          fkcstr = coral::StringOps::remove( vfkc[pidx], '(' );
        fkcstr = coral::StringOps::remove( fkcstr    , ')' );
        // Next remove comma ','
        fkcstr = coral::StringOps::remove( fkcstr    , ',' );
        fkcstr = coral::StringOps::remove( fkcstr    , '\n' );

        // Finaly split by MySQL (back)quotes "`"
        coral::StringList fkcsl = coral::StringOps::split( fkcstr, '`', coral::StringOps::TrimEmpty );
        for( size_t fki = 0; fki < fkcsl.size(); fki++ )
        {
          fkc.FKcolumns.push_back( fkcsl[fki] );
        }

        // Extract referenced PK column names
        // First remove braces;
        std::string
          pkcstr = coral::StringOps::remove( vpkc[pidx], '(' );
        pkcstr = coral::StringOps::remove( pkcstr    , ')' );
        // Next remove ','
        pkcstr = coral::StringOps::remove( pkcstr    , ',' );
        // AV 10-01-2006
        //fkcstr = coral::StringOps::remove( fkcstr    , '\n' );
        pkcstr = coral::StringOps::remove( pkcstr    , '\n' );

        // Finaly split by MySQL "`" quotes
        coral::StringList pkcsl = coral::StringOps::split( pkcstr, '`', coral::StringOps::TrimEmpty );
        for( size_t pki = 0; pki < pkcsl.size(); pki++ )
        {
          fkc.PKcolumns.push_back( pkcsl[pki] );
        }

        c.push_back( fkc );
      }

      return c;
    }
  }
}
