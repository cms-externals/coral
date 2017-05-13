#ifndef ORACLEACCESS_TRANSACTION_OBSERVER_H
#define ORACLEACCESS_TRANSACTION_OBSERVER_H

namespace coral {

  namespace OracleAccess {

    /**
     * Class ITransactionObserver
     *
     * Callback interface to follow the beginning and the starting of a transaction
     */

    class ITransactionObserver {
    public:
      virtual ~ITransactionObserver(){}
      /// Reacts on an End-Of-Transaction signal
      virtual void reactOnEndOfTransaction() = 0;
    };

  }

}

#endif
