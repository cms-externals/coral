#ifndef TEST_MTHREAD_SINGLETHREADREAD_H
#define TEST_MTHREAD_SINGLETHREADREAD_H 1

#include "TestEnv/Testing.h"

class SingleThreadRead : public Testing
{

public:

  SingleThreadRead( const TestEnv& env, size_t no );

  virtual ~SingleThreadRead();

  void operator()();

private:

  size_t m_tableno;

};

#endif
