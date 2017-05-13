#ifndef SCHEMAAPP_H
#define SCHEMAAPP_H

#include "../Common/TestBase.h"

#include <string>

class SchemaApp : virtual public TestBase
{
public:
  SchemaApp( const std::string& connectionString1, const std::string& connectionString2, const std::string& userName, const std::string& password );
  virtual ~SchemaApp();
  void run();

private:
  std::string m_connectionString1;
  std::string m_connectionString2;
  std::string m_userName;
  std::string m_password;
};

#endif
