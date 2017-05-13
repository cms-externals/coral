#ifndef TEST_NETWORKFAILURE_H
#define TEST_NETWORKFAILURE_H 1

#include "TestEnv/SealSwitch.h"
#include "TestEnv/Testing.h"
#include "RelationalAccess/ISchema.h"

class StoneHandler;

class NetworkFailure : public Testing
{
public:

  NetworkFailure( const TestEnv& env );

  virtual ~NetworkFailure();

  void setup();

  void test01();

  void test02();

  void test03();

  void test04();

private:

  coral::SealSwitcher m_sw;

  StoneHandler* m_sh;

};

#endif
