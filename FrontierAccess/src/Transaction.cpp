#include "Transaction.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "ITransactionObserver.h"

#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SessionException.h"

#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/SimpleTimer.h"

#include "CoralBase/MessageStream.h"

#include "CoralKernel/Service.h"

coral::FrontierAccess::Transaction::Transaction( coral::FrontierAccess::SessionProperties& properties, ITransactionObserver& observer )
  : m_sessionProperties( properties ), m_observer( observer ), m_isReadOnly( false ), m_isActive( false )
{
}

coral::FrontierAccess::Transaction::~Transaction()
{
  if ( this->isActive() )
    this->rollback();
}

void coral::FrontierAccess::Transaction::start( bool readOnly )
{
  coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

  if ( this->isActive() )
  {
    log << coral::Warning << "A transaction is already active" << coral::MessageStream::endmsg;
    return;
  }
  // Frontier sessions are always read only (fix bug #76072)
  if ( !readOnly )
  {
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(),
                                                          "ITransaction::start" );
  }

  m_isReadOnly = readOnly;

  if ( m_sessionProperties.monitoringService() )
  {
    m_sessionProperties.monitoringService()->record( m_sessionProperties.connectionString(), coral::monitor::Transaction, coral::monitor::Info,
                                                     monitoringEventDescription.sessionBegin(), ( m_isReadOnly ? monitoringEventDescription.transactionReadOnly() : monitoringEventDescription.transactionUpdate() ) );
  }

  this->m_isActive = true;
}

void coral::FrontierAccess::Transaction::commit()
{
  coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

  if ( ! ( this->isActive() ) )
  {
    log << coral::Warning << "No active transaction to commit" << coral::MessageStream::endmsg;
    return;
  }

  coral::SimpleTimer timer;
  timer.start();

  timer.sample();

  if ( m_sessionProperties.monitoringService() )
  {
    timer.stop();
    m_sessionProperties.monitoringService()->record( m_sessionProperties.connectionString(),
                                                     coral::monitor::Transaction, coral::monitor::Time,
                                                     monitoringEventDescription.transactionCommit(),
                                                     timer.total() );
  }

  m_observer.reactOnEndOfTransaction();

  this->m_isActive = false;
}

void coral::FrontierAccess::Transaction::rollback()
{
  coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );

  if ( ! ( this->isActive() ) )
  {
    log << coral::Warning << "No active transaction to roll back" << coral::MessageStream::endmsg;
    return;
  }

  if ( m_sessionProperties.monitoringService() )
  {
    m_sessionProperties.monitoringService()->record( m_sessionProperties.connectionString(), coral::monitor::Transaction, coral::monitor::Info, monitoringEventDescription.transactionRollback() );
  }

  m_observer.reactOnEndOfTransaction();

  this->m_isActive = false;
}

void coral::FrontierAccess::Transaction::allocateHandles() { ; }

void coral::FrontierAccess::Transaction::releaseHandles() { ; }

bool coral::FrontierAccess::Transaction::isActive() const { return m_isActive; }

bool coral::FrontierAccess::Transaction::isReadOnly() const { return m_isReadOnly; }
