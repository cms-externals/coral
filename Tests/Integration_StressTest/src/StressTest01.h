#ifndef TEST_STRESSTEST_STRESSTEST01_H
#define TEST_STRESSTEST_STRESSTEST01_H 1

#include "TestEnv/Testing.h"

class StressTest01 : public Testing {

public:

  // Constructor
  StressTest01(const TestEnv& env, size_t blobsize);

  // Create schemas
  void prepare();

  // Run read-only tests
  void run(size_t runs);

private:

  size_t m_blobsize;

};

#endif
