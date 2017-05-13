#include "RelationalService.h"

#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/RelationalServiceException.h"

#include "CoralCommon/URIParser.h"

#include "CoralKernel/Context.h"

#include "CoralBase/MessageStream.h"

#include <sstream>

namespace coral
{
  namespace RelationalService
  {
    /*     class CallbackForRDBMSImplementations {
    public:
      explicit CallbackForRDBMSImplementations( coral::RelationalService::RelationalService& relationalService ):
        m_relationalService( relationalService )
      {}

      void operator() ( const seal::PropertyBase& p ) {
        m_relationalService.m_properties = boost::any_cast< std::vector< std::string > >( p.get() );
        m_relationalService.setDefaultImplementations();
      }

    private:
      coral::RelationalService::RelationalService& m_relationalService;
    };
    */
  }
}

coral::RelationalService::RelationalService::RelationalService( const std::string& componentName )
  : coral::Service( componentName ),
    m_pluginPrefix( "CORAL/RelationalPlugins/" ),
    m_technologiesAndImplementations(),
    m_defaultImplementations(),
    m_properties(),
    m_mutexForDomains()
{
  //   propertyManager()->declareProperty( "DefaultImplementations",
  //                                       m_properties,
  //                                       std::vector<std::string>(),
  //                                       "The default implementations for the RDBMS technologies.",
  //                                       coral::RelationalService::CallbackForRDBMSImplementations( *this ) );
  this->retrievePlugins();
  this->setDefaultImplementations();
}

coral::RelationalService::RelationalService::~RelationalService()
{
}

coral::IRelationalDomain&
coral::RelationalService::RelationalService::domain( const std::string& technology )
{
  // Check if the technology string is void
  if ( technology.empty() ) throw coral::NonExistingDomainException( technology );

  // Check if the technology is supported
  std::map< std::string, std::string >::const_iterator iDefaultImplementation = m_defaultImplementations.find( technology );
  if ( iDefaultImplementation == m_defaultImplementations.end() ) {
    throw coral::NonExistingDomainException( technology );
  }

  // Search for the domain object in the current context tree.
  std::string componentName = m_pluginPrefix + technology;

  if ( !( iDefaultImplementation->second.empty() ) ) {
    componentName += "/" + iDefaultImplementation->second;
  }

  boost::mutex::scoped_lock lock( m_mutexForDomains );

  coral::IHandle<coral::IRelationalDomain> iHandle = coral::Context::instance().query<coral::IRelationalDomain>( componentName );

  if ( ! iHandle.isValid() )
  {
    coral::Context::instance().loadComponent( componentName );
    // If not existing throw an exception.
    iHandle = coral::Context::instance().query<coral::IRelationalDomain>( componentName );

    if ( ! iHandle.isValid() )
    {
      throw coral::NonExistingDomainException( technology );
    }
  }

  return *iHandle;
}

coral::IRelationalDomain&
coral::RelationalService::RelationalService::domainForConnection( const std::string& connectionString )
{
  coral::URIParser parser;
  parser.setURI( connectionString );
  return this->domain( parser.technology() );
}

void
coral::RelationalService::RelationalService::retrievePlugins()
{
  /*
  seal::PluginManager& pluginManager = *( seal::PluginManager::get());
  for ( seal::PluginManager::ModuleIterator iModule = pluginManager.beginModules();
        iModule != pluginManager.endModules(); ++iModule ) {
    const seal::Module& module = **iModule;
    for ( seal::Module::InfoIterator info = module.beginInfos();
          info != module.endInfos(); ++info ) {
      const seal::PluginInfo& pluginInfo = **info;
      if ( pluginInfo.category() == seal::ComponentFactory::get()->category() ) {
        const std::string& name = pluginInfo.name();
        if ( name.find( m_pluginPrefix ) != 0 ) continue;
        std::string technology = name.substr( m_pluginPrefix.size() );
        std::string implementation = "";
        std::string::size_type slashPosition = technology.find("/");
        if ( slashPosition != std::string::npos ) {
          implementation = technology.substr( slashPosition + 1 );
          technology = technology.substr( 0, slashPosition );
        }
        m_technologiesAndImplementations[ technology ].insert( implementation );

        log << seal::Msg::Info << "Found plugin for RDBMS technology \"" << technology << "\"";
        if ( ! implementation.empty() ) {
          log << " with implementation \"" << implementation << "\"";
        }
        else {
          log << " with native implementation";
        }
        log << seal::flush;
      }
    }
  }
  */

  coral::MessageStream log( this->name() );
  std::set<std::string> cList = coral::Context::instance().knownComponents();

  for( std::set<std::string>::const_iterator comit = cList.begin(); comit != cList.end(); ++comit )
  {
    std::string name = *comit;

    if ( name.find( m_pluginPrefix ) != 0 )
      continue;

    std::string technology = name.substr( m_pluginPrefix.size() );

    std::string implementation = "";
    std::string::size_type slashPosition = technology.find("/");

    if ( slashPosition != std::string::npos )
    {
      implementation = technology.substr( slashPosition + 1 );
      technology     = technology.substr( 0, slashPosition );
    }

    m_technologiesAndImplementations[ technology ].insert( implementation );

    log << coral::Info << "Found plugin for RDBMS technology \"" << technology << "\"";
    if ( ! implementation.empty() ) {
      log << " with implementation \"" << implementation << "\"";
    }
    else {
      log << " with native implementation";
    }
    log << coral::MessageStream::endmsg;
  }
}

void
coral::RelationalService::RelationalService::setDefaultImplementations()
{
  coral::MessageStream log( this->name() );

  for ( std::map< std::string, std::set< std::string > >::const_iterator iTechnology = m_technologiesAndImplementations.begin();
        iTechnology != m_technologiesAndImplementations.end(); ++iTechnology ) {
    m_defaultImplementations[ iTechnology->first ] = *( iTechnology->second.begin() );
  }

  // Retrieve other defaults from the properties
  for ( std::vector<std::string>::const_iterator iProperty = m_properties.begin();
        iProperty != m_properties.end(); ++iProperty ) {
    std::string::size_type iDoubleColonPosition = iProperty->find( ":" );
    if ( iDoubleColonPosition == std::string::npos ) continue;
    std::istringstream isTechnology( iProperty->substr( 0, iDoubleColonPosition ).c_str() );
    std::string technology;
    isTechnology >> technology;
    std::map< std::string, std::set< std::string > >::const_iterator iTechnology = m_technologiesAndImplementations.find( technology );
    if ( iTechnology == m_technologiesAndImplementations.end() ) {
      log << coral::Warning << "RDBMS technology \"" << technology << "\" is not supported." << coral::MessageStream::endmsg;
      continue;
    }
    std::istringstream isImplementation( iProperty->substr( iDoubleColonPosition + 1 ).c_str() );
    std::string implementation;
    isImplementation >> implementation;
    if ( iTechnology->second.find( implementation ) == iTechnology->second.end() ) {
      log << coral::Warning << "No ";
      if ( implementation.empty() ) log << "native";
      else log << "\"" << implementation << "\"";
      log << " implementation exists for RDBMS technology \"" << technology << "\"" << coral::MessageStream::endmsg;
      continue;
    }
    m_defaultImplementations[ technology ] = implementation;
  }

  // Print the current default implementations.
  for ( std::map< std::string, std::string >::const_iterator iTechnology = m_defaultImplementations.begin();
        iTechnology != m_defaultImplementations.end(); ++iTechnology ) {
    log << coral::Info << "Default implementation for RDBMS technology \"" << iTechnology->first << "\" is ";
    if ( iTechnology->second.empty() ) {
      log << "native" << coral::MessageStream::endmsg;
    }
    else {
      log << "\"" << iTechnology->second << "\"" << coral::MessageStream::endmsg;
    }
  }
}


void
coral::RelationalService::RelationalService::setDefaultImplementationForDomain( const std::string& technology,
                                                                                std::string implementation )
{
  coral::MessageStream log( this->name() );
  std::map< std::string, std::set< std::string > >::const_iterator iTechnology = m_technologiesAndImplementations.find( technology );
  if ( iTechnology == m_technologiesAndImplementations.end() ) {
    throw coral::NonExistingDomainException( technology );
  }

  if ( iTechnology->second.find( implementation ) == iTechnology->second.end() ) {
    throw coral::NonExistingDomainException( technology + "/" + implementation );
  }

  m_defaultImplementations[ technology ] = implementation;
  log << coral::Info << "Setting default implementation for RDBMS technology \"" << technology << "\" to ";
  if ( implementation.empty() ) {
    log << "native" << coral::MessageStream::endmsg;
  }
  else {
    log << "\"" << implementation << "\"" << coral::MessageStream::endmsg;
  }
}


std::vector< std::string >
coral::RelationalService::RelationalService::availableTechnologies() const
{
  std::vector< std::string > result;
  for ( std::map< std::string, std::set< std::string > >::const_iterator iTechnology = m_technologiesAndImplementations.begin();
        iTechnology != m_technologiesAndImplementations.end(); ++iTechnology )
    result.push_back( iTechnology->first );
  return result;
}


std::vector< std::string >
coral::RelationalService::RelationalService::availableImplementationsForTechnology( const std::string& technologyName ) const
{
  std::vector< std::string > result;
  std::map< std::string, std::set< std::string > >::const_iterator iTechnology = m_technologiesAndImplementations.find( technologyName );
  for ( std::set< std::string >::const_iterator iImplementation = iTechnology->second.begin();
        iImplementation != iTechnology->second.end(); ++iImplementation )
    result.push_back( *iImplementation );
  return result;
}
