#ifndef TEST_NETWORKFAILURE_STONEHANDLER_H
#define TEST_NETWORKFAILURE_STONEHANDLER_H 1

#include <string>

class StoneHandler
{

public:

  StoneHandler();

  ~StoneHandler();

  void activate();

  void deactivate();

private:

  std::string m_cmd;

  int m_pid;

};


#endif
