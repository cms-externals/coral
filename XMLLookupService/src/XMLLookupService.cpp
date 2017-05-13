#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>

#include "CoralBase/boost_filesystem_headers.h"
#include "CoralBase/MessageStream.h"
#include "CoralCommon/DatabaseServiceDescription.h"
#include "CoralCommon/DatabaseServiceSet.h"
#include "CoralCommon/SearchPath.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/IPropertyManager.h"

#include "XercesWrapper.h"
#include "XMLLookupService.h"

namespace coral
{
  namespace XMLLookupService
  {
    const std::string lookupFileName = "dblookup.xml";
  }
}

coral::XMLLookupService::XMLLookupService::XMLLookupService( const std::string& componentName ) :
  coral::Service( componentName ),
  m_isInitialized( false ),
  m_data()
{
  boost::function1<void, std::string> cb(boost::bind(&coral::XMLLookupService::XMLLookupService::setInputFileName, this, _1));
  coral::Property* pm = dynamic_cast<coral::Property*>(coral::Context::instance().PropertyManager().property("DBLookupFile"));
  m_inputFileName = pm->get();
  m_callbackID = pm->registerCallback(cb);
}

coral::XMLLookupService::XMLLookupService::~XMLLookupService()
{
  coral::Property* pm = dynamic_cast<coral::Property*>(coral::Context::instance().PropertyManager().property("DBLookupFile"));
  pm->unregisterCallback(m_callbackID);
}


void
coral::XMLLookupService::XMLLookupService::setInputFileName(  const std::string& inputFileName )
{
  boost::mutex::scoped_lock lock(m_mutexLock);
  m_inputFileName = inputFileName;
  m_isInitialized = false;
  m_data.clear();
}


bool
coral::XMLLookupService::XMLLookupService::initialize()
{
  std::string inputFileName = this->verifyFileName();
  if ( inputFileName.empty() ) {
    // We do not give up as it might be not a filename but a URI
    if( this->m_inputFileName.find( "http:" ) != std::string::npos ) {

      // It's a URI, we believe that a user supplied a valid URI
      inputFileName = this->m_inputFileName;
      coral::MessageStream log( this->name() );
      log << coral::Warning << "Trying to open \"" << m_inputFileName << "\" for reading via HTTP" << coral::MessageStream::endmsg;
    } else {
      coral::MessageStream log( this->name() );
      log << coral::Error << "Could not open \"" << m_inputFileName << "\" for reading" << coral::MessageStream::endmsg;
      return false;
    }
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
    xercesc::XMLString::transcode( "logicalservice", tempStr, 19);

    xercesc::DOMNodeList* connectionList = document->getElementsByTagName( tempStr );

    if ( connectionList )
    {
      XMLSize_t numberOfConnections = connectionList->getLength();

      for ( XMLSize_t iConnection = 0; iConnection < numberOfConnections; ++iConnection )
      {
        xercesc::DOMNode* connectionNode = connectionList->item( iConnection );

        if ( connectionNode ) {
          char* sLogicalServiceName = xercesc::XMLString::transcode( connectionNode->getAttributes()->item( 0 )->getNodeValue() );
          std::string logicalServiceName = sLogicalServiceName;
          m_data[logicalServiceName] = std::vector< coral::XMLLookupService::XMLLookupService::DbEntry >(); // create or replace list...
          std::vector< coral::XMLLookupService::XMLLookupService::DbEntry >& vectorOfEntries = m_data[logicalServiceName];
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

                if ( sNodeName != "service" )
                  continue;

                std::string connectionName = "";
                std::string authenticationMechanism = "";
                coral::AccessMode accesMode = coral::ReadOnly;

                int nparams = parameterNode->getAttributes()->getLength();

                for ( int i = 0; i < nparams; ++i ) {
                  char* parameterValue = xercesc::XMLString::transcode( parameterNode->getAttributes()->item( i )->getNodeValue() );
                  std::string sParameterValue = parameterValue;
                  xercesc::XMLString::release( &parameterValue );

                  char* parameterName = xercesc::XMLString::transcode( parameterNode->getAttributes()->item( i )->getNodeName() );
                  std::string sParameterName = parameterName;
                  xercesc::XMLString::release( &parameterName );

                  if ( sParameterName == "name" ) {
                    connectionName = sParameterValue;
                  }
                  else if ( sParameterName == "authentication" ) {
                    authenticationMechanism = sParameterValue;
                  }
                  else if ( sParameterName == "accessMode" ) {
                    if ( sParameterValue.find( "update" ) != std::string::npos ) {
                      accesMode = coral::Update;
                    }
                  }
                }
                vectorOfEntries.push_back( coral::XMLLookupService::XMLLookupService::DbEntry( connectionName,
                                                                                               authenticationMechanism,
                                                                                               accesMode ) );
                // Retrieve the service parameters
                xercesc::DOMNodeList* serviceParameterList = parameterNode->getChildNodes();
                if ( serviceParameterList ) {
                  XMLSize_t numberOfServiceParameters = serviceParameterList->getLength();
                  for ( XMLSize_t iServiceParameter = 0; iServiceParameter < numberOfServiceParameters; ++iServiceParameter ) {
                    xercesc::DOMNode* serviceParameterNode = serviceParameterList->item( iServiceParameter );
                    if ( serviceParameterNode && serviceParameterNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE ) {
                      char* serviceNodeName = xercesc::XMLString::transcode( serviceParameterNode->getNodeName() );
                      std::string sServiceNodeName = serviceNodeName;
                      xercesc::XMLString::release( &serviceNodeName );
                      if ( sServiceNodeName == "parameter" ) {
                        XMLSize_t numberOfAttributes = serviceParameterNode->getAttributes()->getLength();
                        std::string serviceParameterName("");
                        std::string serviceParameterValue("");
                        for(XMLSize_t iAttr = 0; iAttr<numberOfAttributes; ++iAttr) {
                          char* attrName = xercesc::XMLString::transcode( serviceParameterNode->getAttributes()->item( iAttr )->getNodeName() );
                          char* attrValue = xercesc::XMLString::transcode( serviceParameterNode->getAttributes()->item( iAttr )->getNodeValue() );
                          if(::strcmp(attrName,"name")==0) {
                            serviceParameterName = attrValue;
                          } else if (::strcmp(attrName,"value")==0) {
                            serviceParameterValue = attrValue;
                          }
                          xercesc::XMLString::release( &attrValue );
                          xercesc::XMLString::release( &attrName );
                        }
                        if(!serviceParameterName.empty() && !serviceParameterValue.empty()) {
                          coral::XMLLookupService::XMLLookupService::DbEntry& vecEntry = vectorOfEntries.back();
                          vecEntry.serviceParameters.insert(std::make_pair(serviceParameterName,serviceParameterValue));
                        }

                      }
                    }
                  }
                }
              }

            }
            xercesc::XMLString::release( &sLogicalServiceName );
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

  xercesc::XMLPlatformUtils::Terminate();

  m_isInitialized = result;
  return result;
}


coral::IDatabaseServiceSet*
coral::XMLLookupService::XMLLookupService::lookup( const std::string& logicalName,
                                                   coral::AccessMode accessMode,
                                                   std::string authenticationMechanism ) const
{

  boost::mutex::scoped_lock lock(m_mutexLock);
  if ( ! m_isInitialized ) const_cast<coral::XMLLookupService::XMLLookupService*>( this )->initialize();

  coral::DatabaseServiceSet* result = new coral::DatabaseServiceSet( this->name() );

  std::map< std::string, std::vector< coral::XMLLookupService::XMLLookupService::DbEntry > >::const_iterator iLogicalEntry = m_data.find( logicalName );
  if ( iLogicalEntry == m_data.end() ) return static_cast<coral::IDatabaseServiceSet*>( result );
  const std::vector< coral::XMLLookupService::XMLLookupService::DbEntry >& entryVector = iLogicalEntry->second;
  for ( std::vector< coral::XMLLookupService::XMLLookupService::DbEntry >::const_iterator iEntry = entryVector.begin();
        iEntry != entryVector.end(); ++iEntry ) {
    const coral::XMLLookupService::XMLLookupService::DbEntry& entry = *iEntry;
    if ( ( accessMode == coral::ReadOnly || // check the access mode first
           accessMode == entry.accessMode ) &&
         ( authenticationMechanism.empty() || // check the authentication mechanism(s)
           entry.authenticationMechanism.find( authenticationMechanism ) != std::string::npos ) ) {
      coral::DatabaseServiceDescription& descr = result->appendReplica( entry.connection,
                                                                        ( authenticationMechanism.empty() ) ? entry.authenticationMechanism : authenticationMechanism,
                                                                        entry.accessMode );
      descr.serviceParameters() = entry.serviceParameters;
    }
  }

  return static_cast<coral::IDatabaseServiceSet*>( result );
}

std::string
coral::XMLLookupService::XMLLookupService::verifyFileName() const
{
  // If the file name is not a fully qualified file name (absolute path),
  // try to find it first in each of the paths in CORAL_DBLOOKUP_PATH
  const char* thePathVariable = ::getenv( "CORAL_DBLOOKUP_PATH" );
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
          std::cout << "Data source lookup using " << guess.string()
                    << " file" << std::endl;
          return std::string( guess.string() );
        }
      }
      catch( const boost::filesystem::filesystem_error& e )
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
      std::cout << "Data source lookup using "
                << boost::filesystem::current_path().string() << "/"
                << m_inputFileName << " file" << std::endl;
      return m_inputFileName;
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

  return std::string("");
}
