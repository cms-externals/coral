#include <string>
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/IConnectionService.h"
#include "TestEnv/TestEnv.h"

namespace coral
{
  class ISessionProxy;
}

class Connection : public TestEnv
{
public:
  // Constructor
  Connection(const char * testName);

  // Destructor
  ~Connection();

  void Init();

  coral::ISessionProxy* getSessionProxy( int connIndex, coral::AccessMode mode);

  void listTables(coral::ISessionProxy * proxy);

private:
  std::string T1;

};
