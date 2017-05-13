#ifndef CORALBASE_CORALPLUGINDEF_H
#define CORALBASE_CORALPLUGINDEF_H

#include "ILoadableComponentFactory.h"
#include "ILoadableComponent.h"

namespace coral {
  template<typename T> class CoralPluginFactory : virtual public coral::ILoadableComponentFactory
  {
  public:
    CoralPluginFactory( const std::string& name ) : m_name( name ) {}
    virtual ~CoralPluginFactory() {}
    coral::ILoadableComponent* component() const
    { return static_cast< coral::ILoadableComponent* >( new T( m_name ) ); }
    std::string name() const { return m_name; }
  private:
    std::string m_name;
  };
}


#define CORAL_PLUGIN_MODULE(NAME,PLUGINCLASS) \
  static coral::CoralPluginFactory< PLUGINCLASS > theFactory( std::string( NAME ) ); \
  extern "C" { coral::ILoadableComponentFactory& coral_component_factory = theFactory; }

#endif
