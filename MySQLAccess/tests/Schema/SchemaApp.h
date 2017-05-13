#ifndef SCHEMAAPP_H
#define SCHEMAAPP_H

#include "../Common/TestBase.h"

class SchemaApp : virtual public TestBase
{
public:
  SchemaApp( const std::string& connectionString,
             const std::string& userName,
             const std::string& password );
  virtual ~SchemaApp();
  void run();

private:
  std::string m_connectionString;
  std::string m_userName;
  std::string m_password;
};

#endif
