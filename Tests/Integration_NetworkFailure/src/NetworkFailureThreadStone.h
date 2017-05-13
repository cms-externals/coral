#ifndef TEST_NETWORKFAILURE_THREADSTONE_H
#define TEST_NETWORKFAILURE_THREADSTONE_H 1

#include <string>

// Forward declaration
class StoneHandler;

class NetworkFailureThreadStone
{

public:
  // Constructor
  NetworkFailureThreadStone(StoneHandler& sh)
    : m_sh( sh )
  {}

  // Destructor
  ~NetworkFailureThreadStone() {}

  // The thread body
  void operator()();

private:

  StoneHandler& m_sh;

};

#endif
