#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralKernel/Service.h"
#include "DomainProperties.h"
#include "OracleErrorHandler.h"
#include "OutputVariableHolder.h"

namespace {

  // Callback function used by OCILobRead when reading BLOBs,
  // context pointer passed to callback must point to a coral::Blob
  sb4
  blobReadCallback ( dvoid* ctxp, CONST dvoid* bufp, ub4 len, ub1 )
  {
    coral::Blob* blob = (coral::Blob*)ctxp ;

    // extend blob
    long size = blob->size() ;
    blob->extend( len ) ;

    // copy new data
    std::copy ( (char*)bufp, (char*)bufp+len, (char*)blob->startingAddress()+size ) ;

    return OCI_CONTINUE;
  }

  // Callback function used by OCILobRead when reading CLOBs,
  // context pointer passed to callback must point to std::string object
  sb4
  clobReadCallback ( dvoid* ctxp, CONST dvoid* bufp, ub4 len, ub1 )
  {
    std::string* str = (std::string*)ctxp ;

    // extend string
    str->append ( (char*)bufp, len ) ;

    return OCI_CONTINUE;
  }



}


coral::OracleAccess::DateOutputVariableHolder::DateOutputVariableHolder( coral::Date& date ) :
  m_date( date ),
  m_ociDate( new OCIDate )
{
}


coral::OracleAccess::DateOutputVariableHolder::~DateOutputVariableHolder()
{
  if ( m_ociDate ) delete m_ociDate;
}


bool
coral::OracleAccess::DateOutputVariableHolder::copyData()
{
  sb2 year;
  ub1 month, day;
  OCIDateGetDate( m_ociDate, &year, &month, &day );
  m_date = coral::Date( year, month, day );
  return true;
}

/// Constructor
coral::OracleAccess::TimeOutputVariableHolder::TimeOutputVariableHolder( coral::TimeStamp& timeStamp,
                                                                         boost::shared_ptr<const SessionProperties> sessionProperties ) :
  m_time( timeStamp ),
  m_sessionProperties( sessionProperties ),
  m_ociDateTime( 0 )
{
  // Construct a time stamp descriptor
  void* temporaryPointer = 0;
  sword status = OCIDescriptorAlloc( m_sessionProperties->ociEnvHandle(),
                                     &temporaryPointer, OCI_DTYPE_TIMESTAMP, 0, 0 );
  if ( status != OCI_SUCCESS ) {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << "Could not allocate a TIMESTAMP descriptor" << coral::MessageStream::endmsg;
  }
  else {
    m_ociDateTime = static_cast< OCIDateTime*>( temporaryPointer );
  }
}

coral::OracleAccess::TimeOutputVariableHolder::~TimeOutputVariableHolder()
{
  if ( m_ociDateTime ) OCIDescriptorFree( m_ociDateTime, OCI_DTYPE_TIMESTAMP );
}

bool
coral::OracleAccess::TimeOutputVariableHolder::copyData()
{
  sb2 year;
  ub1 month, day;
  sword status = OCIDateTimeGetDate( m_sessionProperties->ociEnvHandle(),
                                     m_sessionProperties->ociErrorHandle(),
                                     m_ociDateTime, &year, &month, &day );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the date from the time stamp" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    return false;
  }

  ub1 hour, min, sec;
  ub4 fsec;
  status = OCIDateTimeGetTime(  m_sessionProperties->ociEnvHandle(),
                                m_sessionProperties->ociErrorHandle(),
                                m_ociDateTime, &hour, &min, &sec, &fsec );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the time from the time stamp" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    return false;
  }

  m_time = coral::TimeStamp( year, month, day, hour, min, sec, fsec );
  return true;
}




coral::OracleAccess::NumberOutputVariableHolder::NumberOutputVariableHolder( void* userNumber,
                                                                             boost::shared_ptr<const SessionProperties> sessionProperties,
                                                                             bool isSigned ) :
  m_userNumber( userNumber ),
  m_sessionProperties( sessionProperties ),
  m_ociNumber( new OCINumber ),
  m_signFlag( (isSigned) ? OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED )
{
}

coral::OracleAccess::NumberOutputVariableHolder::~NumberOutputVariableHolder()
{
  delete m_ociNumber;
}


bool
coral::OracleAccess::NumberOutputVariableHolder::copyData()
{

  sword status = OCINumberToInt( m_sessionProperties->ociErrorHandle(),
                                 m_ociNumber,
                                 sizeof(long long),
                                 m_signFlag,
                                 m_userNumber );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving a 64-bit integer variable" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    return false;
  }
  return true;
}




coral::OracleAccess::BlobOutputVariableHolder::BlobOutputVariableHolder( coral::Blob& blob,
                                                                         boost::shared_ptr<const SessionProperties> sessionProperties ) :
  m_blob( blob ),
  m_sessionProperties( sessionProperties ),
  m_locator( 0 )
{
  void* temporaryPointer = 0;
  sword status = OCIDescriptorAlloc( m_sessionProperties->ociEnvHandle(),
                                     &temporaryPointer,
                                     OCI_DTYPE_LOB, 0, 0 );
  if ( status != OCI_SUCCESS ) {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << "Could not allocate a lob locator" << coral::MessageStream::endmsg;
  }
  else {
    m_locator = static_cast<OCILobLocator*>( temporaryPointer );
  }
}


coral::OracleAccess::BlobOutputVariableHolder::~BlobOutputVariableHolder()
{
  if (m_locator) OCIDescriptorFree(m_locator, OCI_DTYPE_LOB);
}


bool
coral::OracleAccess::BlobOutputVariableHolder::copyData()
{
  // Call OCILobOpen and OCILobClose?
  static const bool lobOpenClose = ( ::getenv( "CORAL_ORA_OCI_LOB_OPEN_CLOSE" ) != 0 );

  // Get the lob chunk size
  const int lobChunkSize = m_sessionProperties->domainProperties().lobChunkSize();

  // 1. OCILobOpen - open the BLOB
  if ( lobOpenClose )
  {
    sword status = OCILobOpen( m_sessionProperties->ociSvcCtxHandle(),
                               m_sessionProperties->ociErrorHandle(),
                               m_locator,
                               OCI_LOB_READONLY );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Opening a BLOB for reading." );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return false;
    }
  }

  // 2. OCILobRead - read from the BLOB
  {
    m_blob.resize( 0 );
    //char chunk[lobChunkSize];
    coral::Blob chunk( lobChunkSize ); // for windows and osx
    ub4 bytesRead = 0;
    sword status = OCILobRead( m_sessionProperties->ociSvcCtxHandle(),
                               m_sessionProperties->ociErrorHandle(),
                               m_locator,
                               &bytesRead,
                               1,
                               (dvoid*)chunk.startingAddress(),
                               lobChunkSize,
                               (dvoid*)&m_blob,
                               blobReadCallback,
                               0,
                               SQLCS_IMPLICIT );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Reading a BLOB." );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return false;
    }
  }

  // 3. OCILobClose - close the BLOB
  if ( lobOpenClose )
  {
    sword status = OCILobClose( m_sessionProperties->ociSvcCtxHandle(),
                                m_sessionProperties->ociErrorHandle(),
                                m_locator );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Closing a BLOB after reading." );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return false;
    }
  }

  // Success
  return true;
}

coral::OracleAccess::ClobOutputVariableHolder::ClobOutputVariableHolder( std::string& userData,
                                                                         boost::shared_ptr<const SessionProperties> sessionProperties ) :
  m_userData( userData ),
  m_sessionProperties( sessionProperties ),
  m_locator( 0 )
{
  void* temporaryPointer = 0;
  sword status = OCIDescriptorAlloc( m_sessionProperties->ociEnvHandle(),
                                     &temporaryPointer,
                                     OCI_DTYPE_LOB, 0, 0 );
  if ( status != OCI_SUCCESS ) {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << "Could not allocate a lob locator" << coral::MessageStream::endmsg;
  }
  else {
    m_locator = static_cast<OCILobLocator*>( temporaryPointer );
  }
}


coral::OracleAccess::ClobOutputVariableHolder::~ClobOutputVariableHolder()
{
  if (m_locator) OCIDescriptorFree(m_locator, OCI_DTYPE_LOB);
}


bool
coral::OracleAccess::ClobOutputVariableHolder::copyData()
{
  // Call OCILobOpen and OCILobClose?
  static const bool lobOpenClose = ( ::getenv( "CORAL_ORA_OCI_LOB_OPEN_CLOSE" ) != 0 );

  // Get the lob chunk size
  const int lobChunkSize = m_sessionProperties->domainProperties().lobChunkSize();

  // 1. OCILobOpen - open the CLOB
  if ( lobOpenClose )
  {
    sword status = OCILobOpen( m_sessionProperties->ociSvcCtxHandle(),
                               m_sessionProperties->ociErrorHandle(),
                               m_locator,
                               OCI_LOB_READONLY );
    if ( status != OCI_SUCCESS )
    {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Opening a CLOB for reading." );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return false;
    }
  }

  // 2. OCILobRead - read from the CLOB
  {
    m_userData = "";
    //char chunk[lobChunkSize];
    coral::Blob chunk( lobChunkSize ); // for windows and osx
    ub4 bytesRead = 0;
    sword status = OCILobRead( m_sessionProperties->ociSvcCtxHandle(),
                               m_sessionProperties->ociErrorHandle(),
                               m_locator,
                               &bytesRead,
                               1,
                               (dvoid*)chunk.startingAddress(),
                               lobChunkSize,
                               (dvoid*)&m_userData,
                               clobReadCallback,
                               0,
                               SQLCS_IMPLICIT );

    if ( status != OCI_SUCCESS )
    {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Reading a CLOB." );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return false;
    }
  }

  // 3. OCILobClose - close the CLOB
  if ( lobOpenClose )
  {
    sword status = OCILobClose( m_sessionProperties->ociSvcCtxHandle(),
                                m_sessionProperties->ociErrorHandle(),
                                m_locator );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Closing a CLOB after reading." );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return false;
    }
  }

  // Success
  return true;
}






coral::OracleAccess::StringOutputVariableHolder::StringOutputVariableHolder( std::string& userData,
                                                                             ub2 stringSize ) :
  m_userData( userData ),
  m_data( new char[stringSize] ),
  m_size( stringSize )
{
}


coral::OracleAccess::StringOutputVariableHolder::~StringOutputVariableHolder()
{
  if ( m_data ) delete [] m_data;
}


bool
coral::OracleAccess::StringOutputVariableHolder::copyData()
{
  m_userData = std::string( m_data );
  return true;
}



coral::OracleAccess::LongStringOutputVariableHolder::LongStringOutputVariableHolder( std::string& userData ) :
  m_userData( userData ),
  m_data( new char[65536] ),
  m_size( 65535 )
{
}


coral::OracleAccess::LongStringOutputVariableHolder::~LongStringOutputVariableHolder()
{
  if ( m_data ) delete [] m_data;
}


bool
coral::OracleAccess::LongStringOutputVariableHolder::copyData()
{
  m_data[ m_size ] = 0;
  m_userData = std::string( m_data );
  return true;
}
