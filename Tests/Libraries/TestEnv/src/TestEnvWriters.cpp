#include "TestEnv/TestEnvWriters.h"

#include <iostream>

TestEnvWriters::TestEnvWriters()
  : m_current("")
{
}

TestEnvWriters::~TestEnvWriters()
{
  std::map<std::string, TestEnvReaders*>::iterator i(m_backends.begin());
  for( ; i != m_backends.end(); i++ ) {
    delete i->second;
  }
  m_backends.clear();
}

TestEnvWriters&
TestEnvWriters::add(const std::string backend)
{
  m_backends.insert(std::pair<std::string, TestEnvReaders*>(backend, new TestEnvReaders(backend)));
  return *this;
}

TestEnvWriters&
TestEnvWriters::addDefault(const std::string backend)
{
  m_current = backend;
  return add(backend);
}

TestEnvReaders&
TestEnvWriters::readers(const std::string backend)
{
  return *(m_backends[backend]);
}

void
TestEnvWriters::setCurrentName(const std::string name)
{
  std::map<std::string, TestEnvReaders*>::iterator i(m_backends.begin());
  for( ; i != m_backends.end(); i++ ) {
    if( i->first.compare(name) == 0) {
      m_current = i->first;
      return;
    }
  }
  std::cout << "Could not set " << name << " as database backend for Writing." << std::endl;
}

std::string&
TestEnvWriters::getCurrentName()
{
  if( m_current.empty() ) {
    //it means we have no default backend
    //take the first element instead
    std::map<std::string, TestEnvReaders*>::iterator i(m_backends.begin());
    m_current = i->first;
  }
  return m_current;
}

void
TestEnvWriters::entries(std::vector<std::string>& storing)
{
  storing.clear();
  std::map<std::string, TestEnvReaders*>::iterator i(m_backends.begin());
  for( ; i != m_backends.end(); i++ ) {
    storing.push_back(i->first);
  }
}
