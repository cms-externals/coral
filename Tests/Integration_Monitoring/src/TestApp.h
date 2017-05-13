#ifndef INCLUDE_TESTAPP_H
#define INCLUDE_TESTAPP_H

#include <string>

#include "TestEnv/SealSwitch.h"
#include "TestEnv/TestEnv.h"

namespace coral
{
  class IRelationalDomain;
}

class TestApp : public TestEnv
{
public:
  // Constructor
  TestApp(const char * testName);

  // Destructor
  ~TestApp();

  // run method
  void run();
  // Loads the basic services
  void loadServices();

private:

  /// The test functions
  void writeData();
  void readData();
  void report();


private:

  coral::SealSwitcher sw;

  std::string T1;

};

#endif
