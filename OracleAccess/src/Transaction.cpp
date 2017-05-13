#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include <iostream>

#include "oci.h"

#include "CoralBase/MessageStream.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/SimpleTimer.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "ITransactionObserver.h"
#include "OracleErrorHandler.h"
#include "SessionProperties.h"
#include "Transaction.h"


coral::OracleAccess::Transaction::Transaction( boost::shared_ptr<const SessionProperties> properties,
                                               ITransactionObserver& observer ) :
  m_sessionProperties( properties ),
  m_observer( observer ),
  m_isSerializableIfRO( true ), // actual value is set in startUserSession
  m_ociTransHandle( 0 ),
  m_isReadOnly( false ),
  m_mutex(),
  m_invalidSession( false ),
  m_userSessionStarted( false ) // startUserSession must be called explicitly
{
  //std::cout << "Create Transaction " << this << std::endl; //debug bug #80098
}


coral::OracleAccess::Transaction::~Transaction()
{
  //std::cout << "Delete Transaction " << this << std::endl; //debug bug #80098
  if ( !m_userSessionStarted ) return; // Transaction was already 'deleted'
  if ( ! m_invalidSession ) // this flag is only used here - fix bug #80092
  {
    if ( this->isActive() ) this->rollback();
  }
}


void
coral::OracleAccess::Transaction::start( bool readOnly )
{
  if ( !m_userSessionStarted ) // This transaction was "deleted" (bug #80174)
    throw SessionException( "User session has already ended",
                            "Transaction::start",
                            m_sessionProperties->domainServiceName() );
  if ( this->isActive() )
  {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Warning
        << "A transaction is already active" << coral::MessageStream::endmsg;
    return;
  }

  boost::mutex::scoped_lock lock(m_mutex);
  if ( ! readOnly && m_sessionProperties->isReadOnly() )
  {
    throw InvalidOperationInReadOnlyModeException
      ( m_sessionProperties->domainServiceName(), "ITransaction::start" );
  }
  this->allocateOCIHandles();
  if ( !readOnly || m_isSerializableIfRO ) // SKIP READONLY TRANSACTIONS
  {
    sword status =
      OCITransStart( m_sessionProperties->ociSvcCtxHandle(),
                     m_sessionProperties->ociErrorHandle(),
                     0,
                     OCI_TRANS_NEW | ( readOnly ? OCI_TRANS_READONLY : 0 ) );
    if ( status != OCI_SUCCESS )
    {
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "start transaction" );
      if ( errorHandler.isError() )
      {
        log << coral::Error
            << errorHandler.message() << coral::MessageStream::endmsg;
        this->releaseOCIHandles();
        throw TransactionNotStartedException
          ( m_sessionProperties->domainServiceName() );
      }
      log << coral::Warning
          << errorHandler.message() << coral::MessageStream::endmsg;
    }
  }
  m_isReadOnly = readOnly;
  if ( m_sessionProperties->monitoringService() )
  {
    m_sessionProperties->monitoringService()->record
      ( "oracle://" + m_sessionProperties->connectionString() +
        "/" + m_sessionProperties->schemaName(),
        coral::monitor::Transaction,
        coral::monitor::Info,
        monitoringEventDescription.sessionBegin(),
        ( m_isReadOnly ?
          monitoringEventDescription.transactionReadOnly() :
          monitoringEventDescription.transactionUpdate() ) );
  }
}


void
coral::OracleAccess::Transaction::commit()
{
  if ( !m_userSessionStarted ) // This transaction was "deleted" (bug #80174)
    throw SessionException( "User session has already ended",
                            "Transaction::commit",
                            m_sessionProperties->domainServiceName() );
  coral::MessageStream log( m_sessionProperties->domainServiceName() );
  if ( ! ( this->isActive() ) )
  {
    log << coral::Warning
        << "No active transaction to commit" << coral::MessageStream::endmsg;
    return;
  }

  boost::mutex::scoped_lock lock(m_mutex);
  coral::SimpleTimer timer;
  if ( m_sessionProperties->monitoringService() ) timer.start();
  sword status = OCI_SUCCESS;
  if ( !m_isReadOnly || m_isSerializableIfRO ) // SKIP READONLY TRANSACTIONS
    status = OCITransCommit( m_sessionProperties->ociSvcCtxHandle(),
                             m_sessionProperties->ociErrorHandle(),
                             OCI_DEFAULT );

  if ( m_sessionProperties->monitoringService() )
  {
    timer.stop();
    double idleTimeSeconds = timer.total() * 1e-6;
    m_sessionProperties->monitoringService()->record
      ( "oracle://" + m_sessionProperties->connectionString() +
        "/" + m_sessionProperties->schemaName(),
        coral::monitor::Transaction,
        coral::monitor::Time,
        monitoringEventDescription.transactionCommit(),
        idleTimeSeconds );
  }

  m_observer.reactOnEndOfTransaction();
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "commit transaction" );
    log << coral::Error
        << errorHandler.message() << coral::MessageStream::endmsg;
    this->releaseOCIHandles();
    throw TransactionNotCommittedException
      ( m_sessionProperties->domainServiceName() );
  }
  this->releaseOCIHandles();
}


void
coral::OracleAccess::Transaction::rollback()
{
  if ( !m_userSessionStarted ) // This transaction was "deleted" (bug #80174)
    throw SessionException( "User session has already ended",
                            "Transaction::rollback",
                            m_sessionProperties->domainServiceName() );
  coral::MessageStream log( m_sessionProperties->domainServiceName() );
  if ( ! ( this->isActive() ) )
  {
    log << coral::Warning
        << "No active transaction to roll back"
        << coral::MessageStream::endmsg;
    return;
  }

  boost::mutex::scoped_lock lock(m_mutex);
  sword status = OCI_SUCCESS;
  if ( !m_isReadOnly || m_isSerializableIfRO ) // SKIP READONLY TRANSACTIONS
    status = OCITransRollback( m_sessionProperties->ociSvcCtxHandle(),
                               m_sessionProperties->ociErrorHandle(),
                               OCI_DEFAULT );
  if ( m_sessionProperties->monitoringService() )
  {
    m_sessionProperties->monitoringService()->record
      ( "oracle://" + m_sessionProperties->connectionString() +
        "/" + m_sessionProperties->schemaName(),
        coral::monitor::Transaction,
        coral::monitor::Info,
        monitoringEventDescription.transactionRollback() );
  }
  m_observer.reactOnEndOfTransaction();
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "roll back transaction" );
    log << coral::Error
        << errorHandler.message() << coral::MessageStream::endmsg;
    // Added maybe temporarily to study the issue reported in bug #87164
    // (CMS observed an error involving ORA-25408 during a roll back)
    if ( errorHandler.lastErrorCode() == 25408 )
      throw coral::TransactionException( m_sessionProperties->domainServiceName(),
                                         "Transaction::rollback",
                                         errorHandler.message()  );
  }
  this->releaseOCIHandles();
}


void
coral::OracleAccess::Transaction::allocateOCIHandles()
{
  if ( !m_userSessionStarted ) // This transaction was "deleted" (bug #80174)
    throw SessionException( "User session has already ended",
                            "Transaction::allocateOCIHandles",
                            m_sessionProperties->domainServiceName() );
  coral::MessageStream log( m_sessionProperties->domainServiceName() );

  // Allocate a new transaction handle
  void* temporaryPointer = 0;
  sword status = OCIHandleAlloc( m_sessionProperties->ociEnvHandle(),
                                 &temporaryPointer,
                                 OCI_HTYPE_TRANS, 0, 0 );
  if ( status != OCI_SUCCESS )
  {
    throw TransactionNotStartedException
      ( m_sessionProperties->domainServiceName(),
        "Failed to allocate a new transaction handle" );
  }
  m_ociTransHandle = static_cast< OCITrans* >( temporaryPointer );
  //std::cout << "OCITrans created " << this << std::endl; // bug #83601

  // Set the transaction attribute in the service context
  status = OCIAttrSet( m_sessionProperties->ociSvcCtxHandle(),
                       OCI_HTYPE_SVCCTX, m_ociTransHandle, 0,
                       OCI_ATTR_TRANS, m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler(  m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase
      ( status, "Setting the transaction attribute in the service context" );
    if ( errorHandler.isError() )
    {
      log << coral::Error
          << errorHandler.message() << coral::MessageStream::endmsg;
      OCIHandleFree( m_ociTransHandle, OCI_HTYPE_TRANS );
      m_ociTransHandle = 0;
      throw TransactionNotStartedException
        ( m_sessionProperties->domainServiceName(),
          "Failed to set the transaction attribute in the service context" );
    }
    else
    {
      log << coral::Warning
          << errorHandler.message() << coral::MessageStream::endmsg;
    }
  }
  //std::cout << "OCITrans set in ctx " << this << std::endl; // bug #83601
}


void
coral::OracleAccess::Transaction::releaseOCIHandles()
{
  // This transaction was "deleted" (bug #80174)
  if ( !m_userSessionStarted )
    throw SessionException( "User session has already ended",
                            "Transaction::releaseOCIHandles",
                            m_sessionProperties->domainServiceName() );
  // Clear the transaction attribute in the service context (fix bug #61090)
  sword status = OCIAttrSet( m_sessionProperties->ociSvcCtxHandle(),
                             OCI_HTYPE_SVCCTX, 0, 0, OCI_ATTR_TRANS,
                             m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase
      ( status, "Clearing the transaction attribute in the service context" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    if ( errorHandler.isError() )
      log << coral::Error
          << errorHandler.message() << coral::MessageStream::endmsg;
    else
      log << coral::Warning
          << errorHandler.message() << coral::MessageStream::endmsg;
  }
  //std::cout << "OCITrans unset in ctx " << this << std::endl; // bug #83601
  // Release the transaction handle
  status = OCIHandleFree( m_ociTransHandle, OCI_HTYPE_TRANS );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler(  m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Releasing the transaction handle" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    if ( errorHandler.isError() )
      log << coral::Error
          << errorHandler.message() << coral::MessageStream::endmsg;
    else
      log << coral::Warning
          << errorHandler.message() << coral::MessageStream::endmsg;
  }
  m_ociTransHandle = 0;
  //std::cout << "OCITrans deleted " << this << std::endl; // bug #83601
}


bool
coral::OracleAccess::Transaction::isActive() const
{
  if ( !m_userSessionStarted ) // This transaction was "deleted" (bug #80174)
    throw SessionException( "User session has already ended",
                            "Transaction::isActive",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock(m_mutex);
  return ( m_ociTransHandle != 0 );
}


bool
coral::OracleAccess::Transaction::isReadOnly() const
{
  if ( !m_userSessionStarted ) // This transaction was "deleted" (bug #80174)
    throw SessionException( "User session has already ended",
                            "Transaction::isReadOnly",
                            m_sessionProperties->domainServiceName() );
  boost::mutex::scoped_lock lock(m_mutex);
  return m_isReadOnly;
}


void
coral::OracleAccess::Transaction::invalidateSession()
{
  m_invalidSession = true; // do not check m_userSessionStarted here
}


void
coral::OracleAccess::Transaction::startUserSession( bool isSerializableIfRO )
{
  // Sanity check: 'creating' an already 'created' transaction?
  if ( m_userSessionStarted )
    throw SessionException( "PANIC! User session has already started",
                            "Transaction::startUserSession",
                            m_sessionProperties->domainServiceName() );
  m_isSerializableIfRO = isSerializableIfRO;
  m_userSessionStarted = true;
}


void
coral::OracleAccess::Transaction::endUserSession()
{
  // Sanity check: 'deleting' an already 'deleted' transaction?
  if ( !m_userSessionStarted )
    throw SessionException( "PANIC! User session has already ended",
                            "Transaction::endUserSession",
                            m_sessionProperties->domainServiceName() );
  // Rollback the transaction (fully reset its state) if 'deleting' it
  if ( !m_invalidSession ) // this flag is only used here - fix bug #80092
  {
    if ( this->isActive() ) this->rollback();
  }
  m_userSessionStarted = false;
}
