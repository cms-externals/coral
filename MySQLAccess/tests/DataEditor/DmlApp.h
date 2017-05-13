#ifndef DMLAPP_H
#define DMLAPP_H

#include "../Common/TestBase.h"

class DmlApp : virtual public TestBase
{
public:
  DmlApp( const std::string& connectionString,
          const std::string& userName,
          const std::string& password );
  virtual ~DmlApp();
  void run();

private:
  std::string m_connectionString;
  std::string m_userName;
  std::string m_password;
};

#endif
