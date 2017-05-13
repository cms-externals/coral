#include "RelationalAccess/ISessionProxy.h"
#include <string>

class MysqlSchema
{
public:
  MysqlSchema( coral::ISessionProxy *proxy );

  ~MysqlSchema();

  void listTables(const std::string& schema);



private:

  coral::ISessionProxy *m_proxy;

};
