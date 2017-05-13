#include "CoralKernel/Context.h"

#include <set>
#include <algorithm>
#include <iostream>

class Dummy
{
public:
  virtual ~Dummy() {};

  virtual void go() = 0;
};

int main( int, char** )
{
  std::string prefix = "CORAL/Plugins/";

  typedef std::set<std::string> Plugs;

  Plugs pl;

  pl.insert( "CORAL/Services/RelationalService" );
  pl.insert( "CORAL/Plugins/sqlite" );
  pl.insert( "CORAL/Plugins/mysql" );
  pl.insert( "CORAL/Plugins/oracle" );

  std::cout << "Plugs has " << pl.size() << " items" << std::endl;

  for( Plugs::iterator plit = pl.begin(); plit != pl.end(); ++plit )
  {
    std::string::size_type pos = (*plit).find( prefix );
    if( pos != std::string::npos )
    {
      std::cout << "Found plugin: " << *plit << std::endl;
      std::cout << "Technology: " << (*plit).substr(prefix.size()) << std::endl;
    }
  }

  // The bug causing segfault if an empty handle is returned by coral::Context::query<...>()
  // FIXED now
  coral::IHandle<Dummy> dumdum;
  dumdum = coral::Context::instance().query<Dummy>();

  return 0;
}
