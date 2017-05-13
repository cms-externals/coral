#ifndef READTHREAD_H
#define READTHREAD_H

#include "TestEnv.h"

namespace coral {
  class ISchema;
}

class Reporter;
class GlobalStatus;

class ReadThread {
public:
  ReadThread( coral::ISchema& schema,
              Reporter& reporter,
              int id,
              int tableId,
              GlobalStatus& ok,
              TestEnv * env ) :
    m_schema( schema ),
    m_reporter( reporter ),
    m_id( id ),
    m_tableId( tableId ),
    m_ok( ok )
  { m_env = env; }

  ~ReadThread() {}

  void operator()();

private:
  coral::ISchema& m_schema;
  Reporter& m_reporter;
  int m_id;
  int m_tableId;
  GlobalStatus& m_ok;
  TestEnv * m_env;
};

#endif
