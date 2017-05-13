// $Id: Statement.cpp,v 1.41 2011/03/22 10:36:50 avalassi Exp $
#include <cstdlib>
#include <sstream>
#include <vector>
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/IWebCacheControl.h"
#include "RelationalAccess/IWebCacheInfo.h"
#include "frontier_client/frontier-cpp.h"
#include "frontier_client/FrontierException.hpp"

#include "ISessionProperties.h"
#include "DomainProperties.h"
#include "DomainPropertyNames.h"
#include "NamedInputParametersParser.h"
#include "Statement.h"

// Use empty namespace to avoid name conflicts
namespace {

  // Prototype class to parse the Oracle timestamp to coral timestamp
  // This class can also be moved later to a common area

  class TimestampParser
  {

  public:

    TimestampParser(const char* timestamp)
      : m_time( 0 )
    {
      if( !timestamp )
        throw coral::Exception( "TimestampParser", "timestamp was zero", "coral::TimestampParser::constructor" );

      // the format supported is: 'YYYY-MM-DD hh:mm:ss'
      m_time = new coral::TimeStamp( boost::posix_time::time_from_string(std::string(timestamp)));
    }

    ~TimestampParser()
    {
      if( m_time ) delete m_time;
    }

    coral::TimeStamp& getTimeStamp()
    {
      return *m_time;
    }

  private:

    coral::TimeStamp* m_time;

  };

}

namespace coral
{
  namespace FrontierAccess
  {
    Statement::Statement( const coral::FrontierAccess::ISessionProperties& properties, const std::string& sqlStatement )
      : m_properties( const_cast<coral::FrontierAccess::ISessionProperties&>( properties ) )
      , m_session( 0 )
      , m_listOfRequests()
      , m_boundInputData( 0 )
      , m_boundOutputData( 0 )
      , m_currentRow( 0 )
      , m_nulls()
      , m_metaData()
      , m_serverVersion( properties.serverVersion() )
      , m_preparedSqlStatement( sqlStatement )
      , m_sqlStatement( sqlStatement )
      , m_field( new frontier::AnyData )
      , m_mslog( 0 )
    {
      this->m_session = new frontier::Session( &(m_properties.connection()) );

      mslog() << coral::Verbose << "*** statement#1 :" << sqlStatement << coral::MessageStream::endmsg;

      this->prepare( m_sqlStatement );
      mslog() << coral::Verbose << "*** statement#2 :" << sqlStatement << coral::MessageStream::endmsg;
    }

    Statement::~Statement()
    {
      this->reset();

      if( m_mslog != 0 )
      {
        delete m_mslog;
        m_mslog = 0;
      }
    }

    bool Statement::prepare( const std::string& sqlStatement )
    {
      // Prepare the statement
      m_preparedSqlStatement = sqlStatement;
      m_sqlStatement         = sqlStatement;

      mslog() << coral::Verbose << "Prepared statement : \"" << m_sqlStatement << "\"" << coral::MessageStream::endmsg;

      return true;
    }

    bool Statement::bind( const coral::AttributeList& inputData )
    {
      m_boundInputData = &inputData;

      coral::FrontierAccess::NamedInputParametersParser nipp;
      mslog() << coral::Verbose << "*** statement#5 :" << m_sqlStatement << coral::MessageStream::endmsg;

      m_sqlStatement = m_preparedSqlStatement;
      mslog() << coral::Verbose << "*** statement#6 :" << m_sqlStatement << coral::MessageStream::endmsg;

      //       const BVarsPositions& bvars = nipp.searchAndMark( m_sqlStatement );
      //       for( BVarsPositions::const_iterator bvit = bvars.begin(); bvit != bvars.end(); ++bvit )
      //       {
      //         mslog() << coral::Verbose << "Bound variable: " << (*bvit).first << " at " << (*bvit).second.first << " length " << (*bvit).second.second << coral::MessageStream::endmsg;
      //       }

      for ( coral::AttributeList::const_iterator iColumn = (*m_boundInputData).begin(); iColumn != (*m_boundInputData).end(); ++iColumn )
      {

        mslog() << coral::Verbose << " bin data " << iColumn->specification().name() << coral::MessageStream::endmsg;

        const std::type_info& colType = iColumn->specification().type();
        if( colType == typeid(std::string) )
        {
          std::string value = iColumn->data<std::string>();
          value = "'" + value + "'";
          nipp.bindVariable( iColumn->specification().name(), value, m_sqlStatement );
        }
        else if( colType == typeid(coral::Blob) )
        {
          // FIXME - not really needed, as one hardly select on BLOB type
          ;
        }
        else if( colType == typeid(coral::Date) )
        {
          std::stringstream os;
          const coral::Date& dateVal = iColumn->data<coral::Date>();
          os << "'" << dateVal.year() << "-" << dateVal.month() << "-" << dateVal.day() << "'";
          nipp.bindVariable( iColumn->specification().name(), os.str(), m_sqlStatement );
        }
        else if( colType == typeid(coral::TimeStamp) )
        {
          std::ostringstream os;
          const coral::TimeStamp& tsVal = iColumn->data<coral::TimeStamp>();
	  boost::posix_time::time_facet* facet = new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S%F");
	  os.imbue(std::locale(os.getloc(), facet));
	  os << "TO_TIMESTAMP('"<<tsVal.time()<<"','YYYY-MM-DD HH24:MI:SS.FF')";
          nipp.bindVariable( iColumn->specification().name(), os.str(), m_sqlStatement );
        }
        else if( colType == typeid(bool) )
        {
          nipp.bindVariable( iColumn->specification().name(), ( iColumn->data<bool>() ? "1" : "0"), m_sqlStatement );
        }
        else
        {
          std::stringstream os; iColumn->toOutputStream(os,true);
          nipp.bindVariable( iColumn->specification().name(), os.str(), m_sqlStatement );
        }
      }

      //       mslog() << coral::Verbose << "Bound variables in prepared SQL: " << m_sqlStatement << coral::MessageStream::endmsg;

      return true;
    }

    bool Statement::execute( const coral::AttributeList& rowBuffer, int timeToLive )
    {
      try
      {
        // Bind first
        mslog() << coral::Verbose << "*** statement#3 :" << m_sqlStatement << coral::MessageStream::endmsg;

        this->bind( rowBuffer );

        std::string req_data="frontier_request:1:DEFAULT";

        mslog() << coral::Verbose << "*** statement#4 :" << m_sqlStatement << coral::MessageStream::endmsg;


        std::string param;
        {
          std::string sql = m_sqlStatement;

          sql += '\n';
          boost::mutex::scoped_lock lock( m_properties.lock() );
          param = frontier::Request::encodeParam( sql );
        }

        frontier::Request* request = new frontier::Request( req_data , frontier::BLOB );

        request->addKey( "p1", param );

        m_listOfRequests.push_back( request );

        {
          boost::mutex::scoped_lock lock( m_properties.lock() );

          // Locate the web cache control and set up the compression level for the request
          try
          {
            coral::IWebCacheControl& cachectrl = const_cast<coral::IWebCacheControl&>( m_properties.cacheControl() );
            request->setRetrieveZipLevel( cachectrl.compressionLevel() );
            mslog() << coral::Debug << "Running with the compression level: " << cachectrl.compressionLevel() << coral::MessageStream::endmsg;
          }
          catch( const std::exception& )
          {
            mslog() << coral::Debug << "Running with the default compression level" << coral::MessageStream::endmsg;
          }

          try
          {
            // Set timeToLive value on the connection
            m_properties.connection().setTimeToLive( timeToLive );
            const char *ttlName = "";
            if ( timeToLive == 1 ) ttlName = "short ";
            else if ( timeToLive == 3 ) ttlName = "forever ";
            mslog() << coral::Verbose << "Executing " << ttlName << "query " << m_sqlStatement << coral::MessageStream::endmsg;

            // Perform the execution
            m_session->getData( m_listOfRequests );
            m_session->setCurrentLoad( 1 );

            // Extract result set metadata
            // Seek to the first record, which is supposed to be result set metadata
            // metadata consists of one record with field names.
            //         int result = m_connection->next();
            m_session->next();

            //         mslog() << coral::Verbose << "frontier::DataSource::next() returned " << result << coral::MessageStream::endmsg;
            //         mslog() << coral::Verbose << "Result set metadata:" << coral::MessageStream::endmsg;

            while( ! (m_session->isEOR()) )
            {
              Field f;
              m_session->assignString( &(f.name) );
              m_session->assignString( &(f.type) );
              m_metaData.push_back( f );
              m_nulls.push_back( 0 );
              //           mslog() << coral::Verbose << f.name <<" => "<< f.type << coral::MessageStream::endmsg;
            }

            mslog() << coral::Debug << "Result for query " << m_sqlStatement << " contains " << this->numberOfRowsProcessed() << " rows" << coral::MessageStream::endmsg;

            m_properties.log().push_back( coral::FrontierAccess::LogEntry( m_sqlStatement ) );
          }
          catch( const std::exception& )
          {
            throw;
          }
        }
      }
      catch( const std::exception& e )
      {
        throw coral::QueryException( m_properties.domainProperties().service()->name(), e.what(), "coral::FrontierAccess::Statement::execute" );
      }

      return true;
    }

    unsigned int Statement::numberOfRowsProcessed() const
    {
      return( m_session->getRecNum() );
    }

    const std::type_info* Statement::typeForOutputColumn( int columnId ) const
    {
      const coral::ITypeConverter& typeConverter = m_properties.typeConverter();

      // We assume that statement has been already executed and we have the Frontier MetaRequest data available
      const std::type_info* result = 0;

      if( !m_metaData.empty() && columnId < (int)m_metaData.size() )
      {
        std::string sqlType = m_metaData[columnId].type;
        if ( sqlType == "NUMBER(63)" ) sqlType = "FLOAT(63)";  // Fix bug #70208
        else if ( sqlType == "NUMBER(126)" ) sqlType = "FLOAT(126)";  // Fix bug #70208
        result = coral::AttributeSpecification::typeIdForName( typeConverter.cppTypeForSqlType( sqlType ) );
        //std::cout << "SQL: " << m_metaData[columnId].type << " [" << sqlType << "], C++: " << result->name() << std::endl;
      }

      return result;
    }

    bool Statement::defineOutput( coral::AttributeList& outputData )
    {
      m_boundOutputData = &outputData;

      //       mslog() << coral::Verbose << "Bound output buffer:" << coral::MessageStream::endmsg;
      //       for ( coral::AttributeList::const_iterator iColumn = (*m_boundOutputData).begin(); iColumn != (*m_boundOutputData).end(); ++iColumn )
      //       {
      //         mslog() << coral::Verbose << iColumn->specification().name() << coral::MessageStream::endmsg;
      //       }

      return true;
    }

    bool Statement::setNumberOfPrefetchedRows( unsigned int /* numberOfRows */ )
    {
      // Not supported (see bug #64215)
      return true;
    }

    bool Statement::setCacheSize( unsigned int /* sizeInMB */ )
    {
      // Not supported (see bug #64215)
      return true;
    }

    bool Statement::fetchNext()
    {
      int dataAvailable = 0;

      try
      {
        dataAvailable = m_session->next();
      }
      catch( const std::exception& e )
      {
        throw coral::QueryException( m_properties.domainProperties().service()->name(), e.what(), "coral::FrontierAccess::Statement::fetchNext" );
      }

      if( dataAvailable )
      {
        ++m_currentRow;
        this->copyData();
      }

      return dataAvailable;
    }

    unsigned int Statement::currentRowNumber() const
    {
      return m_currentRow;
    }

    void Statement::reset()
    {
      m_currentRow = 0;
      m_boundInputData  = 0;
      m_boundOutputData  = 0;

      for( std::vector<const frontier::Request*>::size_type i = 0; i < m_listOfRequests.size(); i++ )
        delete m_listOfRequests[i];

      if( m_session != 0 )
        delete m_session;

      m_listOfRequests.clear();
      m_metaData.clear();
      m_sqlStatement = "";

      m_nulls.clear();

      if( m_field != 0 )
      {
        delete m_field;
        m_field = 0;
      }
    }

    template <class T> inline void setField( const char*, coral::Attribute&, unsigned long )
    {
      throw coral::QueryException( "unknown", "no default setField type implementation is available", "coral::FrontierAccess::Statement::setField" );
    }

    template <> inline void setField<bool>( const char* buffer, coral::Attribute& attr, unsigned long len )
    {
      bool value;
      if( ( *buffer == '0' ) && ( len == 1 ) )
        value = false;
      else
        value = true;
      attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<std::string>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      std::string value( buffer ); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<char>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      // Not clear if we have unsigned or signed char
      const coral::AttributeSpecification& spec = attr.specification();

      if( spec.type() == typeid(std::string) )
      {
        std::string value(buffer);
        attr.setValueFromAddress( (void*)(&value) );
      }
      else
      {
        attr.setValueFromAddress( (void*)(buffer) );
      }
      attr.setNull( false );
    }
    template <> inline void setField<unsigned char>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      unsigned char value=0; value=::atoi(buffer); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<short>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      short value=0; value=::atoi(buffer); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }
    template <> inline void setField<unsigned short>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      unsigned short value=0; value=::atoi(buffer); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<int>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      int value=0; value=::atoi(buffer); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }
    template <> inline void setField<unsigned int>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      unsigned int value=0; value=::strtoul(buffer,0,10); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<long>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      long value=0; value=::strtol(buffer,0,10); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }
    template <> inline void setField<unsigned long>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      unsigned long value=0; value=::strtoul(buffer,0,10); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<long long>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      long long value=0; value=::strtoll(buffer,0,10); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }
    template <> inline void setField<unsigned long long>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      unsigned long long value=0; value=::strtoull(buffer,0,10); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<float>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      float value=0.; value=::atof(buffer); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<double>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      double value=0.; value=::strtod(buffer,0); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> inline void setField<long double>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      double long value=0.; value=::strtold(buffer,0); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> void setField<coral::Date>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      std::string dbuf = buffer;

      int year  = atoi( dbuf.substr( 0, 4 ).c_str() );
      int month = atoi( dbuf.substr( 5, 2 ).c_str() );
      int day   = atoi( dbuf.substr( 8    ).c_str() );

      coral::Date value( year, month, day );

      attr.setValueFromAddress( (void*)(&value) );
    }

    template <> void setField<coral::Blob>( const char* buffer, coral::Attribute& attr, unsigned long size )
    {
      coral::Blob&   blobVal = attr.data<coral::Blob>();
      unsigned char* blobRow = (unsigned char*)buffer;
      blobVal.resize( size );
      unsigned char* bvp     = (unsigned char*)blobVal.startingAddress();
      ::memcpy( bvp, blobRow, blobVal.size() );
    }

    template <> void setField<coral::TimeStamp>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      // Start with a timestamp parser
      TimestampParser timestamp(buffer);
      // Bind the timestamp to the attribute
      attr.setValueFromAddress( (void*)(&(timestamp.getTimeStamp())) );
      //attr.bind( timestamp.getTimeStamp() );
    }

    bool Statement::copyData()
    {
      size_t numOfColumns      = m_metaData.size();
      frontier::AnyData& field = *m_field;

      // Loop over the current record fields and extract data into bound data attribute list
      for( size_t fieldIdx = 0; fieldIdx < numOfColumns; fieldIdx++ )
      {
        if( m_session->isEOR() )
          break;

        //         std::string& fieldName = m_metaData[fieldIdx].name;
        std::string& fieldType = m_metaData[fieldIdx].type;

        //         mslog() << coral::Verbose << "Loading field " << fieldIdx << " " << fieldName << " of type " << fieldType << " out of " << numOfColumns << " fields" << coral::MessageStream::endmsg;

        // Fill the field
        m_session->getAnyData( &field );

        // According to its type set properly the bound attribute list data
        switch( field.type() )
        {
          //case frontier::BLOB_TYPE_BYTE:       vc=ds.getByte(); break;
        case frontier::BLOB_TYPE_INT4:
          {
            int vi = field.getInt()     ;
            (*m_boundOutputData)[fieldIdx].setValue<int>( vi );
            break;
          }
        case frontier::BLOB_TYPE_INT8:
          {
            long long vl = field.getLongLong();
            (*m_boundOutputData)[fieldIdx].setValue<unsigned long long>( vl );
            break;
          }
        case frontier::BLOB_TYPE_FLOAT:
          {
            float vf = field.getFloat()   ;
            (*m_boundOutputData)[fieldIdx].setValue<float>( vf )    ;
            break;
          }
        case frontier::BLOB_TYPE_DOUBLE:
          {
            double vd = field.getDouble()  ;
            (*m_boundOutputData)[fieldIdx].setValue<double>( vd )   ;
            break;
          }
        case frontier::BLOB_TYPE_TIME:
          {
            long long vl = field.getLongLong();
            (*m_boundOutputData)[fieldIdx].setValue<long long>( vl );
            break;
          }
        case frontier::BLOB_TYPE_ARRAY_BYTE:
          {
            if( field.getRawStrP() != NULL )
            {
              const std::type_info& rtti_type = (*m_boundOutputData)[fieldIdx].specification().type();

              const char* strBuffer = field.getRawStrP();
              unsigned int strLen   = field.getRawStrS();

              if( fieldType.find( "NUMBER" ) != std::string::npos )
              {
                if( rtti_type  == typeid(bool) )                      { setField<bool>                      ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(unsigned char) )             { setField<unsigned char>             ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(short) )                     { setField<short>                     ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(unsigned short) )            { setField<unsigned short>            ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(int) )                       { setField<int>                       ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(unsigned int) )              { setField<unsigned int>              ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(long) )                      { setField<long>                      ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(unsigned long) )             { setField<unsigned long>             ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(float) )                     { setField<float>                     ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(double) )                    { setField<double>                    ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(long double) )               { setField<long double>               ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(long long) ) { setField<long long> ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else if( rtti_type  == typeid(unsigned long long) ) { setField<unsigned long long> ( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen ); }
                else
                {
                  std::ostringstream msg;
                  msg << "Unknown rtti_type " << rtti_type.name();
                  throw coral::QueryException( m_properties.domainProperties().service()->name(), msg.str(), "coral::FrontierAccess::Statement::copyData" );
                }
              }
              else if( fieldType == "BINARY_FLOAT" )
              {
                setField<float>( strBuffer, (*m_boundOutputData)[fieldIdx], strLen );
              }
              else if( fieldType == "BINARY_DOUBLE" )
              {
                setField<double>( strBuffer, (*m_boundOutputData)[fieldIdx], strLen );
              }
              else if( fieldType == "CHAR" )
              {
                setField<char>( strBuffer, (*m_boundOutputData)[fieldIdx], strLen );
              }
              else if( fieldType == "DATE" )
              {
                setField<coral::Date>( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen );
              }
              else if( fieldType == "TIMESTAMP" )
              {
                setField<coral::TimeStamp>( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen );
              }
              else if( fieldType == "BLOB" )
              {
                setField<coral::Blob>( strBuffer, (*(this->m_boundOutputData))[fieldIdx], strLen );
              }
              else if( ( fieldType == "VARCHAR2" ) || ( fieldType == "CLOB" ) )
              {
                setField<std::string>( strBuffer, (*m_boundOutputData)[fieldIdx], strLen );
              }
              else
              {
                std::string msg = "Unknown fieldType ";
                msg.append(fieldType);
                throw coral::QueryException( m_properties.domainProperties().service()->name(), msg, "coral::FrontierAccess::Statement::copyData" );
              }
            }
            else
            {
              (*m_boundOutputData)[fieldIdx].setNull();
              m_nulls[fieldIdx] = 1;
            }

            break;
          }
        default:
          {
            std::ostringstream msg;
            msg << "Unknown typeID " << ((int)(field.type()));
            throw coral::QueryException( m_properties.domainProperties().service()->name(), msg.str(), "coral::FrontierAccess::Statement::copyData" );
          }
        };

        field.clean();
      }

      field.clean();

      return true;
    }

    coral::MessageStream& Statement::mslog()
    {
      if( this->m_mslog == 0 )
        this->m_mslog = new  coral::MessageStream( m_properties.domainProperties().service()->name() );

      return *(this->m_mslog);
    }
  }
}
