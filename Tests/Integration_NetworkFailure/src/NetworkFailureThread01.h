#ifndef TEST_NETWORKFAILURE_THREAD01_H
#define TEST_NETWORKFAILURE_THREAD01_H 1

#include <string>
#include "TestEnv/Testing.h"

class NetworkFailureThread01 : public Testing
{

public:
  // Constructor
  NetworkFailureThread01( const TestEnv& env );

  // Destructor
  ~NetworkFailureThread01();

  // The thread body
  void operator()();

private:


};

#endif
