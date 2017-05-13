#include <iostream>
#include "CoralKernel/Context.h"
#include "CoralKernel/ILoadableComponent.h"
#include "PluginManager.h"
#include "PropertyManager.h"


coral::Context&
coral::Context::instance( coral::IPluginManager* pluginManager )
{
  static coral::Context theInstance( pluginManager );
  return theInstance;
}


bool
coral::Context::existsComponent( const std::string& componentName )
{
  boost::mutex::scoped_lock lock( m_mutex );
  if ( m_components.find( componentName ) != m_components.end() )
    return true;
  else
    return false;
}


void
coral::Context::loadComponent( const std::string& componentName,
                               coral::IPluginManager* pluginManager )
{
  /// Check if the component has already been loaded.
  if ( this->existsComponent( componentName ) ) return;

  /// Use the provided plugin manager or the default implementation
  coral::IPluginManager* thePluginManager = pluginManager;
  if ( pluginManager == 0 )
  {
    boost::mutex::scoped_lock lock( m_mutex );
    if ( ! m_pluginManager )
    {
      m_nativePluginManager = new coral::PluginManager();
      m_pluginManager = m_nativePluginManager;
    }
    thePluginManager = m_pluginManager;
  }

  // Load the component
  ILoadableComponent* comp = thePluginManager->newComponent( componentName );
  if ( comp )
  {
    boost::mutex::scoped_lock lock( m_mutex );
    m_components.insert( std::make_pair( componentName, comp ) );
  }
}


coral::IPropertyManager& coral::Context::PropertyManager()
{
  return *m_propertyManager;
}


std::set<std::string>
coral::Context::loadedComponents() const
{
  std::set<std::string> componentSet;
  for ( std::map<std::string, ILoadableComponent*>::const_iterator
          iCom = m_components.begin(); iCom != m_components.end(); iCom++ )
    componentSet.insert( iCom->first );
  return componentSet;
}


std::set<std::string>
coral::Context::knownComponents() const
{
  std::set<std::string> componentSet = m_pluginManager->knownPlugins();
  return componentSet;
}


coral::Context::Context( coral::IPluginManager* pluginManager )
  : m_mutex()
  , m_components()
  , m_nativePluginManager( 0 )
  , m_pluginManager( 0 )
  , m_propertyManager(new coral::PropertyManager)
{
  //std::cout << "Create Context " << this << std::endl;
  if ( pluginManager == 0 )
  {
    boost::mutex::scoped_lock lock( m_mutex );
    if ( ! m_pluginManager )
    {
      m_nativePluginManager = new coral::PluginManager();
      m_pluginManager = m_nativePluginManager;
    }
  }
  else
    m_pluginManager = pluginManager;
}


coral::Context::~Context()
{
  //std::cout << "Delete Context " << this << "..." << std::endl;
  ILoadableComponent* pMonSvc = 0;
  // Fix destruction order of loaded components (bug #63042 and bug #73529).
  // Destruction order is normally the inverse of construction order,
  // but the CoralMonitoringService should always be deleted last.
  for ( std::map<std::string, ILoadableComponent*>::const_iterator
          iCom = m_components.begin(); iCom != m_components.end(); iCom++ )
  {
    std::string name = iCom->second->name();
    if( name == "CORAL/Services/CoralMonitoringService" )
    {
      pMonSvc = iCom->second; // todo: check pMonSvc==0? or names are unique?
    }
    else
    {
      // Delete all components except the monitoring service
      iCom->second->removeReference();
      //int count = iCom->second->removeReference();
      //std::cout << "RefCount-- " << iCom->second << " " << name
      //          << ": " << count << std::endl;
    }
  }
  // Delete the monitoring service last
  if( pMonSvc )
  {
    pMonSvc->removeReference();
    //int count = pMonSvc->removeReference();
    //std::string name = pMonSvc->name();
    //std::cout << "RefCount-- " << pMonSvc << " " << name
    //          << ": " << count << std::endl;
  }

  // Clear all components from the map
  m_components.clear();
  // Delete the native plugin if it exists
  if ( m_nativePluginManager ) delete m_nativePluginManager;
  //std::cout << "Delete Context " << this << "... done" << std::endl;
}
