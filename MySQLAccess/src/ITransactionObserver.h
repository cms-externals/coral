#ifndef MYSQLACCESS_TRANSACTION_OBSERVER_H
#define MYSQLACCESS_TRANSACTION_OBSERVER_H

namespace coral
{
  namespace MySQLAccess
  {
    /**
     * Class ITransactionObserver
     *
     * Callback interface to follow the beginning and the starting of a transaction
     */
    class ITransactionObserver
    {
    public:
      /// Virtual destructor
      virtual ~ITransactionObserver() {};
      /// Reacts on an End-Of-Transaction signal
      virtual void reactOnEndOfTransaction() = 0;
    };
  }
}

#endif
