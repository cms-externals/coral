#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralKernel/Service.h"

#include "BindVariableHolder.h"
#include "DomainProperties.h"
#include "OracleErrorHandler.h"
#include "SessionProperties.h"


coral::OracleAccess::NumberVariableHolder::NumberVariableHolder( void* numberData,
                                                                 bool isSigned,
                                                                 boost::shared_ptr<const SessionProperties> sessionProperties,
                                                                 ub2 numberSize ) :
  m_number( new OCINumber )
{
  uword flag = ( isSigned ) ? OCI_NUMBER_SIGNED : OCI_NUMBER_UNSIGNED;
  sword status = OCINumberFromInt( sessionProperties->ociErrorHandle(),
                                   numberData,
                                   numberSize,
                                   flag,
                                   m_number );
  if ( status != OCI_SUCCESS )
  {
    coral::OracleAccess::OracleErrorHandler errorHandler( sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Converting an integer into a number" );
    coral::MessageStream log( sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    delete m_number;
    m_number = 0;
  }
}


coral::OracleAccess::NumberVariableHolder::~NumberVariableHolder()
{
  if ( m_number ) delete m_number;
}


coral::OracleAccess::DateVariableHolder::DateVariableHolder( const coral::Date& inputDate ) :
  m_date( new OCIDate )
{
  OCIDateSetDate( m_date, static_cast<sb2>( inputDate.year() ),
                  static_cast<ub1>( inputDate.month() ),
                  static_cast<ub1>( inputDate.day() ) );
}


coral::OracleAccess::DateVariableHolder::~DateVariableHolder()
{
  if ( m_date ) delete m_date;
}


coral::OracleAccess::TimeVariableHolder::TimeVariableHolder( const coral::TimeStamp& inputTime,
                                                             boost::shared_ptr<const SessionProperties> sessionProperties ) :
  m_time( 0 )
{
  // Construct a time stamp descriptor
  void* temporaryPointer = 0;
  sword status = OCIDescriptorAlloc( sessionProperties->ociEnvHandle(),
                                     &temporaryPointer, OCI_DTYPE_TIMESTAMP, 0, 0 );
  if ( status != OCI_SUCCESS )
  {
    coral::MessageStream log( sessionProperties->domainServiceName() );
    log << coral::Error << "Could not allocate a TIMESTAMP descriptor" << coral::MessageStream::endmsg;
  }
  else
  {
    m_time = static_cast< OCIDateTime* >( temporaryPointer );

    // Assign the time
    status = OCIDateTimeConstruct( sessionProperties->ociEnvHandle(),
                                   sessionProperties->ociErrorHandle(),
                                   m_time,
                                   static_cast<sb2>( inputTime.year() ),
                                   static_cast<ub1>( inputTime.month() ),
                                   static_cast<ub1>( inputTime.day() ),
                                   static_cast<ub1>( inputTime.hour() ),
                                   static_cast<ub1>( inputTime.minute() ),
                                   static_cast<ub1>( inputTime.second() ),
                                   static_cast<ub4>( inputTime.nanosecond() ),
                                   0, 0 );
    if ( status != OCI_SUCCESS )
    {
      coral::OracleAccess::OracleErrorHandler errorHandler( sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Assigning values into a timestamp" );
      coral::MessageStream log( sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      OCIDescriptorFree( m_time, OCI_DTYPE_TIMESTAMP );
      m_time = 0;
    }
  }
}


coral::OracleAccess::TimeVariableHolder::~TimeVariableHolder()
{
  if ( m_time ) OCIDescriptorFree( m_time, OCI_DTYPE_TIMESTAMP );
}


coral::OracleAccess::BlobVariableHolder::BlobVariableHolder( const coral::Blob& inputData,
                                                             boost::shared_ptr<const SessionProperties> sessionProperties ) :
  m_data( 0 ),
  m_sessionProperties( sessionProperties )
{
  // Allocate the LOB locator
  void* temporaryPointer = 0;
  sword status = OCIDescriptorAlloc( sessionProperties->ociEnvHandle(),
                                     &temporaryPointer, OCI_DTYPE_LOB, 0, 0 );
  if ( status != OCI_SUCCESS )
  {
    coral::MessageStream log( sessionProperties->domainServiceName() );
    log << coral::Error << "Could not allocate a TIMESTAMP descriptor" << coral::MessageStream::endmsg;
  }
  else
  {
    m_data = static_cast< OCILobLocator* >( temporaryPointer );

    // Create a temporary LOB
    status = OCILobCreateTemporary( sessionProperties->ociSvcCtxHandle(),
                                    sessionProperties->ociErrorHandle(),
                                    m_data,
                                    OCI_DEFAULT, OCI_DEFAULT, OCI_TEMP_BLOB,
                                    FALSE,
                                    OCI_DEFAULT );
    if ( status != OCI_SUCCESS )
    {
      coral::OracleAccess::OracleErrorHandler errorHandler( sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Creating a temporary LOB" );
      coral::MessageStream log( sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      OCIDescriptorFree( m_data, OCI_DTYPE_LOB );
      m_data = 0;
    }
    else
    {
      // Write the data into the LOB
      ub4 amtp = inputData.size();
      status = OCILobWriteAppend( sessionProperties->ociSvcCtxHandle(),
                                  sessionProperties->ociErrorHandle(),
                                  m_data, &amtp,
                                  const_cast<void*>( inputData.startingAddress() ),
                                  inputData.size(),
                                  OCI_ONE_PIECE, 0, 0, 0, 0 );
      if ( status != OCI_SUCCESS )
      {
        coral::OracleAccess::OracleErrorHandler errorHandler( sessionProperties->ociErrorHandle() );
        errorHandler.handleCase( status, "Filling with input data a LOB Locator" );
        coral::MessageStream log( sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;

        OCILobFreeTemporary( sessionProperties->ociSvcCtxHandle(),
                             sessionProperties->ociErrorHandle(),
                             m_data );
        OCIDescriptorFree( m_data, OCI_DTYPE_LOB );
        m_data = 0;
      }
    }
  }
}


coral::OracleAccess::BlobVariableHolder::~BlobVariableHolder()
{
  if ( m_data )
  {
    OCILobFreeTemporary( m_sessionProperties->ociSvcCtxHandle(),
                         m_sessionProperties->ociErrorHandle(),
                         m_data );
    OCIDescriptorFree( m_data, OCI_DTYPE_LOB );
  }
}
