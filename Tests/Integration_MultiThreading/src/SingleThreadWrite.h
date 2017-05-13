#ifndef TEST_MTHREAD_SINGLETHREADWRITE_H
#define TEST_MTHREAD_SINGLETHREADWRITE_H 1

#include "TestEnv/Testing.h"

class SingleThreadWrite : public Testing
{

public:

  SingleThreadWrite( const TestEnv& env, size_t no );

  virtual ~SingleThreadWrite();

  void operator()();

private:

  size_t m_tableno;

};

#endif
