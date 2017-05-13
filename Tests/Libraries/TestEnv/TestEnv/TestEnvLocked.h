#ifndef TESTCORE_TESTENVLOCKED_H
#define TESTCORE_TESTENVLOCKED_H 1

#include "TestEnv.h"

class TestEnvLocked : public TestEnv {
public:
  //constructor
  TestEnvLocked(std::string testName);
  //destructor
  virtual ~TestEnvLocked();

protected:

  void setupUserLock();

private:

  std::string m_lockfile;

  int m_lfd;

};

#endif
