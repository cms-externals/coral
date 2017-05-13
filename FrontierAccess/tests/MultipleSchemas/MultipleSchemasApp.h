#ifndef CONNECTIONAPP_H
#define CONNECTIONAPP_H

#include "../Common/TestBase.h"

class MultipleSchemasApp : virtual public TestBase
{
public:
  MultipleSchemasApp( const std::string& connectionString,
                      const std::string& userName,
                      const std::string& password,
                      const std::string& otherSchema );
  virtual ~MultipleSchemasApp();
  void run();
private:
  std::string m_connectionString;
  std::string m_userName;
  std::string m_password;
  std::string m_otherSchema;
};

#endif
