#include <iostream>
#include "Transaction.h"
#include "SessionProperties.h"
#include "SQLiteStatement.h"
#include "DomainProperties.h"

#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"

#include "CoralBase/MessageStream.h"

#include "CoralKernel/Service.h"

#include "sqlite3.h"

coral::SQLiteAccess::Transaction::Transaction( boost::shared_ptr<const SessionProperties> properties ) :
  m_properties( properties ),
  m_readOnly( true ),
  m_isActive( false )
{

}

coral::SQLiteAccess::Transaction::~Transaction()
{

  if ( this->isActive() ) this->rollback();
}

void
coral::SQLiteAccess::Transaction::start( bool readOnly )
{

  if ( this->isActive() ) {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<< coral::Warning << "A transaction is already active" << coral::MessageStream::endmsg;
    return;
  }
  if ( ! readOnly && m_properties->isReadOnly() ) {
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(),
                                                          "ITransaction::start" );
  }
  SQLiteStatement stmt(m_properties);
  if( !stmt.prepare("BEGIN") || !stmt.execute() ) {
    throw coral::TransactionNotStartedException( m_properties->domainProperties().service()->name() );
  }
  m_readOnly = readOnly;
  m_isActive =true;
}


bool
coral::SQLiteAccess::Transaction::isReadOnly() const
{

  return m_readOnly;
}

bool
coral::SQLiteAccess::Transaction::isActive() const
{

  return m_isActive;
}


void
coral::SQLiteAccess::Transaction::commit()
{

  if ( ! ( this->isActive() ) ) {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<< coral::Warning << "No active transaction to commit" << coral::MessageStream::endmsg;
    return ;
  }
  SQLiteStatement stmt(m_properties);
  if( !stmt.prepare("COMMIT")|| !stmt.execute() ) {
    throw coral::TransactionNotCommittedException( m_properties->domainProperties().service()->name() );
  }
  m_isActive=false;
}

void
coral::SQLiteAccess::Transaction::rollback()
{

  if ( ! ( this->isActive() ) ) {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<< coral::Warning << "No active transaction to roll back" << coral::MessageStream::endmsg;
    return;
  }
  SQLiteStatement stmt(m_properties);
  stmt.prepare("ROLLBACK");
  stmt.execute();
  m_isActive=false;
  return;
}
