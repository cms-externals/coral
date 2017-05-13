#ifndef FRONTIER_ACCESS_TRANSACTION_OBSERVER_H
#define FRONTIER_ACCESS_TRANSACTION_OBSERVER_H

namespace coral {

  namespace FrontierAccess {

    /**
     * Class ITransactionObserver
     *
     * Callback interface to follow the beginning and the starting of a transaction
     */

    class ITransactionObserver {
    public:
      virtual ~ITransactionObserver() {};

      /// Reacts on an End-Of-Transaction signal
      virtual void reactOnEndOfTransaction() = 0;
    };

  }

}

#endif
