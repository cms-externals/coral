#ifndef LFCREPLICASERVICE_XMLLOOKUPFILEPARSER_H
#define LFCREPLICASERVICE_XMLLOOKUPFILEPARSER_H

#include "RelationalAccess/AccessMode.h"
#include <string>
#include <vector>
#include <map>

namespace coral {

  class DatabaseServiceDescription;

  class XMLLookupFileParser {
  public:
    explicit XMLLookupFileParser(const std::string& serviceCallerName);

    virtual ~XMLLookupFileParser(){}

    // ownership of the DatabaseServiceDescription* objects is left to the caller
    bool parse(const std::string& inputFileName, std::map< std::string, std::vector< DatabaseServiceDescription* > >& data);

  private:

    std::string m_serviceCallerName;
  };

}

inline
coral::XMLLookupFileParser::XMLLookupFileParser(const std::string& serviceCallerName) :
  m_serviceCallerName(serviceCallerName){
}

#endif
