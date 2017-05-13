#ifndef QUERIESAPP_H
#define QUERIESAPP_H

#include "../Common/TestBase.h"

class QueriesApp : virtual public TestBase
{
public:
  QueriesApp( const std::string& connectionString1, const std::string& connectionString2, const std::string& userName, const std::string& password );
  virtual ~QueriesApp();
  void run();

private:
  void fillData();
  void readData();

private:
  std::string m_connectionString1;
  std::string m_connectionString2;
  std::string m_userName;
  std::string m_password;
};

#endif
