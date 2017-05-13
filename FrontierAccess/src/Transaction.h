// -*- C++ -*-
// $Id: Transaction.h,v 1.3 2011/03/22 10:36:50 avalassi Exp $
#ifndef FRONTIER_ACCESS_TRANSACTION_H
#define FRONTIER_ACCESS_TRANSACTION_H 1

#include "RelationalAccess/ITransaction.h"

namespace coral
{
  namespace FrontierAccess
  {
    class SessionProperties;
    class ITransactionObserver;

    /**
     * Class Transaction
     * Implementation of the ITransaction interface for the FrontierAccess plugin
     */
    class Transaction : virtual public coral::ITransaction
    {
    public:
      /// Constructor
      Transaction( SessionProperties& properties, ITransactionObserver& observer );

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
      /// Allocates the necessary handles
      void allocateHandles();

      /// Releases the handles
      void releaseHandles();

    private:
      /// The session properties
      SessionProperties& m_sessionProperties;

      /// The transaction observer
      ITransactionObserver& m_observer;

      /// Flag indicating whether a transaction is read-only
      bool m_isReadOnly;

      /// Activity flag
      bool m_isActive;
    };
  }
}

#endif
