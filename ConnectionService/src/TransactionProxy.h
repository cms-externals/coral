#ifndef CONNECTIONSERVICE_TRANSACTIONPROXY_H
#define CONNECTIONSERVICE_TRANSACTIONPROXY_H

#include <string>
#include "CoralBase/boost_thread_headers.h"
#include "RelationalAccess/ITransaction.h"

namespace coral
{

  namespace ConnectionService
  {

    class SessionHandle;

    /**
     * Class TransactionProxy.
     * Proxy implementation of the ITransaction interface.
     */
    class TransactionProxy : virtual public coral::ITransaction
    {

    public:

      /// constructor
      TransactionProxy( const SessionHandle& sessionHandle );

      /// destructor
      virtual ~TransactionProxy();

      /// was the transaction active?
      bool wasActive()
      {
        return m_wasActive;
      }

      /// was the transaction readOnly?
      bool wasReadOnly()
      {
        return m_wasReadOnly;
      }

      /**
       * Starts a new transaction.
       * In case of failure a TransactionNotStartedException is thrown.
       *
       */
      void start( bool readOnly = false );

      /**
       * Commits the transaction.
       * In case of failure a TransactionNotCommittedException is thrown.
       */
      void commit();

      /**
       * Aborts and rolls back a transaction.
       */
      void rollback();

      /**
       * Returns the status of the transaction (if it is active or not)
       */
      bool isActive() const;

      /**
       * Returns the mode of the transaction (if it is read-only or not)
       */
      bool isReadOnly() const;

    private:

      const SessionHandle& m_session;

      boost::mutex m_mutex;

      bool m_wasActive; // if a transaction was broken, was it active?

      bool m_wasReadOnly; // if a transaction was broken, was it RO?

    };

  }

}
#endif
