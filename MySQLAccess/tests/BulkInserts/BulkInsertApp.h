#ifndef BULKINSERTAPP_H
#define BULKINSERTAPP_H

#include "../Common/TestBase.h"

class BulkInsertApp : virtual public TestBase
{
public:
  BulkInsertApp( const std::string& connectionString, const std::string& userName, const std::string& password, coral::AccessMode=coral::Update );
  virtual ~BulkInsertApp();
  void run();

private:
  void fillData();
  void readData();

private:
  std::string m_connectionString;
  std::string m_userName;
  std::string m_password;
  coral::AccessMode m_accessMode;
};

#endif
