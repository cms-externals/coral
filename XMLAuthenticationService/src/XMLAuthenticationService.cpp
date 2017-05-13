#include "XMLAuthenticationService.h"
#include "DataSourceEntry.h"
#include "XercesWrapper.h"

#include "RelationalAccess/AuthenticationServiceException.h"

#include "CoralBase/MessageStream.h"
#include "CoralCommon/SearchPath.h"

#include "RelationalAccess/AuthenticationCredentials.h"

#include "CoralKernel/Context.h"
#include "CoralKernel/IPropertyManager.h"

#include <boost/bind.hpp>

#include <memory>
#include <cstdlib>
#include <iostream>

namespace coral
{
  namespace XMLAuthenticationService
  {
    const std::string authFileName = "authentication.xml";
  }
}

coral::XMLAuthenticationService::XMLAuthenticationService::XMLAuthenticationService( const std::string& componentName )
  : coral::Service( componentName ),
    m_isInitialized( false ),
    m_data()
{
  boost::function1<void, std::string> cb(boost::bind(&coral::XMLAuthenticationService::XMLAuthenticationService::setInputFileName, this, _1));
  coral::Property* pm = dynamic_cast<coral::Property*>(coral::Context::instance().PropertyManager().property("AuthenticationFile"));
  m_inputFileName = pm->get();
  m_callbackID = pm->registerCallback(cb);

}


coral::XMLAuthenticationService::XMLAuthenticationService::~XMLAuthenticationService()
{
  coral::Property* pm = dynamic_cast<coral::Property*>(coral::Context::instance().PropertyManager().property("AuthenticationFile"));
  pm->unregisterCallback(m_callbackID);

  for ( std::map< std::string, coral::XMLAuthenticationService::DataSourceEntry* >::iterator iConnection = m_data.begin();
        iConnection != m_data.end(); ++iConnection ) delete iConnection->second;
}


void
coral::XMLAuthenticationService::XMLAuthenticationService::setInputFileName(  const std::string& inputFileName )
{
  m_inputFileName = inputFileName;
  for ( std::map< std::string, coral::XMLAuthenticationService::DataSourceEntry* >::iterator iConnection = m_data.begin();
        iConnection != m_data.end(); ++iConnection ) delete iConnection->second;
  m_data.clear();
  m_isInitialized = false;
}


bool
coral::XMLAuthenticationService::XMLAuthenticationService::processFile( const std::string& inputFileName )
{
  bool result = true;
  try
  {
    xercesc::XercesDOMParser parser;
    parser.setValidationScheme( xercesc::XercesDOMParser::Val_Always );
    parser.setDoNamespaces( true );

    xercesc::HandlerBase errorHandler;
    parser.setErrorHandler( &errorHandler );

    parser.parse( inputFileName.c_str() );

    xercesc::DOMDocument* document = parser.getDocument();

    XMLCh tempStr[20];
    xercesc::XMLString::transcode( "connection", tempStr, 19);

    xercesc::DOMNodeList* connectionList = document->getElementsByTagName( tempStr );

    if ( connectionList )
    {
      XMLSize_t numberOfConnections = connectionList->getLength();

      for ( XMLSize_t iConnection = 0; iConnection < numberOfConnections; ++iConnection )
      {
        xercesc::DOMNode* connectionNode = connectionList->item( iConnection );

        if ( connectionNode )
        {
          char*       connectionName  = xercesc::XMLString::transcode( connectionNode->getAttributes()->item( 0 )->getNodeValue() );
          std::string sConnectionName = connectionName;
          xercesc::XMLString::release( &connectionName );

          // Locate the credential
          coral::XMLAuthenticationService::DataSourceEntry* credential = 0;
          std::map< std::string, coral::XMLAuthenticationService::DataSourceEntry* >::iterator iConnection2 = m_data.find( sConnectionName );
          if ( iConnection2 != m_data.end() ) {
            credential = iConnection2->second;
            // Issue a warning here.
            coral::MessageStream log( this->name() );
            log << coral::Warning << "Credential parameters for connection string \""
                << sConnectionName
                << "\" have already been defined. Only new elements are appended, while existing will be ignored."
                << coral::MessageStream::endmsg;
          }
          else {
            credential = new coral::XMLAuthenticationService::DataSourceEntry( this->name(), sConnectionName );
            m_data.insert( std::make_pair( sConnectionName, credential ) );
          }

          xercesc::DOMNodeList* parameterList = connectionNode->getChildNodes();

          if ( parameterList )
          {
            XMLSize_t numberOfParameters = parameterList->getLength();

            for ( XMLSize_t iParameter = 0; iParameter < numberOfParameters; ++iParameter )
            {
              xercesc::DOMNode* parameterNode = parameterList->item( iParameter );

              if ( parameterNode && parameterNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE )
              {
                char* nodeName = xercesc::XMLString::transcode( parameterNode->getNodeName() );
                std::string sNodeName = nodeName;
                xercesc::XMLString::release( &nodeName );

                if ( sNodeName == "parameter" ) { // The default parameters
                  char* parameterName = xercesc::XMLString::transcode( parameterNode->getAttributes()->item( 0 )->getNodeValue() );
                  std::string sParameterName = parameterName;
                  xercesc::XMLString::release( &parameterName );
                  char* parameterValue = xercesc::XMLString::transcode( parameterNode->getAttributes()->item( 1 )->getNodeValue() );
                  std::string sParameterValue = parameterValue;
                  xercesc::XMLString::release( &parameterValue );

                  credential->appendCredentialItem( sParameterName, sParameterValue );
                }
                else if ( sNodeName == "role" ) { // A role
                  char* roleName  = xercesc::XMLString::transcode( parameterNode->getAttributes()->item( 0 )->getNodeValue() );
                  std::string sRoleName = roleName;
                  xercesc::XMLString::release( &roleName );

                  // Retrieve the parameters for the role
                  xercesc::DOMNodeList* roleParameterList = parameterNode->getChildNodes();


                  if ( roleParameterList )
                  {
                    XMLSize_t numberOfRoleParameters = roleParameterList->getLength();

                    for ( XMLSize_t iRoleParameter = 0; iRoleParameter < numberOfRoleParameters; ++iRoleParameter )
                    {
                      xercesc::DOMNode* roleParameterNode = roleParameterList->item( iRoleParameter );
                      if ( roleParameterNode && roleParameterNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE )
                      {
                        char* roleNodeName = xercesc::XMLString::transcode( roleParameterNode->getNodeName() );
                        std::string sRoleNodeName = roleNodeName;
                        xercesc::XMLString::release( &roleNodeName );

                        if ( sRoleNodeName == "parameter" ) {
                          char* roleParameterName = xercesc::XMLString::transcode( roleParameterNode->getAttributes()->item( 0 )->getNodeValue() );
                          std::string sRoleParameterName = roleParameterName;
                          xercesc::XMLString::release( &roleParameterName );
                          char* roleParameterValue = xercesc::XMLString::transcode( roleParameterNode->getAttributes()->item( 1 )->getNodeValue() );
                          std::string sRoleParameterValue = roleParameterValue;
                          xercesc::XMLString::release( &roleParameterValue );

                          credential->appendCredentialItemForRole( sRoleParameterName, sRoleParameterValue, sRoleName );
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    parser.reset();
  }
  catch ( const xercesc::XMLException& toCatch )
  {
    char* message = xercesc::XMLString::transcode( toCatch.getMessage() );
    coral::MessageStream log( this->name() );
    log << coral::Error << message << coral::MessageStream::endmsg;
    xercesc::XMLString::release( &message );
    result = false;
  }
  catch ( const xercesc::DOMException& toCatch )
  {
    char* message = xercesc::XMLString::transcode( toCatch.msg );
    coral::MessageStream log( this->name() );
    log << coral::Error << message << coral::MessageStream::endmsg;
    xercesc::XMLString::release( &message );
    result = false;
  }
  catch (...)
  {
    coral::MessageStream log( this->name() );
    log << coral::Error << "Unexpected Exception parsing file \"" << inputFileName << "\"" << coral::MessageStream::endmsg;
    result = false;
  }

  return result;
}


bool
coral::XMLAuthenticationService::XMLAuthenticationService::initialize()
{
  std::vector< std::string > inputFileNames = this->verifyFileName();
  if ( inputFileNames.empty() )
  {
    coral::MessageStream log( this->name() );
    log << coral::Error << "Could not open \"" << m_inputFileName << "\" for reading" << coral::MessageStream::endmsg;
    return false;
  }

  try
  {
    xercesc::XMLPlatformUtils::Initialize();
  }
  catch ( const xercesc::XMLException& toCatch )
  {
    char* message = xercesc::XMLString::transcode( toCatch.getMessage() );
    coral::MessageStream log( this->name() );
    log << coral::Error << message << coral::MessageStream::endmsg;
    xercesc::XMLString::release( &message );
    return false;
  }

  bool result = true;
  for ( std::vector< std::string >::const_iterator iFileName = inputFileNames.begin();
        iFileName != inputFileNames.end(); ++iFileName ) {
    if ( ! this->processFile( *iFileName ) ) {
      result = false;
      break;
    }
  }

  xercesc::XMLPlatformUtils::Terminate();

  m_isInitialized = result;
  return result;
}


const coral::IAuthenticationCredentials&
coral::XMLAuthenticationService::XMLAuthenticationService::credentials( const std::string& connectionString ) const
{
  boost::mutex::scoped_lock lock(m_mutexLock);
  if ( ! m_isInitialized ) {
    const_cast< coral::XMLAuthenticationService::XMLAuthenticationService* >( this )->initialize();
  }
  std::map< std::string, coral::XMLAuthenticationService::DataSourceEntry* >::const_iterator iConnection = m_data.find( connectionString );
  if ( iConnection == m_data.end() )
    throw coral::UnknownConnectionException( this->name(), connectionString );
  return iConnection->second->credentials();
}


const coral::IAuthenticationCredentials&
coral::XMLAuthenticationService::XMLAuthenticationService::credentials( const std::string& connectionString,
                                                                        const std::string& role ) const
{
  boost::mutex::scoped_lock lock(m_mutexLock);
  if ( ! m_isInitialized ) {
    const_cast< coral::XMLAuthenticationService::XMLAuthenticationService* >( this )->initialize();
  }
  std::map< std::string, coral::XMLAuthenticationService::DataSourceEntry* >::const_iterator iConnection = m_data.find( connectionString );
  if ( iConnection == m_data.end() )
    throw coral::UnknownConnectionException( this->name(), connectionString );
  return iConnection->second->credentials( role );
}


std::vector< std::string >
coral::XMLAuthenticationService::XMLAuthenticationService::verifyFileName() const
{
  std::vector< std::string > fileNames;

  // If the file name is not a fully qualified file name (absolute path),
  // try to find it first in each of the paths in CORAL_AUTH_PATH
  const char* thePathVariable = ::getenv( "CORAL_AUTH_PATH" );
  if ( ! boost::filesystem::path( m_inputFileName ).is_complete() &&
       thePathVariable )
  {
    coral::SearchPath path( thePathVariable );
    for ( coral::SearchPath::const_iterator iDirectory = path.begin(); iDirectory != path.end(); ++iDirectory )
    {
      try
      {
        boost::filesystem::path guess( *iDirectory/m_inputFileName );
        if ( boost::filesystem::exists( guess.string() ) )
        {
          coral::MessageStream log( this->name() );
          log << coral::Info << "Authentication using " << guess.string()
              << " file" << coral::MessageStream::endmsg;
          fileNames.push_back( guess.string() );
        }
      }
      catch( const boost::filesystem::filesystem_error &e )
      {
        coral::MessageStream log( this->name() );
        log << coral::Error
            << "Did not find " << m_inputFileName
            << " in " << iDirectory->string() << std::endl
            << "The problem was: " << e.what() << coral::MessageStream::endmsg;
      }
    }
  }

  // Then try to find the file name in the current directory (this is actually
  // the only path that will be tried if the file name is an absolute path)
  try
  {
    if ( boost::filesystem::exists( m_inputFileName ) )
    {
      coral::MessageStream log( this->name() );
      log << coral::Info << "Authentication using " << boost::filesystem::current_path().string() << "/" <<  m_inputFileName << " file" << coral::MessageStream::endmsg;
      fileNames.push_back( m_inputFileName );
    }
  }
  catch( const boost::filesystem::filesystem_error &e )
  {
    coral::MessageStream log( this->name() );
    log << coral::Error
        << "Did not find " << m_inputFileName << " (current directory is "
        << boost::filesystem::current_path().string() << ")" << std::endl
        << "The problem was: " << e.what() << coral::MessageStream::endmsg;
  }

  return fileNames;
}
