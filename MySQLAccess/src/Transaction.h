#ifndef MYSQLACCESS_TRANSACTION_H
#define MYSQLACCESS_TRANSACTION_H

#include "RelationalAccess/ITransaction.h"

namespace coral {

  namespace MySQLAccess {

    class SessionProperties;
    class ITransactionObserver;

    /**
     * Class Transaction
     * Implementation of the ITransaction interface for the MySQLAccess plugin
     */
    class Transaction : virtual public coral::ITransaction
    {
    public:
      /// Constructor
      Transaction( SessionProperties& properties,
                   ITransactionObserver& observer );

      /// Destructor
      virtual ~Transaction();

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
      /// The session properties
      SessionProperties&    m_sessionProperties;

      /// The transaction observer
      ITransactionObserver& m_observer;

      /// Flag indicating whether a transaction has been started
      bool m_transactionActive;

      /// Flag indicating whether a transaction is read-only
      bool m_isReadOnly;
    };

  }

}

#endif
