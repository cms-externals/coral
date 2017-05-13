#ifndef CONNECTIONSERVICE_CONNECTIONPOOLTIMER_H
#define CONNECTIONSERVICE_CONNECTIONPOOLTIMER_H

namespace coral {

  namespace ConnectionService {

    class ConnectionPool;

    /// functor acting aa connection pool cleaner - lauched as a parallel thread
    class ConnectionPoolTimer {

    public:

      /// default constructor
      ConnectionPoolTimer();

      /// main constructor
      explicit ConnectionPoolTimer( ConnectionPool& pool );

      /// destructor
      virtual ~ConnectionPoolTimer();

      // functor procedure
      void operator()();

    private:

      // the connection pool to clean up
      ConnectionPool* m_pool;

    };

  }

}

#endif
