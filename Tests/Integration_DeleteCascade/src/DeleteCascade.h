#ifndef TEST_DELETECASCADE_H
#define TEST_DELETECASCADE_H 1

#include "TestEnv/Testing.h"
#include "RelationalAccess/ISchema.h"

class DeleteCascade : public Testing
{
public:

  DeleteCascade( const TestEnv& env );

  virtual ~DeleteCascade();

  void setup( bool cascade );

  void test( bool cascade );

private:

};

#endif
