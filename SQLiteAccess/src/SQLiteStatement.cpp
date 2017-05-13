#include <algorithm>
#include <climits>
#include <cstdlib>
#include <iostream>
#include "sqlite3.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Date.h"
#include "CoralBase/Exception.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralCommon/SimpleTimer.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "SessionProperties.h"
#include "SQLiteCommon.h"
#include "SQLiteStatement.h"
#include "StatementStatistics.h"

using namespace coral::SQLiteAccess;

SQLiteStatement::SQLiteStatement( boost::shared_ptr<const SessionProperties> properties ) :
  m_properties( properties ),
  m_stmt( 0 ),
  m_rowcounter( 0 ),
  m_statementStatistics( 0 ),
  m_rollBack( false )
{
  if (m_properties->monitoringService()) m_statementStatistics = new StatementStatistics();
}


bool
SQLiteStatement::prepare( const std::string& statement )
{
  if ( statement == "ROLLBACK" )
  {
    m_rollBack = true;
  }

  //#ifdef _DEBUG
  //else {
  //    coral::MessageStream log( m_properties->domainProperties().service(),
  //                             m_properties->domainProperties().service()->name(),
  //                             coral::Debug );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log << Debug << "SQLiteStatement::prepare " << statement << MessageStream::endmsg;
  //#endif

  if(m_statementStatistics) m_statementStatistics->sqlStatement=statement;
  //if you reuse an already preparedstatement, release it first (bug #71449)
  if ( m_stmt ) finalize();
  //lock the sqlite3 methods
  m_properties->mutex()->lock();
  //fill the sqlite handle, with the prepared statement
  int rs = sqlite3_prepare( m_properties->dbHandle(), statement.c_str(), statement.length(), &m_stmt, 0);
  //std::cout << "PREPARED statement " << m_stmt << " " << statement.c_str() << std::endl;
  m_properties->mutex()->unlock();

  if( rs != SQLITE_OK )
  {
    //  coral::MessageStream log( m_properties->domainProperties().service(),
    //m_properties->domainProperties().service()->name() );
    log << Error << "SQLiteStatement::prepare " << rs << " " << sqlite3_errmsg( m_properties->dbHandle() ) << MessageStream::endmsg;
    return false;
  }
  return true;
}


int
SQLiteStatement::step()
{
  if (!m_stmt) throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::step",m_properties->domainProperties().service()->name());
  //m_log<<coral::Debug<<"SQLiteStatement::step"<<coral::MessageStream::endmsg;
  //lock the sqlite3 methods
  m_properties->mutex()->lock();
  int rs = sqlite3_step(m_stmt);
  m_properties->mutex()->unlock();
  return rs;
}


bool
SQLiteStatement::bind( const coral::AttributeList& inputData )
{
  if (!m_stmt) throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::bind",m_properties->domainProperties().service()->name());
  //    coral::MessageStream log( m_properties->domainProperties().service(),
  //                             m_properties->domainProperties().service()->name(),
  //                             coral::Debug );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  if((int)inputData.size() != sqlite3_bind_parameter_count(m_stmt) )
  {
    log<<coral::Error<<"SQLiteStatement::bind wrong number of bind variables"<<coral::MessageStream::endmsg;
    return false;
  }
  log<<coral::Debug<<"SQLiteStatement::bind variables ";
  int rs = 0;
  for(coral::AttributeList::iterator iAttribute = inputData.begin(); iAttribute != inputData.end(); ++iAttribute )
  {
    //lock the sqlite3 methods
    m_properties->mutex()->lock();
    std::string attributeName = iAttribute->specification().name();
    //std::cout<<attributeName<<std::endl;
    int idx = sqlite3_bind_parameter_index(m_stmt, ( std::string(":") + attributeName).c_str() );
    if(iAttribute != inputData.begin())
    {
      log << Debug << " , ";
    }
    log<<coral::Debug<<attributeName;
    //      std::cout<<"attributeName: "<<idx<<std::endl;
    //checking nulless
    if( iAttribute->isNull() )
    {
      rs = sqlite3_bind_null(m_stmt,idx);
      log << Debug << " NULL ";
    }
    else
    {
      const std::type_info& attributeType = iAttribute->specification().type();
      SQLiteType st;
      void* addr = iAttribute->addressOfData();
      if(st(attributeType) == SQLT_INT)
      {
        castAttributeToInt c;
        int value = c(addr,attributeType);
        rs = sqlite3_bind_int(m_stmt,idx,value );
        log << coral::Debug << "(int) " << value << " ";
      }
      else if(st(attributeType) == SQLT_UIN)
      {
        castAttributeToUInt c;
        long long value=c(addr,attributeType);
        rs=sqlite3_bind_int64(m_stmt,idx,value );
        log<<coral::Debug<<"(unsigned int) "<<value<<" ";
      }
      else if(st(attributeType)==SQLT_64INT)
      {
        castAttributeTo64Int c;
        long long int value=c(addr,attributeType);
        rs=sqlite3_bind_int64(m_stmt,idx,value);
        log<<coral::Debug<<"(long long int) "<<value<<" ";
      }
      else if(st(attributeType)==SQLT_64UIN)
      {
        castAttributeTo64UInt c;
        unsigned long long int value=c(addr,attributeType);
        rs=sqlite3_bind_int64(m_stmt,idx,value);
        log<<coral::Debug<<"(unsigned long long int) "<<value<<" ";
      }
      else if(st(attributeType)==SQLT_DOUBLE)
      {
        castAttributeToDouble c;
        double value=c(addr,attributeType);
        rs=sqlite3_bind_double(m_stmt,idx,value);
        log<<coral::Debug<<"(double) "<<value<<" ";
      }
      else if(st(attributeType)==SQLT_TEXT)
      {
        std::string val=(*iAttribute).data<std::string>();
        rs=sqlite3_bind_text(m_stmt,idx,val.c_str(),val.length(),SQLITE_TRANSIENT);
        log<<coral::Debug<<"(string) "<<val<<" ";
      }
      else if(st(attributeType)==SQLT_BLOB)
      {
        long blobsize=iAttribute->data<coral::Blob>().size();
        if( blobsize==0 )
        {
          const char key='a';
          //rs=sqlite3_bind_zeroblob(m_stmt,idx, 0);
          rs=sqlite3_bind_blob(m_stmt,idx,&key,0,SQLITE_TRANSIENT);
        }
        else
        {
          void* blobdataaddress=iAttribute->data<coral::Blob>().startingAddress();
          rs=sqlite3_bind_blob(m_stmt, idx,blobdataaddress,(int)blobsize,SQLITE_TRANSIENT);
        }
        log<<coral::Debug<<"(BLOB) of size "<<blobsize<<" ";
      }
      else if(st(attributeType)==SQLT_DATE )
      {
        coral::TimeStamp::ValueType value=coral::TimeStamp((*iAttribute).data<coral::Date>().time()).total_nanoseconds();
        rs=sqlite3_bind_int64(m_stmt,idx,(long long int)value);
        log<<coral::Debug<<"(DATE) "<<value<<" ";
      }
      else if(st(attributeType)==SQLT_TIMESTAMP)
      {
        coral::TimeStamp::ValueType value=(*iAttribute).data<coral::TimeStamp>().total_nanoseconds();
        rs=sqlite3_bind_int64(m_stmt,idx,(long long int)value);
        log<<coral::Debug<<"(TIME) "<<value<<" ";
      }
    }
    m_properties->mutex()->unlock();

    if( rs != SQLITE_OK )
    {
      //coral::MessageStream log( m_properties->domainProperties().service(),
      //                         m_properties->domainProperties().service()->name() );
      log << Error << "SQLiteStatement::bind " << rs << " " << sqlite3_errmsg( m_properties->dbHandle() ) << coral::MessageStream::endmsg;
      return false;
    }
  }
  log<<coral::Debug<<coral::MessageStream::endmsg;
  return true;
}


bool
SQLiteStatement::setNumberOfPrefetchedRows( unsigned int /* numberOfRows */ )
{
  //dummy now (see bug #64215)
  return true;
}


bool
SQLiteStatement::setCacheSize( unsigned int /* sizeInMB */ )
{
  //dummy now (see bug #64215)
  return true;
}


bool
SQLiteStatement::execute()
{
  if (!m_stmt) throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::execute",m_properties->domainProperties().service()->name());
  //m_log<<coral::Debug<<"SQLiteStatement::execute"<<coral::MessageStream::endmsg;

  coral::SimpleTimer::ValueType t0=0, t1=0;
  coral::SimpleTimer timer;
  timer.start();

  if ( m_statementStatistics ) t0 = timer.sample();
  int rs = this->step();
  if( rs == SQLITE_DONE )
  {
    //lock the sqlite3 methods
    m_properties->mutex()->lock();
    m_rowcounter=(unsigned int)sqlite3_changes( m_properties->dbHandle() );
    m_properties->mutex()->unlock();

    if ( m_statementStatistics ) {
      t1 = timer.sample();
      double idleTimeSeconds = ( t1-t0 ) * 1e-9;
      m_statementStatistics->idleTime = idleTimeSeconds;
      m_properties->monitoringService()->record( m_properties->connectionString(), coral::monitor::Statement,coral::monitor::Time,m_statementStatistics->sqlStatement, m_statementStatistics->idleTime);
    }
    return true;
  }
  if ( m_statementStatistics )
  {
    t1 = timer.sample();
    double idleTimeSeconds = ( t1-t0 ) * 1e-6;
    m_statementStatistics->idleTime = idleTimeSeconds;
    m_properties->monitoringService()->record( m_properties->connectionString(), coral::monitor::Statement,coral::monitor::Time,m_statementStatistics->sqlStatement, m_statementStatistics->idleTime);
  }
  if ( ! m_rollBack )
  {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    // AV&MW - several changes to fix bug #45716
    //log<<coral::Error<<"SQLiteStatement::execute "<<rs<<" "<<sqlite3_errmsg(  m_properties->dbHandle() )<<coral::MessageStream::endmsg; // AV: NOT OK before finalize (eg "1 SQL logic error or missing database")
    this->finalize(); // AV&MW - fix bug #45716
    log<<coral::Error<<"SQLiteStatement::execute "<<rs<<" "<<sqlite3_errmsg(  m_properties->dbHandle() )<<coral::MessageStream::endmsg; // AV: OK after finalize (eg "1 column CHANNEL_ID is not unique")
    throw coral::Exception(sqlite3_errmsg(  m_properties->dbHandle() ),"SQLiteStatement::execute",m_properties->domainProperties().service()->name());
  }
  return false;
}


bool
SQLiteStatement::fetchNext()
{
  //std::cout<<"===== ANALYSIS CURSOR FETCHNEXT ======"<<std::endl;
  //std::cout<<"===== ANALYSIS CURSOR FETCHNEXT ------>"<<m_stmt<<std::endl;
  if (!m_stmt) throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::fetchNext",m_properties->domainProperties().service()->name());

  if( !m_properties->dbHandle() ) // fix bug #79983 (see bug #80022)
    throw QueryException( m_properties->domainProperties().service()->name(),
                          "The sqlite3 database handle is no longer valid",
                          "SQLiteStatement::fetchNext" );

  coral::SimpleTimer timer;

  if ( m_statementStatistics ) timer.start();

  int rs=this->step();
  if( rs==SQLITE_ROW ) {
    if ( m_statementStatistics )
    {
      timer.stop();
      double idleTimeSeconds = timer.total() * 1e-6;
      m_statementStatistics->idleTime += idleTimeSeconds;
    }
    m_rowcounter++;
    return true;
  }
  if( rs==SQLITE_DONE ) {
    if ( m_statementStatistics ) {
      m_properties->monitoringService()->record( m_properties->connectionString(),
                                                 coral::monitor::Statement,
                                                 coral::monitor::Time,m_statementStatistics->sqlStatement,
                                                 m_statementStatistics->idleTime );
    }

    this->reset();
    return false;
  }
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Error<<"SQLiteStatement::fetchNext "<<rs<<" "<<sqlite3_errmsg( m_properties->dbHandle() )<<coral::MessageStream::endmsg; ///should throw here!
  if( m_statementStatistics ) {
    m_properties->monitoringService()->record( m_properties->connectionString(), coral::monitor::Statement, coral::monitor::Time,m_statementStatistics->sqlStatement, m_statementStatistics->idleTime );
  }
  return false;
}


bool
SQLiteStatement::defineOutput( coral::AttributeList& outputData )
{
  if (!m_stmt) throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::defineOutput",m_properties->domainProperties().service()->name());

  //m_log<<coral::Debug<<"SQLiteStatement::defineOutput"<<coral::MessageStream::endmsg;
  unsigned int idx = 0;
  if ( outputData.size() == 0 ) return true;
  for ( coral::AttributeList::iterator iAttribute = outputData.begin(); iAttribute != outputData.end(); ++iAttribute, ++idx )
  {
    //lock the sqlite3 methods
    m_properties->mutex()->lock();
    int rs = sqlite3_column_type( m_stmt, idx);
    m_properties->mutex()->unlock();

    if( rs == SQLITE_NULL)
    {
      iAttribute->setNull(true);
    }
    else
    {
      iAttribute->setNull(false);
      const std::type_info& attributeType = iAttribute->specification().type();
      SQLiteType st;
      if( st(attributeType) == SQLT_INT )
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        int result = sqlite3_column_int(m_stmt,idx);
        m_properties->mutex()->unlock();

        if(attributeType==typeid(int)) {
          iAttribute->data<int>()=result;
        }else if( attributeType==typeid(short)) {
          iAttribute->data<short>()=(short)result;
        }else if( attributeType==typeid(char)) {
          iAttribute->data<char>()=(char)result;
        } else if( attributeType==typeid(long)) {
          iAttribute->data<long>()=(long)result;
        }
      }
      else if(st(attributeType)==SQLT_UIN)
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        unsigned int result = (unsigned int)sqlite3_column_int(m_stmt,idx);
        m_properties->mutex()->unlock();

        if( attributeType==typeid(unsigned short)) {
          iAttribute->data<unsigned short>()=(unsigned short)result;
        }else if( attributeType==typeid(bool)) {
          iAttribute->data<bool>()=(result!=0);
        }else if( attributeType==typeid(unsigned char)) {
          iAttribute->data<unsigned char>()=(unsigned char)result;
        }
      }
      else if( st(attributeType) == SQLT_64INT )
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        long long int result = sqlite3_column_int64(m_stmt,idx);
        m_properties->mutex()->unlock();

        if(attributeType==typeid(long long int)) {
          iAttribute->data<long long int>()=result;
        }else if( attributeType==typeid(long) ) {
          iAttribute->data<long>()=(long)result;
        }
      }
      else if( st(attributeType) == SQLT_64UIN )
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        unsigned long long int result = (unsigned long long int)sqlite3_column_int64(m_stmt,idx);
        m_properties->mutex()->unlock();

        if(attributeType==typeid(unsigned int)) {
          iAttribute->data<unsigned int>()=(unsigned int)result;
        }else if( attributeType==typeid(unsigned long)) {
          iAttribute->data<unsigned long>()=(unsigned long)result;
        }else if(attributeType==typeid(unsigned long long int)) {
          iAttribute->data<unsigned long long int>()=result;
        }
      }
      else if( st(attributeType) == SQLT_DOUBLE )
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        double result = sqlite3_column_double(m_stmt,idx);
        m_properties->mutex()->unlock();

        if(attributeType==typeid(double)) {
          iAttribute->data<double>()=result;
        }else if( attributeType==typeid(float)) {
          iAttribute->data<float>()=(float)result;
        }else if( attributeType==typeid(long double)) {
          iAttribute->data<long double>()=(long double)result;
        }
      }
      else if( st(attributeType) == SQLT_TEXT )
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        const char* result = (const char*)sqlite3_column_text(m_stmt,idx);
        m_properties->mutex()->unlock();

        iAttribute->data<std::string>() = std::string(result);
      }
      else if( st(attributeType) == SQLT_BLOB )
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        const void* result = sqlite3_column_blob(m_stmt,idx);
        int s = sqlite3_column_bytes(m_stmt,idx);
        m_properties->mutex()->unlock();

        coral::Blob& attrblob=iAttribute->data<coral::Blob>();
        attrblob.resize(s);
        void* attrblobad=attrblob.startingAddress();
        ::memcpy(attrblobad, result, s);
      }
      else if( (st(attributeType) == SQLT_DATE) || (st(attributeType) == SQLT_TIMESTAMP) )
      {
        //lock the sqlite3 methods
        m_properties->mutex()->lock();
        long long int result = sqlite3_column_int64(m_stmt,idx);
        m_properties->mutex()->unlock();

        coral::TimeStamp t(result);
        if( st(attributeType)==SQLT_DATE ) {
          iAttribute->data<coral::Date>()=coral::Date(t.time());
        }else{
          iAttribute->data<coral::TimeStamp>()=t;
        }
      }else{
        coral::MessageStream log( m_properties->domainProperties().service()->name() );
        log << coral::Error << "Unsupported type : \"" + iAttribute->specification().typeName() + "\"" << coral::MessageStream::endmsg;
        this->reset();
        return false;
      }
    } //end-of-if
  } //end-of-for
  return true;
}


unsigned int
SQLiteStatement::numberOfRowsProcessed() const
{
  //m_log<<coral::Debug<<"SQLiteStatement::numberOfRowsProcessed "<<m_rowcounter<<coral::MessageStream::endmsg;
  return m_rowcounter;
}


const std::type_info*
SQLiteStatement::decltypeForColumn( int columnId ) const
{
  if (!m_stmt) throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::decltypeForColumn",m_properties->domainProperties().service()->name());
  /*#ifdef _DEBUG
      coral::MessageStream log( m_properties->domainProperties().service(),
      m_properties->domainProperties().service()->name(),
      coral::Nil );
      log<<coral::Debug<<"SQLiteStatement::decltypeForColumn "<<columnId<<coral::MessageStream::endmsg;
      #endif
  */
  //const coral::ITypeConverter& typeConverter=m_properties->typeConverter();
  //lock the sqlite3 methods
  m_properties->mutex()->lock();
  const char* t = sqlite3_column_decltype(m_stmt, columnId);

  if ( t == 0 )
    throw coral::Exception("could not determine type for output column, consider using defineOutput()!",
                           "SQLiteStatement::decltypeForColumn", m_properties->domainProperties().service()->name());
  m_properties->mutex()->unlock();

  return coral::AttributeSpecification::typeIdForName( m_properties->typeConverter().cppTypeForSqlType(std::string(t)) );
}


bool
SQLiteStatement::reset()
{
  if (!m_stmt) throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::reset",m_properties->domainProperties().service()->name());
  //m_log<<coral::Debug<<"SQLiteStatement::reset"<<coral::MessageStream::endmsg;
  //lock the sqlite3 methods
  m_properties->mutex()->lock();
  int rs = sqlite3_reset(m_stmt);
  m_properties->mutex()->unlock();

  if(rs!=SQLITE_OK ) {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<<coral::Error<<"SQLiteStatement::reset "<<rs<<" "<<sqlite3_errmsg(  m_properties->dbHandle() )<<coral::MessageStream::endmsg;
    return false;
  }
  m_rowcounter=0;
  if(m_statementStatistics) m_statementStatistics->reset();
  return true;
}


void
SQLiteStatement::finalize()
{
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  if (!m_stmt)
  {
    //log<<coral::Warning<<"m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)"<<coral::MessageStream::endmsg;
    //throw coral::Exception("m_stmt is NULL (SQLiteStatement::prepare failed or m_stmt has already been deleted?)","SQLiteStatement::finalize",m_properties->domainProperties().service()->name()); // AV too strict - this fails all the time
    return;
  }
  //m_log<<coral::Debug<<"SQLiteStatement::finalize"<<coral::MessageStream::endmsg;
  //lock the sqlite3 methods
  m_properties->mutex()->lock();

  int rs=sqlite3_finalize(m_stmt);

  //std::cout << "FINALIZED statement " << m_stmt << std::endl;

  m_properties->mutex()->unlock();

  m_stmt=0; // AV&MW - patch for bug #45716
  if(rs!=SQLITE_OK ) {
    if ( ! m_rollBack ) {
      log<<coral::Error<<"SQLiteStatement::finalize "<<rs<<" "<<sqlite3_errmsg(  m_properties->dbHandle() )<<coral::MessageStream::endmsg;
    }
  }
}


SQLiteStatement::~SQLiteStatement()
{
  //m_log<<coral::Debug<<"SQLiteStatement::~SQLiteStatement"<<coral::MessageStream::endmsg;
  this->finalize(); // AV&MW - fix bug #45716
  if(m_statementStatistics) delete m_statementStatistics;
}


boost::shared_ptr<const SessionProperties>
SQLiteStatement::sessionProperties() const
{
  return m_properties;
}
