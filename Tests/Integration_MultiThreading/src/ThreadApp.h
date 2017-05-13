#ifndef TEST_MTHREAD_THREAD_H
#define TEST_MTHREAD_THREAD_H 1

#include "TestEnv/Testing.h"

class ThreadApp : public Testing
{

public:

  ThreadApp( const TestEnv& env );

  virtual ~ThreadApp();

  void write();

  void read();

};

#endif
