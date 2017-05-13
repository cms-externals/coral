// $Id: CALOpcodeException.h,v 1.1.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_CALOPCODEEXCEPTION_H
#define CORALSERVERBASE_CALOPCODEEXCEPTION_H 1

// Include files
#include "CoralBase/Exception.h"

namespace coral
{

  /** @class CALOpcodeException
   *
   *  Exception thrown when manipulating CAL opcodes.
   *
   *  @author Andrea Valassi
   *  @date   2009-01-23
   */

  class CALOpcodeException : public Exception
  {

  public:

    /// Constructor
    CALOpcodeException( const std::string& message,
                        const std::string& method )
      : Exception( message, method, "coral::CoralServerBase" ){}

    /// Destructor
    virtual ~CALOpcodeException() throw() {}

  };

}
#endif // CORALSERVERBASE_CALOPCODEEXCEPTION_H
