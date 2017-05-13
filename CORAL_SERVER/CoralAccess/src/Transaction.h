#ifndef CORALACCESS_TRANSACTION_H
#define CORALACCESS_TRANSACTION_H 1

// Include files
#include "RelationalAccess/ITransaction.h"
#include "CoralServerBase/ICoralFacade.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class SessionProperties;

    /** @class Transaction
     *
     *  @author Andrea Valassi
     *  @date   2007-12-05
     */

    class Transaction : virtual public coral::ITransaction
    {

    public:

      /// Constructor
      Transaction( const SessionProperties& sessionProperties );

      /// Destructor
      virtual ~Transaction();

      /*
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

      /// Returns the facade for the CORAL server connection.
      const coral::ICoralFacade& facade() const;

    private:

      /// The properties of this remote database session.
      const SessionProperties& m_sessionProperties;

      /// Is the transaction active?
      bool m_isActive;

      /// Is the transaction readOnly?
      bool m_isReadOnly;

    };

  }

}
#endif // CORALACCESS_TRANSACTION_H
