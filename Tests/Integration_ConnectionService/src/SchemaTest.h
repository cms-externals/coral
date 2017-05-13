#include "RelationalAccess/ISessionProxy.h"

#include "RelationalAccess/TableDescription.h"

namespace coral {
  class ITableDescription;
  class IView;
}

class SchemaTest
{
public:
  SchemaTest( coral::ISessionProxy *proxy );

  ~SchemaTest();

  bool createSchema();

  bool readSchema(const std::string& schema);

  bool Query(const std::string& workingSchema);
  bool bulkOperations();
  bool atomicOperations();

private:

  void printTableInfo( const coral::ITableDescription& description ) const;
  coral::ISessionProxy *m_proxy;
  std::string m_connectionString;
};
