#ifndef INCLUDE_TESTAPP_H
#define INCLUDE_TESTAPP_H

#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IMonitoringReporter.h"

#include "TestEnv/SealSwitch.h"
#include "TestEnv/TestEnv.h"

#include <string>

namespace coral
{
  class Context;
  class ISessionProxy;
  class IConnectionService;
}

namespace usermon
{
  class UsermonService;
  class UserReporter;
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

private:

  //  coral::IHandle<coral::IConnectionService> connectionService();
  // Returns a reference to the domain object for a given connection
  coral::ISessionProxy* sessionProxy( const std::string& connectionString );

  /// The test functions
  void writeData(coral::IConnectionService& connservice);
  void readData(coral::IConnectionService& connservice);
  void report(coral::IConnectionService& connservice) const;

private:
  std::string T1;

  coral::SealSwitcher sw;

  // The connection service handle
  //  coral::IHandle<coral::IConnectionService>  m_conSvc;
  // The custom monitoring service
  //  usermon::UsermonService*                   m_monSvc;
  // The custom monitoring reporter
  //  usermon::UserReporter*                     m_monReporter;
};

#endif
