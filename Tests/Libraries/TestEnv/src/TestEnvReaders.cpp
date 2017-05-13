#include "TestEnv/TestEnvReaders.h"

#include <iostream>

TestEnvReaders::TestEnvReaders(const std::string& writer)
  : m_current("")
{
  //create the comatible list
  if( writer.compare("oracle") == 0 ) {
    m_complist.push_back("oracle");
    m_complist.push_back("coral");
    m_complist.push_back("frontier");
  }else if( writer.compare("mysql") == 0 ) {
    m_complist.push_back("mysql");
  }else if( writer.compare("sqlite") == 0 ) {
    m_complist.push_back("sqlite");
    m_complist.push_back("coral");
  }
}

TestEnvReaders::~TestEnvReaders()
{
  m_backends.clear();
}

TestEnvReaders&
TestEnvReaders::add(const std::string backend)
{
  size_t size;
  size = m_complist.size();
  for( size_t i = 0; i < size; i++ ) {
    if(m_complist[i].compare(backend) == 0) m_backends.push_back(backend);
  }
  return *this;
}

TestEnvReaders&
TestEnvReaders::addDefault(const std::string backend)
{
  size_t size;
  size = m_complist.size();
  for( size_t i = 0; i < size; i++ ) {
    if(m_complist[i].compare(backend) == 0) {
      m_backends.push_back(backend);
      m_current = backend;
      return *this;
    }
  }
  return *this;
}

void
TestEnvReaders::setCurrentName(const std::string name)
{
  size_t size;
  size = m_backends.size();
  if( size > 0) {
    m_current = m_backends[0];
    for( size_t i = 0; i < size; i++ ) {
      if(m_backends[i].compare(name) == 0) {
        m_current = m_backends[i];
        return;
      }
    }
  }
  std::cout << "Could not set " << name << " as database backend for Reading." << std::endl;
}

std::string&
TestEnvReaders::getCurrentName()
{
  return m_current;
}

std::vector<std::string>&
TestEnvReaders::entries()
{
  return m_backends;
}
