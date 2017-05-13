#ifndef TESTCORE_TESTENV_H
#define TESTCORE_TESTENV_H 1

#define TEST_CORE_SCHEME_ADMIN  "admin"
#define TEST_CORE_SCHEME_READER "reader"
#define TEST_CORE_SCHEME_WRITER "writer"
#define TEST_CORE_SCHEME_PROXY "proxy"

#define TEST_CORE_SERVICE_0 "service0"
#define TEST_CORE_SERVICE_1 "service1"
#define TEST_CORE_SERVICE_2 "service2"
#define TEST_CORE_SERVICE_3 "service3"

#define TEST_CORE_REPLICA_SERVICE_0 "replica0"
#define TEST_CORE_REPLICA_SERVICE_1 "replica1"
#define TEST_CORE_REPLICA_SERVICE_2 "replica2"
#define TEST_CORE_REPLICA_SERVICE_3 "replica3"
#define TEST_CORE_REPLICA_SERVICE_4 "replica4"

#define TEST_CORE_REPLICA_EXCLUDING_0 "replicaExcluding0"
#define TEST_CORE_REPLICA_EXCLUDING_1 "replicaExcluding1"
#define TEST_CORE_REPLICA_EXCLUDING_2 "replicaExcluding2"

#define TEST_CORE_DEVSERVICE_0 "devservice0"
#define TEST_CORE_DEVSERVICE_1 "devservice1"

#define TEXT_CORE_FAILOVER_SERVICE_0 "failoverService0"

#include <string>

#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"

#include "TestEnvWriters.h"
#include "TestEnvExceptions.h"

class TestEnv : public coral::CoralCppUnitDBTest
{

public:
  //constructor
  TestEnv(std::string testName);
  //destructor
  virtual ~TestEnv();
  //check environment and program parameters
  bool check(int argc, char * argv[]);
  //set a service name
  size_t addServiceName(const char* writerexp, const char* readerexp = 0);
  //get the amount of all service names
  size_t getNumberOfServiceNames()
  {
    return m_services.size();
  }
  //get the service name using the index
  const std::string getServiceName(size_t index, coral::AccessMode mode = coral::Update) const;
  //add unique per-platform and per-slot prefix to table names
  void addTablePrefix(std::string& fullName, const std::string& shortName) const;
  //lookup dblookup file
  const std::string& lookupFile() const
  {
    return m_lookupfile;
  }
  //set an environment variable during the test
  //createconfig: if the name is found as file, the relative path will be added
  void setEnv( const std::string& key, const std::string& value );

private:

  void setupEnv();

  void dumpEnv();

  void dumpUsage();

protected:

  virtual void setupUserLock() {};

protected:

  std::string m_testName;

private:

  std::vector< std::pair<std::string, std::string> > m_services;

  bool m_env;

  std::string m_lookupfile;

  std::vector< std::pair<std::string, std::string> > m_evars;

};

#endif
