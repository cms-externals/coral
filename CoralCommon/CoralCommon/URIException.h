#ifndef CORALCOMMON_URI_EXCEPTION_H
#define CORALCOMMON_URI_EXCEPTION_H

#include "CoralBase/Exception.h"

namespace coral {

  class URIException : public Exception
  {
  public:
    /// Constructor
    URIException( const std::string& uriString ) :
      Exception( "URI string \"" + uriString + "\" is malformed",
                 "URIParser::parse", "CoralCommon" )
    {}

    /// Destructor
    virtual ~URIException() throw() {}
  };

}

#endif
