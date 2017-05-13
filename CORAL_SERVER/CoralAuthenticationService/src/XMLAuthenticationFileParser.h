#ifndef LFCREPLICASERVICE_XMLAUTHENTICATIONFILEPARSER_H
#define LFCREPLICASERVICE_XMLAUTHENTICATIONFILEPARSER_H

#include "RelationalAccess/AccessMode.h"
#include <string>
#include <vector>
#include <map>

namespace coral {

  class AuthenticationCredentialSet;

  class XMLAuthenticationFileParser {
  public:
    explicit XMLAuthenticationFileParser(const std::string& serviceCallerName);

    virtual ~XMLAuthenticationFileParser(){}

    // ownership of the DatabaseServiceDescription* objects is left to the caller
    bool parse(const std::string& inputFileName, std::map< std::string,AuthenticationCredentialSet* >& data);

  private:

    std::string m_serviceCallerName;
  };

}

inline
coral::XMLAuthenticationFileParser::XMLAuthenticationFileParser(const std::string& serviceCallerName) :
  m_serviceCallerName(serviceCallerName){
}

#endif
