#ifndef TOPTHREAD_H
#define TOPTHREAD_H

#include "TestEnv.h"

namespace coral {
  class IConnectionService;
}

class Reporter;
class GlobalStatus;

class TopThread {
public:
  TopThread( coral::IConnectionService& connectionService,
             Reporter& reporter,
             int id,
             GlobalStatus& ok,
             TestEnv * env ) :
    m_connectionService( connectionService ),
    m_reporter( reporter ),
    m_id( id ),
    m_ok( ok )
  { m_env = env; }

  ~TopThread() {}

  void operator()();

private:
  coral::IConnectionService& m_connectionService;
  Reporter& m_reporter;
  int m_id;
  GlobalStatus& m_ok;
  TestEnv * m_env;
};

#endif
