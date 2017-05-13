// $Id: PreparedStatement.cpp,v 1.15 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"
#include "mysqld_error.h"

#include <cstdlib>
#include <sstream>
#include <vector>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/MessageStream.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "ISessionProperties.h"
#include "PreparedStatement.h"

namespace coral
{
  namespace MySQLAccess
  {
    PreparedStatement::PreparedStatement( const ISessionProperties& sessionProperties, const std::string& sql )
      : m_sessionProperties( sessionProperties )
      , m_connection( sessionProperties.connectionHandle() )
      , m_bvars()
      , m_boundInputData( 0 )
      , m_boundOutputData( 0 )
      , m_currentRow( 0 )
      , m_metaData()
      , m_serverVersion( sessionProperties.serverVersion() )

#if (  MYSQL_VERSION_ID > 40100 )
      , m_preparedSqlStatement( 0 )
      , m_binders( 0 )
      , m_bindlens( 0 )
      , m_bindnulls( 0 )
#endif

      , m_sqlStatement( sql )
      , m_markedSqlStatement( sql )
      , m_result( 0 )
      , m_isSelect( false )
    {
      this->prepare( sql );
    }

    PreparedStatement::~PreparedStatement()
    {
      this->reset();
    }

    void PreparedStatement::prepare( const std::string& sqlStatement="" )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      if( ! sqlStatement.empty() )
      {
        // Prepare with a new SQL
        m_sqlStatement       = sqlStatement;
        m_markedSqlStatement = sqlStatement;
      }
      else
      {
        // Just re-initialize
        m_markedSqlStatement = m_sqlStatement;
      }

#if (  MYSQL_VERSION_ID > 40100 )
      coral::MySQLAccess::NamedInputParametersParser nipp;

      // Remember the bind variables' meta data before replacing their names with strings
      m_bvars = nipp.searchAndMark( m_markedSqlStatement );

      for( BVars::const_iterator bvit = m_bvars.begin(); bvit != m_bvars.end(); ++bvit )
      {
        log << coral::Verbose << "Bound variable: " << (*bvit).first << " position: " << (*bvit).second.index << " located at " << (*bvit).second.position << " length " << (*bvit).second.length << std::endl << coral::MessageStream::endmsg;

        // Now replace the bind variables by '?' symbols
        nipp.bindVariable( (*bvit).first, "? ", m_markedSqlStatement );
      }

      if( (m_preparedSqlStatement = mysql_stmt_init( m_sessionProperties.connectionHandle())) == 0 )
      {
        log << coral::Error << "Error while trying to initialize prepared statement: " + sqlStatement << std::endl << coral::MessageStream::endmsg;
        throw coral::SessionException( "Out of memory!" , "MySQLAccess::PreparedStatement", m_sessionProperties.domainProperties().service()->name() );
      }

      // Try to prepare statement
      if( mysql_stmt_prepare( m_preparedSqlStatement, m_markedSqlStatement.c_str(), m_markedSqlStatement.size() ) )
      {
        coral::MySQLAccess::ErrorHandler errHandler;
        MYSQL_HANDLE_STMT_ERR(errHandler,m_preparedSqlStatement,"MySQLAccess::PreparedStatement::prepare");
        log << coral::Error
            << "Error preparing statement: " << m_sqlStatement       << std::endl << coral::MessageStream::endmsg
            << "After marking statement:   " << m_markedSqlStatement << std::endl << coral::MessageStream::endmsg;
        throw coral::SessionException( errHandler.stmtMessage() , "MySQLAccess::PreparedStatement", m_sessionProperties.domainProperties().service()->name() );
      }
#endif

      log << coral::Debug << "Prepared statement : \"" << m_markedSqlStatement << "\"" << std::endl << coral::MessageStream::endmsg;
    }

    void PreparedStatement::bind( const coral::AttributeList& inputData )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      // Remember the input bind parameters
      m_boundInputData = &inputData;

#if (  MYSQL_VERSION_ID > 40100 )
      if( m_preparedSqlStatement == 0 )
      {
        // Not prepared or after reset
        this->prepare();
      }

      // Check & store the number of bind input variables
      unsigned int bvNum = mysql_stmt_param_count( m_preparedSqlStatement );

      if( m_bvars.size() != bvNum )
      {
        log << coral::Error << "Number of bound variables does not match, discovered: " << m_bvars.size() << " after prepare: " << bvNum << std::endl << coral::MessageStream::endmsg; // FIXME - throw an exception here
      }

      // Initialize MYSQL_BIND binders & using bind vars info perform real input params binding directly to the input attribute values
      m_binders   = new MYSQL_BIND[bvNum]; // FIXME - re-write using malloc & memset to not mix malloc/new
      memset(m_binders, 0, bvNum*sizeof(MYSQL_BIND));

      // Initialize nullness indicators
      m_bindnulls = new my_bool[bvNum];
      memset(m_binders, 0, bvNum*sizeof(my_bool));

      // Initialize length indicators
      m_bindlens  = new unsigned long[bvNum];
      memset(m_binders, 0, bvNum*sizeof(unsigned long));

      // The number of actually bound parameters
      unsigned int paramsBound = 0;

      for ( coral::AttributeList::iterator iColumn = (*m_boundInputData).begin(); iColumn != (*m_boundInputData).end(); ++iColumn )
      {
        const std::type_info& colType = iColumn->specification().type();

        // Get the list of discovered bind variables & their index
        std::pair<BVars::const_iterator,BVars::const_iterator> bvars = m_bvars.equal_range( iColumn->specification().name() );

        // Loop over range of the bind variable instances and perform proper positional binding
        for( BVars::const_iterator bvit = bvars.first; bvit != bvars.second; ++bvit )
        {
          if( iColumn->isNull() )
          {
            m_bindnulls[(*bvit).second.index]       = 1;
            m_binders[(*bvit).second.index].is_null = &(m_bindnulls[(*bvit).second.index]);
          }
          else
          {
            if( colType == typeid(std::string) ) // FIXME - We need to distinguish among the SQL types CHAR(n),VARCHAR(n) and *TEXT* variants, look into prepared statement metadata
            {
              m_binders[(*bvit).second.index].buffer        = const_cast<char*>(iColumn->data<std::string>().c_str());
              m_binders[(*bvit).second.index].buffer_length = iColumn->data<std::string>().size()+1;
              m_binders[(*bvit).second.index].is_null       = 0;

              m_bindlens[(*bvit).second.index]              = iColumn->data<std::string>().size()+1;
              m_binders[(*bvit).second.index].length        = &(m_bindlens[(*bvit).second.index]);

              // Try the best we can do to guess the actual SQL type behind the scene
              if( m_binders[(*bvit).second.index].buffer_length < 256 )
                m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_STRING;  // FIXME - How to distinguish the CHAR(n) vs. VARCHAR(n) in MySQL?
              else if( m_binders[(*bvit).second.index].buffer_length >= 256 && m_binders[(*bvit).second.index].buffer_length < 65536 ) // <255,35536>
                m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_BLOB;
              else if( m_binders[(*bvit).second.index].buffer_length >= 65536 && m_binders[(*bvit).second.index].buffer_length < 16777216 ) // <35536,16777216>
                m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_MEDIUM_BLOB;
              else if( m_binders[(*bvit).second.index].buffer_length >= 16777216 ) // && m_binders[(*bvit).second.index].buffer_length < 4294967296 ) // <16777216,4294967296>
                m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_LONG_BLOB;
            }
            else if( colType == typeid(bool) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_TINY;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].buffer_length = sizeof(bool);
              m_binders[(*bvit).second.index].is_null       = 0;

              m_bindlens[(*bvit).second.index]              = sizeof(bool);
              m_binders[(*bvit).second.index].length        = &(m_bindlens[(*bvit).second.index]);
            }
            else if( colType == typeid(coral::Date) )
            {
              MYSQL_TIME* bval                              = new MYSQL_TIME; // FIXME - re-write using malloc & memset to not mix malloc/new
              coral::Date& dval                             = iColumn->data<coral::Date>();

              bval->year                                    = dval.year();
              bval->month                                   = dval.month();
              bval->day                                     = dval.day();

              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_DATE;
              m_binders[(*bvit).second.index].buffer        = bval;
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;
            }
            else if( colType == typeid(coral::TimeStamp) )
            {
              MYSQL_TIME* bval                              = new MYSQL_TIME; // FIXME - re-write using malloc & memset to not mix malloc/new
              coral::TimeStamp& tsval                       = iColumn->data<coral::TimeStamp>();

              bval->year                                    = tsval.year();
              bval->month                                   = tsval.month();
              bval->day                                     = tsval.day();
              bval->hour                                    = tsval.hour();
              bval->minute                                  = tsval.minute();
              bval->second                                  = tsval.second();

              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_TIMESTAMP;
              m_binders[(*bvit).second.index].buffer        = bval;
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;
            }
            else if( colType == typeid(coral::Blob) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_BLOB;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].buffer_length = iColumn->data<coral::Blob>().size();
              m_binders[(*bvit).second.index].is_null       = 0;

              m_bindlens[(*bvit).second.index]              = iColumn->data<coral::Blob>().size();
              m_binders[(*bvit).second.index].length        = &(m_bindlens[(*bvit).second.index]);
            }
            else if( colType == typeid(char) || colType == typeid(unsigned char) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_TINY;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].is_null       = 0;

              if( colType == typeid(char) )
              {
                m_binders[(*bvit).second.index].is_unsigned   = 0;
                m_binders[(*bvit).second.index].buffer_length = sizeof(char);
                m_bindlens[(*bvit).second.index]              = sizeof(char);
                m_binders[(*bvit).second.index].length        = &(m_bindlens[(*bvit).second.index]);
              }
              else
              {
                m_binders[(*bvit).second.index].is_unsigned   = 1;
                m_binders[(*bvit).second.index].buffer_length = sizeof(unsigned char);
                m_bindlens[(*bvit).second.index]              = sizeof(unsigned char);
                m_binders[(*bvit).second.index].length        = &(m_bindlens[(*bvit).second.index]);
              }
            }
            else if( colType == typeid(short) || colType == typeid(unsigned short) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_SHORT;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;

              if( colType == typeid(short) )
                m_binders[(*bvit).second.index].is_unsigned   = 0;
              else
                m_binders[(*bvit).second.index].is_unsigned   = 1;
            }
            else if( colType == typeid(int) || colType == typeid(unsigned int) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_LONG;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;

              if( colType == typeid(int) )
                m_binders[(*bvit).second.index].is_unsigned   = 0;
              else
                m_binders[(*bvit).second.index].is_unsigned   = 1;
            }
            else if( colType == typeid(long) || colType == typeid(unsigned long) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_LONG;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;

              if( colType == typeid(long) )
                m_binders[(*bvit).second.index].is_unsigned   = 0;
              else
                m_binders[(*bvit).second.index].is_unsigned   = 1;
            }
            else if( colType == typeid(long long) || colType == typeid(unsigned long long) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_LONGLONG;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;

              if( colType == typeid(long long) )
                m_binders[(*bvit).second.index].is_unsigned   = 0;
              else
                m_binders[(*bvit).second.index].is_unsigned   = 1;
            }
            else if( colType == typeid(float) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_FLOAT;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;
            }
            else if( colType == typeid(double) )
            {
              m_binders[(*bvit).second.index].buffer_type   = MYSQL_TYPE_DOUBLE;
              m_binders[(*bvit).second.index].buffer        = iColumn->addressOfData();
              m_binders[(*bvit).second.index].is_null       = 0;
              m_binders[(*bvit).second.index].length        = 0;
            }
            else
            {
              // Should not get here
              // FIXME - Throw an exception about unsupported type!
            }
          }

          // Binding of an input parameter ready
          // FIXME - Bind input param

          paramsBound++;
        }

        // FIXME - check if number of bound parameters matches
      }
#else
      coral::MySQLAccess::NamedInputParametersParser nipp;

      for ( coral::AttributeList::iterator iColumn = (*m_boundInputData).begin(); iColumn != (*m_boundInputData).end(); ++iColumn )
      {
        std::ostringstream os;

        // Check if NULL
        if( iColumn->isNull() )
        {
          os << "NULL";
        }
        else
        {
          const std::type_info& colType = iColumn->specification().type();

          if( colType == typeid(std::string) )
          {
            size_t len       = iColumn->data<std::string>().size();
            const char* orig = iColumn->data<std::string>().c_str();
            char* escbuf     = new char[2*len+1];

            mysql_real_escape_string( m_sessionProperties.connectionHandle(), escbuf, orig, len );

            os << "'" << escbuf << "'";
          }
          else if( colType == typeid(bool) )
          {
            os << (int)iColumn->data<bool>();
          }
          else if( colType == typeid(coral::Date) )
          {
            coral::Date& dateVal = iColumn->data<coral::Date>();
            os << "'" << dateVal.year() << "-" << dateVal.month() << "-" << dateVal.day() << "'";
            //os << "'"; iColumn->toOutputStream( os, true ); os << "'";
          }
          else if( colType == typeid(coral::TimeStamp) )
          {
            coral::TimeStamp& tsVal = iColumn->data<coral::TimeStamp>();
            os << "'"
               << tsVal.year() << "-" << tsVal.month()  << "-" << tsVal.day() << " "
               << tsVal.hour() << ":" << tsVal.minute() << ":" << tsVal.second()
               << "'"; // MySQL does not understand this: tsVal.nanosecond() << "'";
            //os << "'"; iColumn->toOutputStream( os, true ); os << "'";
          }
          else if( colType == typeid(coral::Blob) )
          {
            coral::Blob& blobVal = iColumn->data<coral::Blob>();

            char* escaped = new char[ 2*blobVal.size() + 1 ];
            char* buffer  = (char*)(blobVal.startingAddress());

            mysql_real_escape_string( m_sessionProperties.connectionHandle(), escaped, buffer, blobVal.size() );

            os << "'" << escaped << "'";

            delete [] escaped;
          }
          // AV 11-01-2006 single precision was used instead of double
          else if( colType == typeid(double) )
          {
            int p = os.precision();
            os.precision(30);
            os << iColumn->data<double>();
            os.precision(p);
          }
          // AV 11-01-2006 mysql error as empty character was inserted
          else if( colType == typeid(unsigned char) )
          {
            unsigned char uc = iColumn->data<unsigned char>();
            os << (unsigned int)uc;
          }
          else if( colType == typeid(char) )
          {
            char c = iColumn->data<char>();
            if( c == '\\' )
            {
              os << "'\\\\'";
            }
            else if( c == 32 )
            {
              os << 32;
            }
            else
            {
              char escbuf[3];
              mysql_real_escape_string( m_sessionProperties.connectionHandle(), escbuf, &c, 1 );
              os << "'" << escbuf << "'";
            }
            os << (int)c;
          }
          else
          {
            iColumn->toOutputStream( os, true );
          }
        }

        log << coral::Verbose << "Binding variable " << iColumn->specification().name() << " to value " << os.str() << std::endl << coral::MessageStream::endmsg;

        try
        {
          nipp.bindVariable( iColumn->specification().name(), os.str(), m_sqlStatement );
        }
        catch( const std::exception& e )
        {
          log << coral::Error   << "Binding variable " << iColumn->specification().name() << " to value " << os.str()
              << " failed due to: " << e.what()
              << std::endl << coral::MessageStream::endmsg;
        }
      }
#endif

      log << coral::Debug << "Bound variables in prepared SQL: " << m_sqlStatement << std::endl << coral::MessageStream::endmsg;
    }

    bool PreparedStatement::execute( const coral::AttributeList& bindData )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      // Bind the input data values to the variables
      this->bind( bindData );

      unsigned int num_rows;

      if( mysql_query( m_sessionProperties.connectionHandle(), m_sqlStatement.c_str() ) )
      {
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Executing query " + m_sqlStatement));
        if( errorHandler.lastErrorCode()  == ER_DUP_ENTRY )
          throw coral::DuplicateEntryInUniqueKeyException( m_sessionProperties.domainProperties().service()->name() );
        else
          throw coral::SessionException( errorHandler.message(), "MySQLAccess::Statement", m_sessionProperties.domainProperties().service()->name() );
      }
      else // query succeeded, process any data returned by it
      {
        this->m_result = mysql_store_result( m_sessionProperties.connectionHandle() ); // FIXME - provide more sophisticated result set handling for big result sets of unknown size

        if( this->m_result ) // there are rows
        {
          //unsigned int num_fields;
          //num_fields = mysql_num_fields( this->m_result );
          //mysql_num_fields( this->m_result ); // AV icc port: is this needed?
          // retrieve rows, then call mysql_free_result(result)
          this->m_isSelect = true;
          // extract result set metadata
          this->getResultSetInfo();

          num_rows = numberOfRowsProcessed();
          log << coral::Debug << "Selected " << num_rows << " rows" << std::endl << coral::MessageStream::endmsg;
        }
        else // mysql_store_result() returned nothing; should it have?
        {
          if( mysql_field_count( m_sessionProperties.connectionHandle() ) == 0 )
          {
            // query does not return data
            // (it was not a SELECT)
            this->m_isSelect = false;

            num_rows = numberOfRowsProcessed();
            log << coral::Debug << "Affected rows " << num_rows << " rows" << std::endl << coral::MessageStream::endmsg;
          }
          else // mysql_store_result() should have returned data
          {
            ErrorHandler errorHandler;
            MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Getting result for query " + m_sqlStatement));
            throw coral::SessionException( errorHandler.message(), "MySQLAccess::Statement", m_sessionProperties.domainProperties().service()->name() );
          }
        }
      }

      return true;
    }

    unsigned int PreparedStatement::numberOfRowsProcessed() const
    {
      my_ulonglong rows = 0;

      if( (rows = mysql_affected_rows( m_sessionProperties.connectionHandle())) == ((my_ulonglong)-1))
      {
        coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
        ErrorHandler errorHandler;
        MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Getting " + ( (this->m_isSelect) ? std::string("selected") : std::string("affected") ) + " number of rows after statement " + m_sqlStatement));
        throw coral::SessionException( errorHandler.message(), "MySQLAccess::Statement", m_sessionProperties.domainProperties().service()->name() );
      }

      return (unsigned int)rows;
    }

    const std::type_info* PreparedStatement::typeForOutputColumn( int columnId ) const
    {
      const std::type_info* result = 0;

      // FIXME - add result set metadata extraction if any to be returned by the executed query
      if( !m_metaData.empty() && columnId < (int)m_metaData.size() )
      {
        result =  m_metaData[columnId].type;
      }

      return result;
    }

    void PreparedStatement::defineOutput( coral::AttributeList& outputData )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      m_boundOutputData = &outputData;

      log << coral::Verbose << "Bound output buffer:" << std::endl << coral::MessageStream::endmsg;

      for ( coral::AttributeList::const_iterator iColumn = (*m_boundOutputData).begin(); iColumn != (*m_boundOutputData).end(); ++iColumn )
      {
        log << coral::Verbose << iColumn->specification().name() << std::endl << coral::MessageStream::endmsg;
      }
    }

    void PreparedStatement::setNumberOfPrefetchedRows( unsigned int /* numberOfRows */ )
    {
      // FIXME - set up number of prefetched rows
    }

    void PreparedStatement::setCacheSize( unsigned int /* sizeInMB */ )
    {
      // FIXME - set up the size of client buffer cache, if possible
    }

    bool PreparedStatement::fetchNext()
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      MYSQL_ROW row = 0;

      if( this->m_isSelect )
      {
        // Fetch next row from MySQL result set

        if( (row = mysql_fetch_row( this->m_result )) != 0 )
        {
          ++m_currentRow;
          this->copyData( row );
        }
      }

      return( (row != 0 ) ? true : false );
    }

    unsigned int PreparedStatement::currentRowNumber() const
    {
      return m_currentRow;
    }

    void PreparedStatement::getResultSetInfo()
    {
      unsigned int num_fields = mysql_num_fields( this->m_result );
      MYSQL_FIELD * fields     = mysql_fetch_fields( this->m_result );

      for( unsigned int i = 0; i < num_fields; i++)
      {
        //bool        fixedSize = false;
        MYSQL_FIELD f         = fields[i];

        // We need to find out the following:
        Field column =  { "" /* column name */, 0 /* std::type_info */ };

        column.name           = f.name;

        /*
        // Test for for fixed/variable column size
        if( IS_NUM(f.type)                || f.type == FIELD_TYPE_CHAR || f.type == FIELD_TYPE_STRING ||
            f.type == FIELD_TYPE_DATE     || f.type == FIELD_TYPE_TIME ||
            f.type == FIELD_TYPE_DATETIME || f.type == FIELD_TYPE_YEAR )
            fixedSize = true;
        */

        // Check the C++ type out of MySQL type
        //
        // char
        if( f.type == FIELD_TYPE_CHAR ) {
          if( f.length == 1 )
            column.type = &typeid(char);
          else
            column.type = &typeid(std::string);
        }
        // CHAR(n)
        if( f.type == FIELD_TYPE_STRING ) {
          if( f.length == 1 )
            column.type = &typeid(char);
          else
            column.type = &typeid(std::string);
        }
        // bool & char
        if( f.type == FIELD_TYPE_TINY ) {
          if( f.type == FIELD_TYPE_TINY && f.length == 1 )
            column.type = &typeid(bool);
          else if( f.type == FIELD_TYPE_TINY && (f.flags & UNSIGNED_FLAG ) )
            column.type = &typeid(unsigned char);
          else
            column.type = &typeid(char);
        }
        // short int
        if( f.type == FIELD_TYPE_SHORT ) {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned short);
          else
            column.type = &typeid(short);
        }
        // int
        if( f.type == FIELD_TYPE_INT24 ) {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned int);
          else
            column.type = &typeid(int);
        }
        // long int
        if( f.type == FIELD_TYPE_LONG ) {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned long);
          else
            column.type = &typeid(long);
        }
        // long long
        if( f.type == FIELD_TYPE_LONGLONG ) {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned long long);
          else
            column.type = &typeid(long long);
        }
        // decimal or numeric
        if( f.type == FIELD_TYPE_DECIMAL ) {
          if( f.length < 13 )
            column.type = &typeid(float);
          else
            column.type = &typeid(double);
        }
        // float
        if( f.type == FIELD_TYPE_FLOAT )
          column.type = &typeid(float);
        // double precision
        if( f.type == FIELD_TYPE_DOUBLE )
          column.type = &typeid(double);
        // date, time, ... is treated as strings
        if( f.type == FIELD_TYPE_DATE )
          column.type = &typeid(coral::Date);
        // date, time, ... is treated as strings
        if( f.type == FIELD_TYPE_TIMESTAMP )
          column.type = &typeid(coral::TimeStamp);
        // the rest of the tempoaral SQL types is treated as strings
        if( (f.type == FIELD_TYPE_TIME) || (f.type == FIELD_TYPE_DATETIME) || (f.type == FIELD_TYPE_YEAR) )
          column.type = &typeid(std::string);
        // AV 11-01-2006 split varchar from clob/blob
        // varchar
        if( f.type == FIELD_TYPE_VAR_STRING )
          column.type = &typeid(std::string);  // VARCHAR
        // CLOB + BLOB
        if( f.type == FIELD_TYPE_BLOB ) {
          if( f.flags & BINARY_FLAG )
            column.type = &typeid(coral::Blob);  // BLOB
          else
            column.type = &typeid(std::string);  // CLOB
        }
        // Record the column type information
        this->m_metaData.push_back( column );

      }
    }

    void PreparedStatement::reset()
    {
      m_currentRow       = 0;
      m_boundInputData   = 0;
      m_boundOutputData  = 0;
      m_sqlStatement     = "";

#if (  MYSQL_VERSION_ID > 40100 )
      // Release all prepared statement resources if any
      if( m_preparedSqlStatement != 0 )
      {
        if( mysql_stmt_close( m_preparedSqlStatement ) )
        {
          coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
          ErrorHandler errorHandler;
          MYSQL_HANDLE_STMT_ERR(errorHandler,m_preparedSqlStatement,("Closing prepared statement " + m_sqlStatement));
        }
      }

      // Release all bound variables' resources if any
      if( m_binders != 0 )
      {
        delete [] m_binders; // FIXME - Make it std::vector later
        m_binders = 0;
      }

      if( m_bindlens != 0 )
      {
        delete [] m_bindlens;
        m_bindlens = 0;
      }

      if( m_bindnulls != 0 )
      {
        delete [] m_bindnulls;
        m_bindnulls = 0;
      }
#endif

      // Do not assume the query is SELECT ...
      m_isSelect         = false;

      m_metaData.clear();

      // FIXME - make sure this still valid for MySQL prepared statements
      if( this->m_result )
      {
        mysql_free_result( this->m_result ); this->m_result = 0;
      }
    }

    void PreparedStatement::copyData( MYSQL_ROW& /* row */ ) // FIXME - This is gonna be a BIG fix !!!
    {
      size_t numOfColumns = m_metaData.size();
      //unsigned long*     fieldLens;
      //fieldLens = mysql_fetch_lengths( this->m_result ); // Get the field lengths for each field in the result set
      //mysql_fetch_lengths( this->m_result ); // AV icc port: is this needed?

      for( size_t fieldIdx = 0; fieldIdx < numOfColumns; ++fieldIdx )
      {
        (*(this->m_boundOutputData))[fieldIdx].setNull( true );

        //        const std::type_info& rtti_type = *(m_metaData[fieldIdx].type);
        //         if( row[fieldIdx] )
        //         {
        //                if( rtti_type  == typeid(std::string) )               { setField<std::string>        ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(bool) )                      { setField<bool>               ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(char) )                      { setField<char>               ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(unsigned char) )             { setField<unsigned char>      ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(short) )                     { setField<short>              ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(unsigned short) )            { setField<unsigned short>     ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(int) )                       { setField<int>                ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(unsigned int) )              { setField<unsigned int>       ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(long) )                      { setField<long>               ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(unsigned long) )             { setField<unsigned long>      ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(float) )                     { setField<float>              ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(double) )                    { setField<double>             ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(long double) )               { setField<long double>        ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(long long) )                 { setField<long long>          ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(unsigned long long) )        { setField<unsigned long long> ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(coral::Blob) )               { setField<coral::Blob>        ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(coral::Date) )               { setField<coral::Date>        ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //           else if( rtti_type  == typeid(coral::TimeStamp) )          { setField<coral::TimeStamp>   ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
        //         }
        //         else
        //           (*(this->m_boundOutputData))[fieldIdx].setNull(true); // The null value is in this field
      }
    }

  }
}
