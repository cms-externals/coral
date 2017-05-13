#include "ConnectionService.h"
#include "CoralKernel/CoralPluginDef.h"

CORAL_PLUGIN_MODULE( "CORAL/Services/ConnectionService", coral::ConnectionService::ConnectionService )

/*
#define CORAL_PLUGIN_MODULE(NAME,PLUGINCLASS) \
static coral::CoralPluginFactory< PLUGINCLASS > theFactory( std::string( NAME ) ); \
extern "C" { coral::ILoadableComponentFactory& coral_component_factory = theFactory; }
*/
#define CORAL_PLUGIN2_MODULE(NAME,PLUGINCLASS)                          \
  static coral::CoralPluginFactory< PLUGINCLASS > theFactory2( std::string( NAME ) ); \
  extern "C" { coral::ILoadableComponentFactory& coral_component_factory2 = theFactory2; }
  CORAL_PLUGIN2_MODULE( "CORAL/Services/ConnectionService2", coral::ConnectionService::ConnectionService )
