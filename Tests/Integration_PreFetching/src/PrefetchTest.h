#ifndef TEST_PREFETCH_H
#define TEST_PREFETCH_H 1

#include "TestEnv/Testing.h"

class PreFetch : public Testing
{
public:

  PreFetch( const TestEnv& env );

  virtual ~PreFetch();

  void write();

  void read();

};

#endif
