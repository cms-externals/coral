// $Id: CTLMagicWordException.h,v 1.2.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_CTLMAGICWORDEXCEPTION_H
#define CORALSERVERBASE_CTLMAGICWORDEXCEPTION_H 1

// Include files
#include "CoralBase/Exception.h"

namespace coral
{

  /** @class CTLMagicWordException
   *
   *  Exception thrown if the magic word is not found in the CTL packet header.
   *
   *  @author Andrea Valassi
   *  @date   2009-01-20
   */

  class CTLMagicWordException : public Exception
  {

  public:

    /// Constructor
    CTLMagicWordException( const std::string& methodName )
      : Exception( "Wrong magic word!",
                   methodName,
                   "coral::CoralServerBase" ) {}

    /// Destructor
    virtual ~CTLMagicWordException() throw() {}

  };

}
#endif // CORALSERVERBASE_CTLMAGICWORDEXCEPTION_H
