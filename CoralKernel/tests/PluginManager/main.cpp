#include <iostream>
#include "CoralKernel/ILoadableComponent.h"
#include "CoralKernel/../src/PluginManager.h"

void load( const std::string& pluginName )
{
  coral::PluginManager mgr;
  std::cout << "Load component " << pluginName << std::endl;
  coral::ILoadableComponent* comp = 0;
  try
  {
    comp = mgr.newComponent( pluginName );
  }
  catch( std::exception& e )
  {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }
  catch( ... )
  {
    std::cout << "Unknown exception caught" << std::endl;
  }
  if ( comp )
    std::cout << "Retrieved ILoadableComponent " << comp
              << " (" << comp->name() << ")" << std::endl;
  else
    std::cout << "ERROR! Could not retrieve ILoadableComponent " << std::endl;
}

int main( int, char** )
{
  // Simple test for bug #64014 on Windows
  load( "CORAL/RelationalPlugins/sqlite" );
}
