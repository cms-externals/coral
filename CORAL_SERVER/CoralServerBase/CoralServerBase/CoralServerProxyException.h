// $Id: CoralServerProxyException.h,v 1.1 2009/02/11 18:19:23 avalassi Exp $
#ifndef CORALSERVERBASE_CORALSERVERPROXYEXCEPTION_H
#define CORALSERVERBASE_CORALSERVERPROXYEXCEPTION_H 1

// Include files
#include "CoralBase/Exception.h"

namespace coral 
{

  /** @class CoralServerProxyException
   *
   *  Exception thrown by the CoralServerProxy package.
   *
   *  @author Andrea Valassi, Alexander Kalkhof and Martin Wache
   *  @date   2009-02-11
   */

  class CoralServerProxyException : public Exception
  { 
    
  public:
    
    /// Constructor
    CoralServerProxyException( const std::string& message,
                               const std::string& method )
      : Exception( message, method, "coral::CoralServerProxy" ){}
    
    /// Destructor
    virtual ~CoralServerProxyException() throw() {}
 
    /// Encode the exception as a CAL packet payload
    const std::string asCALPayload() const;

  };
  
}
#endif // CORALSERVERBASE_CORALSERVERPROXYEXCEPTION_H
