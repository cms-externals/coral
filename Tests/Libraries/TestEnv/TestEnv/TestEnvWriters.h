#ifndef TESTCORE_TESTENVWRITERS_H
#define TESTCORE_TESTENVWRITERS_H 1

#include <map>
#include <string>

#include "TestEnvReaders.h"

class TestEnvWriters {
public:
  //constructor
  TestEnvWriters();
  //destructor
  ~TestEnvWriters();
  //add a new backend to the writers
  TestEnvWriters& add(const std::string backend);
  //add a new backend to the writers
  TestEnvWriters& addDefault(const std::string backend);
  //get the readers from the writer string
  TestEnvReaders& readers(const std::string backend);
  //set the current name
  void setCurrentName(const std::string name);
  //get current name
  std::string& getCurrentName();
  //get amount of entries
  void entries(std::vector<std::string>& storing);

private:

  std::map<std::string, TestEnvReaders*> m_backends;

  std::string m_current;

};

#endif
