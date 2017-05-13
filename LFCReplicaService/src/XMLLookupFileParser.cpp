#include "CoralCommon/DatabaseServiceDescription.h"
#include "XMLLookupFileParser.h"
#include "XercesWrapper.h"
#include "xercesc/dom/DOM.hpp"
#include "xercesc/sax/HandlerBase.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/PlatformUtils.hpp"

#include "CoralBase/MessageStream.h"

bool coral::XMLLookupFileParser::parse(const std::string& inputFileName,
                                       std::map< std::string, std::vector< DatabaseServiceDescription* > >& data){
  try
  {
    xercesc::XMLPlatformUtils::Initialize();
  }
  catch ( const xercesc::XMLException& toCatch )
  {
    char* message = xercesc::XMLString::transcode( toCatch.getMessage() );
    coral::MessageStream log( m_serviceCallerName );
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
          data[logicalServiceName] = std::vector< coral::DatabaseServiceDescription* >(); // create or replace list...
          std::vector< coral::DatabaseServiceDescription* >& vectorOfEntries = data[logicalServiceName];
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
                coral::DatabaseServiceDescription* dbEntry = new coral::DatabaseServiceDescription( connectionName,
                                                                                                    authenticationMechanism,
                                                                                                    accesMode );
                vectorOfEntries.push_back( dbEntry );
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
                          dbEntry->serviceParameters().insert(std::make_pair(serviceParameterName,serviceParameterValue));
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
    coral::MessageStream log( m_serviceCallerName );
    log << coral::Error << message << coral::MessageStream::endmsg;
    xercesc::XMLString::release( &message );
    result = false;
  }
  catch ( const xercesc::DOMException& toCatch )
  {
    char* message = xercesc::XMLString::transcode( toCatch.msg );
    coral::MessageStream log( m_serviceCallerName );
    log << coral::Error << message << coral::MessageStream::endmsg;
    xercesc::XMLString::release( &message );
    result = false;
  }
  catch (...)
  {
    coral::MessageStream log( m_serviceCallerName );
    log << coral::Error << "Unexpected Exception parsing file \"" << inputFileName << "\"" << coral::MessageStream::endmsg;
    result = false;
  }

  xercesc::XMLPlatformUtils::Terminate();

  return result;
}
