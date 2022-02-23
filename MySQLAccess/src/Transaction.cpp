// $Id: Transaction.cpp,v 1.20 2011/03/22 10:29:55 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralBase/MessageStream.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/SimpleTimer.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "ITransactionObserver.h"
#include "SessionProperties.h"
#include "Transaction.h"

namespace coral
{
  namespace MySQLAccess
  {
    struct Committer
    {
      virtual ~Committer() {};
      virtual bool commit() = 0;
      virtual bool rollback() = 0;
    };

    struct Committer40 : virtual public Committer
    {
      Committer40( const ISessionProperties& p ) : m_props( p ) {}

      virtual ~Committer40() {};

      virtual bool commit()
      {
        boost::mutex::scoped_lock lock( m_props.lock() );
        // We have to do it for via SQL
        if( mysql_query( m_props.connectionHandle(), "COMMIT" ) )
          return false;

        return true;
      }

      virtual bool rollback()
      {
        boost::mutex::scoped_lock lock( m_props.lock() );
        // We have to do it for via SQL
        if( mysql_query( m_props.connectionHandle(), "ROLLBACK" ) )
          return false;

        return true;
      }

      const ISessionProperties& m_props;
    };

#if (  MYSQL_VERSION_ID > 40100 )
    struct Committer5x : virtual public Committer
    {
      Committer5x( const ISessionProperties& p ) : m_props( p ) {}

      virtual ~Committer5x() {};

      virtual bool commit()
      {
        boost::mutex::scoped_lock lock( m_props.lock() );
        if( mysql_commit( m_props.connectionHandle() ))
          return false;

        return true;
      }

      virtual bool rollback()
      {
        boost::mutex::scoped_lock lock( m_props.lock() );
        if( mysql_rollback( m_props.connectionHandle() ))
          return false;

        return true;
      }

      const ISessionProperties& m_props;
    };
#endif

    struct CommitterFactory
    {
      static Committer* getCommitter( const ISessionProperties& props )
      {
#if (  MYSQL_VERSION_ID > 40100 )
        return new Committer5x( props );
#else
        return new Committer40( props );
#endif
      }
    };
  }
}

coral::MySQLAccess::Transaction::Transaction( coral::MySQLAccess::SessionProperties& properties, ITransactionObserver& observer )
  : m_sessionProperties( properties ),
    m_observer( observer ),
    m_transactionActive( false ),
    m_isReadOnly( false )
{
}

coral::MySQLAccess::Transaction::~Transaction()
{
  if ( this->isActive() ) this->rollback();
}

void coral::MySQLAccess::Transaction::start( bool readOnly )
{
  if ( this->isActive() )
  {
    coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
    log << coral::Warning << "A transaction is already active" << coral::MessageStream::endmsg;
    return;
  }

  if ( ! readOnly && m_sessionProperties.isReadOnly() ) {
    throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(),
                                                          "ITransaction::start" );
  }

  if ( ! m_isReadOnly )
  {
    boost::mutex::scoped_lock lock( m_sessionProperties.lock() );
    if ( mysql_query( m_sessionProperties.connectionHandle(), "START TRANSACTION" ) != 0 )
    {
      coral::MySQLAccess::ErrorHandler errorHandler;
      errorHandler.handleCase( m_sessionProperties.connectionHandle(), "Starting a new transaction" );
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::TransactionNotStartedException( m_sessionProperties.domainProperties().service()->name() );
    }
  }

  m_transactionActive = true;
  m_isReadOnly = readOnly;

  if ( m_sessionProperties.monitoringService() )
  {
    m_sessionProperties.monitoringService()->record( m_sessionProperties.connectionString(), coral::monitor::Transaction, coral::monitor::Info, monitoringEventDescription.sessionBegin(),
                                                     ( m_isReadOnly ? monitoringEventDescription.transactionReadOnly() : monitoringEventDescription.transactionUpdate() ) );
  }
}

void coral::MySQLAccess::Transaction::commit()
{
  if ( ! ( this->isActive() ) )
  {
    coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
    log << coral::Warning << "No active transaction to commit" << coral::MessageStream::endmsg;
    return;
  }

  coral::SimpleTimer timer;

  if ( m_sessionProperties.monitoringService() )
    timer.start();

  if ( ! m_isReadOnly )
  {
    std::unique_ptr<coral::MySQLAccess::Committer>
      committer( coral::MySQLAccess::CommitterFactory::getCommitter( m_sessionProperties ) );

    if ( ! committer->commit() )
    {
      coral::MySQLAccess::ErrorHandler errorHandler;
      errorHandler.handleCase( m_sessionProperties.connectionHandle(), "Committing a transaction" );
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      throw coral::TransactionNotCommittedException( m_sessionProperties.domainProperties().service()->name() );
    }
  }


  m_transactionActive = false;


  if ( m_sessionProperties.monitoringService() )
  {
    timer.stop();
    double idleTimeSeconds = 0.0;
    idleTimeSeconds = timer.total() * 1e-6;
    m_sessionProperties.monitoringService()->record( m_sessionProperties.connectionString()
                                                     , coral::monitor::Transaction, coral::monitor::Time
                                                     , monitoringEventDescription.transactionCommit(), idleTimeSeconds );
  }

  m_observer.reactOnEndOfTransaction();
}

void coral::MySQLAccess::Transaction::rollback()
{
  if ( ! ( this->isActive() ) )
  {
    coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
    log << coral::Warning << "No active transaction to roll back" << coral::MessageStream::endmsg;
    return;
  }

  coral::SimpleTimer timer;

  if ( m_sessionProperties.monitoringService() )
    timer.start();

  if ( ! m_isReadOnly )
  {
    std::unique_ptr<coral::MySQLAccess::Committer>
      committer( coral::MySQLAccess::CommitterFactory::getCommitter( m_sessionProperties ) );

    if ( ! committer->rollback() )
    {
      coral::MySQLAccess::ErrorHandler errorHandler;
      errorHandler.handleCase( m_sessionProperties.connectionHandle(), "Rolling back a transaction" );
      coral::MessageStream log( m_sessionProperties.domainProperties().service()->name() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    }
  }

  m_transactionActive = false;



  if ( m_sessionProperties.monitoringService() )
  {
    timer.stop();
    double idleTimeSeconds = 0.0;
    idleTimeSeconds = timer.total() * 1e-6;
    m_sessionProperties.monitoringService()->record( m_sessionProperties.connectionString()
                                                     , coral::monitor::Transaction, coral::monitor::Time
                                                     , monitoringEventDescription.transactionRollback(), idleTimeSeconds );
  }

  m_observer.reactOnEndOfTransaction();
}

bool coral::MySQLAccess::Transaction::isActive() const
{
  return m_transactionActive;
}

bool coral::MySQLAccess::Transaction::isReadOnly() const
{
  return m_isReadOnly;
}
