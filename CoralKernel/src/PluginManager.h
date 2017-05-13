#ifndef CORALKERNEL_PLUGINMANAGER_H
#define CORALKERNEL_PLUGINMANAGER_H 1

#include "CoralKernel/IPluginManager.h"

#include "CoralBase/boost_thread_headers.h"

#include <list>
#include <map>

namespace coral
{

  class ILoadableComponentFactory;

  /// A Simple implementation of the plugin manager for CORAL
  class PluginManager : virtual public IPluginManager
  {
  public:
    /// Constructor
    PluginManager();

    /// Destructor
    virtual ~PluginManager();

    /// Creates a new ILoadableComponent object given its name
    ILoadableComponent* newComponent( const std::string& componentName );

    /// Returns the list of known components
    virtual std::set<std::string> knownPlugins() const;

  private:
    /// No copy constructor
    PluginManager( const PluginManager& );
    /// No assignment operator
    PluginManager& operator=( const PluginManager& );

    /// Initializes the object
    void initialize();

    /// Loads a library and retrieves the plugins
    void loadLibrary( const std::string& libraryName );

    /// Reads the plugin definitions from a directory name
    void readPluginsFromDir( const std::string& directoryName );

  private:
    /// The plugin factories name -> library name, factory object
    std::map< std::string, std::pair< std::string, ILoadableComponentFactory* > > m_factories;

    /// Flag indicating whether the object has been initialized
    bool m_initialized;

    /// The mutex lock
    boost::mutex m_mutex;

    /// The list of open libraries
    std::list< void* > m_openLibraries;
  };

}

#endif
