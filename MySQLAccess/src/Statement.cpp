// $Id: Statement.cpp,v 1.41 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"
#include "mysqld_error.h"

#include <cstdlib>
#include <iostream>
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
#include "NamedInputParametersParser.h"
#include "SetField.h"
#include "Statement.h"

namespace coral
{
  namespace MySQLAccess
  {

    Statement::Statement( const ISessionProperties& sessionProperties, const std::string& sql )
      : m_sessionProperties( sessionProperties )
      , m_connection( sessionProperties.connectionHandle() )
      , m_boundInputData( 0 )
      , m_boundOutputData( 0 )
      , m_currentRow( 0 )
      , m_nulls()
      , m_metaData()
      , m_serverVersion( sessionProperties.serverVersion() )
      , m_preparedSqlStatement( "" )
      , m_sqlStatement( sql )
      , m_result( 0 )
      , m_isSelect( false )
    {
      this->prepare( sql );
    }

    Statement::~Statement()
    {
      this->reset();
    }

    void Statement::prepare( const std::string& sqlStatement )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      // Make sure we start clean
      this->reset();

      m_preparedSqlStatement = sqlStatement;
      m_sqlStatement         = sqlStatement;

      log << coral::Verbose << "Prepared statement : \"" << m_sqlStatement << "\"" << std::endl << coral::MessageStream::endmsg;
    }

    void Statement::bind( const coral::AttributeList& inputData )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      m_boundInputData = &inputData;

      NamedInputParametersParser nipp;

      m_sqlStatement = m_preparedSqlStatement;

      const BVars bvars = nipp.searchAndMark( m_sqlStatement );

      for( BVars::const_iterator bvit = bvars.begin(); bvit != bvars.end(); ++bvit )
      {
        log << coral::Verbose
            << "Bound variable: " << (*bvit).first << " position: " << (*bvit).second.index
            << " located at " << (*bvit).second.position << " length " << (*bvit).second.length << std::endl << coral::MessageStream::endmsg;
      }

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
            char* escbuf     = new char[4*len+1];

            mysql_real_escape_string( m_sessionProperties.connectionHandle(), escbuf, orig, len );

            os << "'" << escbuf << "'";

            delete [] escbuf;
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
            //             if( c == '\\' )
            //             {
            //               os << "'\\\\'";
            //             }
            //             else if( c == 32 )
            //             {
            //               os << 32;
            //             }
            //             else
            //             {
            //               char escbuf[3];
            //               mysql_real_escape_string( m_sessionProperties.connectionHandle(), escbuf, &c, 1 );
            //               os << "'" << escbuf << "'";
            //             }
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

      log << coral::Verbose << "Bound variables in prepared SQL: " << m_sqlStatement << std::endl << coral::MessageStream::endmsg;
    }

    bool Statement::execute( const coral::AttributeList& bindData )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      // Bind the input data values to the variables
      this->bind( bindData );

      unsigned int num_rows;

      {
        boost::mutex::scoped_lock lock( m_sessionProperties.lock() );

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
            //mysql_num_fields( this->m_result ); // AV icc port: needed?
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
              size_t maxSize = 1024;
              std::string errorMessage;
              if ( m_sqlStatement.size() > maxSize ) {
                errorMessage = m_sqlStatement.substr( 0, maxSize ) + " ...";
              } else {
                errorMessage = m_sqlStatement;
              }
              ErrorHandler errorHandler;
              MYSQL_HANDLE_ERR(errorHandler,m_sessionProperties.connectionHandle(),("Getting result for query " + errorMessage ));
              throw coral::SessionException( errorHandler.message(), "MySQLAccess::Statement", m_sessionProperties.domainProperties().service()->name() );
            }
          } // End of critical section
        }
      }

      return true;
    }

    unsigned int Statement::numberOfRowsProcessed() const
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

    const std::type_info* Statement::typeForOutputColumn( int columnId ) const
    {
      const std::type_info* result = 0;

      // FIXME - add result set metadata extraction if any to be returned by the executed query
      if( !m_metaData.empty() && columnId < (int)m_metaData.size() )
      {
        result =  m_metaData[columnId].type;
      }

      return result;
    }

    void Statement::defineOutput( coral::AttributeList& outputData )
    {
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

      m_boundOutputData = &outputData;

      log << coral::Verbose << "Bound output buffer:" << std::endl << coral::MessageStream::endmsg;

      for ( coral::AttributeList::const_iterator iColumn = (*m_boundOutputData).begin(); iColumn != (*m_boundOutputData).end(); ++iColumn )
      {
        log << coral::Verbose << iColumn->specification().name() << std::endl << coral::MessageStream::endmsg;
      }
    }

    void Statement::setNumberOfPrefetchedRows( unsigned int /* numberOfRows */ )
    {
      // FIXME - set up number of prefetched rows (see bug #64215)
    }

    void Statement::setCacheSize( unsigned int /* sizeInMB */ )
    {
      // FIXME - set up the size of client buffer cache (see bug #64215)
    }

    bool Statement::fetchNext()
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

    unsigned int Statement::currentRowNumber() const
    {
      return m_currentRow;
    }

    void Statement::getResultSetInfo()
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
        if( IS_NUM(f.type) ||
            f.type == MYSQL_TYPE_CHAR ||
            f.type == MYSQL_TYPE_STRING ||
            f.type == MYSQL_TYPE_DATE ||
            f.type == MYSQL_TYPE_TIME ||
            f.type == MYSQL_TYPE_DATETIME ||
            f.type == MYSQL_TYPE_YEAR )
          fixedSize = true;
        */

        // Check the C++ type out of MySQL type
        //
        /*
        // AV 2008-07-17 Fix bug #38968: this entry is duplicated below
        // char
        if( f.type == MYSQL_TYPE_CHAR )              // MYSQL_TYPE_TINY (one)
        {
          if( f.length == 1 )
            column.type = &typeid(char);
          else
            column.type = &typeid(std::string);
        }
        */
        // decimal or numeric
        if( f.type == MYSQL_TYPE_DECIMAL ) // MYSQL_TYPE_DECIMAL
        {
          if( f.length < 13 )
            column.type = &typeid(float);
          else
            column.type = &typeid(double);
        }
        // bool & char
        else if( f.type == MYSQL_TYPE_TINY ) // MYSQL_TYPE_TINY (two)
        {
          if( f.type == MYSQL_TYPE_TINY && f.length == 1 )
            column.type = &typeid(bool);
          else if( f.type == MYSQL_TYPE_TINY && (f.flags & UNSIGNED_FLAG ) )
            column.type = &typeid(unsigned char);
          else
            column.type = &typeid(char);
        }
        // short int
        else if( f.type == MYSQL_TYPE_SHORT ) // MYSQL_TYPE_SHORT
        {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned short);
          else
            column.type = &typeid(short);
        }
        // long int
        else if( f.type == MYSQL_TYPE_LONG ) // MYSQL_TYPE_LONG
        {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned long);
          else
            column.type = &typeid(long);
        }
        // float
        else if( f.type == MYSQL_TYPE_FLOAT ) // MYSQL_TYPE_FLOAT
        {
          column.type = &typeid(float);
        }
        // double precision
        else if( f.type == MYSQL_TYPE_DOUBLE ) // MYSQL_TYPE_DOUBLE
        {
          column.type = &typeid(double);
        }
        // date, time, ... is treated as strings
        else if( f.type == MYSQL_TYPE_TIMESTAMP ) // MYSQL_TYPE_TIMESTAMP
        {
          column.type = &typeid(coral::TimeStamp);
        }
        // long long
        else if( f.type == MYSQL_TYPE_LONGLONG ) // MYSQL_TYPE_LONGLONG
        {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned long long);
          else
            column.type = &typeid(long long);
        }
        // int
        else if( f.type == MYSQL_TYPE_INT24 ) // MYSQL_TYPE_INT24
        {
          if( f.flags & UNSIGNED_FLAG )
            column.type = &typeid(unsigned int);
          else
            column.type = &typeid(int);
        }
        // date, time, ... is treated as strings
        else if( f.type == MYSQL_TYPE_DATE ) // MYSQL_TYPE_DATE
        {
          column.type = &typeid(coral::Date);
        }
        // the rest of the temporal SQL types is treated as strings
        else if( f.type == MYSQL_TYPE_TIME || // MYSQL_TYPE_TIME
                 f.type == MYSQL_TYPE_DATETIME || // MYSQL_TYPE_DATETIME
                 f.type == MYSQL_TYPE_YEAR ) // MYSQL_TYPE_YEAR
        {
          column.type = &typeid(std::string);
        }
        // CLOB + BLOB
        else if( f.type == MYSQL_TYPE_TINY_BLOB  || // MYSQL_TYPE_TINY_BLOB
                 f.type == MYSQL_TYPE_MEDIUM_BLOB || // MYSQL_TYPE_MEDIUM_BLOB
                 f.type == MYSQL_TYPE_LONG_BLOB || // MYSQL_TYPE_LONG_BLOB
                 f.type == MYSQL_TYPE_BLOB ) // MYSQL_TYPE_BLOB
        {
          if( f.flags & BINARY_FLAG )
            column.type = &typeid(coral::Blob);  // BLOB
          else
            column.type = &typeid(std::string);  // CLOB
        }
        // AV 11-01-2006 split varchar from clob/blob
        // varchar
        else if( f.type == MYSQL_TYPE_VAR_STRING ) // MYSQL_TYPE_VAR_STRING
        {
          column.type = &typeid(std::string); // VARCHAR
        }
        // CHAR(n)
        else if( f.type == MYSQL_TYPE_STRING ) // MYSQL_TYPE_STRING
        {
          if( f.length == 1 )
            column.type = &typeid(char);
          else
            column.type = &typeid(std::string);
        }
        // UNSUPPORTED
        else if( f.type == MYSQL_TYPE_NULL  ||
                 f.type == MYSQL_TYPE_NEWDATE ||
                 f.type == MYSQL_TYPE_VARCHAR ||
                 f.type == MYSQL_TYPE_BIT ||
                 f.type == MYSQL_TYPE_NEWDECIMAL ||
                 f.type == MYSQL_TYPE_ENUM ||
                 f.type == MYSQL_TYPE_SET ||
                 f.type == MYSQL_TYPE_GEOMETRY )
          //f.type == MAX_NO_FIELD_TYPES )
        {
          std::stringstream str;
          str << "Column " << f.name << " described by type " << f.type << " in mysql_com.h / enum_field_types";
          throw UnSupportedSqlTypeException(m_sessionProperties.domainProperties().service()->name() , "MySQLAccess::Statement::getResultSetInfo", str.str());
        }
        // UNKNOWN
        else
        {
          std::stringstream str;
          str << "Column " << f.name << " described by UNKNOWN type " << f.type << " in mysql_com.h / enum_field_types";
          throw UnSupportedSqlTypeException(m_sessionProperties.domainProperties().service()->name() , "MySQLAccess::Statement::getResultSetInfo", str.str());
        }

        // Record the column type information
        this->m_metaData.push_back( column );

      }
    }

    void Statement::reset()
    {
      m_currentRow       = 0;
      m_boundInputData   = 0;
      m_boundOutputData  = 0;
      m_sqlStatement     = "";
      m_isSelect         = false;

      m_nulls.clear();
      m_metaData.clear();

      if( this->m_result )
      {
        mysql_free_result( this->m_result ); this->m_result = 0;
      }
    }

    void Statement::copyData( MYSQL_ROW& row )
    {
      size_t numOfColumns = m_metaData.size();
      unsigned long*     fieldLens;
      coral::MySQLAccess::ServerRevision sr = m_sessionProperties.serverRevision();

      fieldLens = mysql_fetch_lengths( this->m_result ); // Get the field lengths for each field in the result set

      for( size_t fieldIdx = 0; fieldIdx < numOfColumns; ++fieldIdx )
      {
        (*(this->m_boundOutputData))[fieldIdx].setNull( true );

        const std::type_info& rtti_type = *(m_metaData[fieldIdx].type);
        if( row[fieldIdx] )
        {
          if( rtti_type  == typeid(std::string) )               { setField<std::string,40>             ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(bool) )                      { setField<bool,40>               ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(char) )                      { setField<char,40>               ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(unsigned char) )             { setField<unsigned char,40>      ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(short) )                     { setField<short,40>              ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(unsigned short) )            { setField<unsigned short,40>     ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(int) )                       { setField<int,40>                ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(unsigned int) )              { setField<unsigned int,40>       ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(long) )                      { setField<long,40>               ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(unsigned long) )             { setField<unsigned long,40>      ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(float) )                     { setField<float,40>              ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(double) )                    { setField<double,40>             ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(long double) )               { setField<long double,40>        ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(long long) )                 { setField<long long,40>          ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(unsigned long long) )        { setField<unsigned long long,40> ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(coral::Blob) )               { setField<coral::Blob,40>        ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(coral::Date) )               { setField<coral::Date,40>        ( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] ); }
          else if( rtti_type  == typeid(coral::TimeStamp) )
          {
            if( (sr.major >= 4 && sr.minor >= 1) || sr.major >= 5 )
            {
              setField<coral::TimeStamp,41>( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] );
            }
            else
            {
              setField<coral::TimeStamp,40>( row[fieldIdx], (*(this->m_boundOutputData))[fieldIdx], fieldLens[fieldIdx] );
            }
          }
        }
        else
          (*(this->m_boundOutputData))[fieldIdx].setNull(true);  // The null value is in this field
      }
    }
  }
}
