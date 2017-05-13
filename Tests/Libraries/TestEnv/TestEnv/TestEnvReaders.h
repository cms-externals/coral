#ifndef TESTCORE_TESTENVREADERS_H
#define TESTCORE_TESTENVREADERS_H 1

#include <vector>
#include <string>

class TestEnvReaders {
public:
  //constructor
  TestEnvReaders(const std::string& writer);
  //destructor
  ~TestEnvReaders();
  //add a backend to the readers
  //the backend will be checked against its compatibility
  TestEnvReaders& add(const std::string backend);
  //add a backend to the readers
  TestEnvReaders& addDefault(const std::string backend);
  //add all compatible backends
  void addAllBackends();
  //set the current name
  void setCurrentName(const std::string name);
  //get current name
  std::string& getCurrentName();
  //get entries
  std::vector<std::string>& entries();

private:

  std::vector<std::string> m_backends;

  std::vector<std::string> m_complist;

  std::string m_current;
};

#endif
