#include "AuthenticationCredentialSet.h"
#include "XMLAuthenticationFileParser.h"
#include "XercesWrapper.h"
#include "xercesc/dom/DOM.hpp"
#include "xercesc/sax/HandlerBase.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/PlatformUtils.hpp"

#include "CoralBase/MessageStream.h"

bool coral::XMLAuthenticationFileParser::parse(const std::string& inputFileName,
                                               std::map< std::string, AuthenticationCredentialSet* >& data){
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
          coral::AuthenticationCredentialSet* credential = 0;
          std::map< std::string, coral::AuthenticationCredentialSet* >::iterator iConnection2 = data.find( sConnectionName );
          if ( iConnection2 != data.end() ) {
            credential = iConnection2->second;
            // Issue a warning here.
            coral::MessageStream log( m_serviceCallerName );
            log << coral::Warning << "Credential parameters for connection string \""
                << sConnectionName
                << "\" have already been defined. Only new elements are appended, while existing will be ignored."
                << coral::MessageStream::endmsg;
          }
          else {
            credential = new coral::AuthenticationCredentialSet( m_serviceCallerName, sConnectionName );
            data.insert( std::make_pair( sConnectionName, credential ) );
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
