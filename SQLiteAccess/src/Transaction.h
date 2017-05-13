#ifndef SQLITEACCESS_TRANSACTION_H
#define SQLITEACCESS_TRANSACTION_H

#include "RelationalAccess/ITransaction.h"
#include <boost/shared_ptr.hpp>

namespace coral {

  namespace SQLiteAccess {

    class SessionProperties;
    /**
       @class Transaction Transaction.h

       Implementation of the Transaction interface for the SQLiteAccess module
       @author Zhen Xie
    */

    class Transaction : virtual public ITransaction {

    public:

      /// Constructor
      explicit Transaction( boost::shared_ptr<const SessionProperties> properties );

      /// Destructor
      virtual ~Transaction();

      /// Sets the transactional context
      //void setContext( sqlite3* dbHandle );
      /**
       * Starts a new transactional context.
       * In case of failure a TransactionNotStartedException is thrown.
       * @param readOnly Boolean flag which if set to true, no updates in the existing data or schema can be made.
       *
       */
      void start( bool readOnly = false );

      /**
       * Returns true if a read-only transaction has been started, false otherwise.
       */
      bool isReadOnly() const;

      /**
       * Returns true if a transaction is active, false otherwise.
       */
      bool isActive() const;

      /**
       * Commits a transaction.
       * In case of failure a TransactionNotCommittedException is thrown.
       */
      void commit();

      /**
       * Rolls back (aborts) a transaction.
       */
      void rollback();

    private:
      /// The session properties
      boost::shared_ptr<const SessionProperties> m_properties;

      /// Flag indicating whether a transaction is read-only
      bool m_readOnly;
      /// Flag indicating whether a transaction is active
      bool m_isActive;
    };

  }

}

#endif
