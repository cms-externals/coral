#ifndef TEST_USERTESTS_H
#define TEST_USERTESTS_H 1

#include "TestEnv/Testing.h"
#include "RelationalAccess/ISchema.h"

class TestUserTests : public Testing
{
public:

  TestUserTests( const TestEnv& env );

  virtual ~TestUserTests();

  void test01();

private:

};

#endif
