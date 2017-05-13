#ifndef TEST_APP_H
#define TEST_APP_H 1

#include "TestEnv/Testing.h"

class InputOutput : public Testing {

public:
  InputOutput( const TestEnv& env );

  void prepareSchema();

  void writeSimple();

  void writeBulk();

  void read();

  void prepareBigSchema();

  void readBigSchema();

  void prepareTypes();

  void readTypes();
};

#endif
