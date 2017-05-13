#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <string>

class TestDriver
{
public:
  TestDriver();
  ~TestDriver();

  void run( const std::string& serviceName );

private:

  void prepareSchema( const std::string& serviceName );
  void writeData( const std::string& serviceName );
  void readData( const std::string& serviceName );
};

#endif
