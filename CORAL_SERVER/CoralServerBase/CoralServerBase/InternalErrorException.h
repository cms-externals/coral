// $Id: InternalErrorException.h,v 1.1.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_INTERNALERROREXCEPTION_H
#define CORALSERVERBASE_INTERNALERROREXCEPTION_H 1

// Include files
#include "CoralBase/Exception.h"

namespace coral
{

  //--------------------------------------------------------------------------

  /** @class InternalErrorException InternalErrorException.h
   *
   *  Exception thrown when a CORAL internal error occurs ("PANIC").
   *  These exceptions signal bugs in the internal logic of CORAL algorithms.
   *  **** No such exceptions should ever be thrown. If you catch one,  ***
   *  **** please report this immediately to the CORAL development team. ***
   *
   *  @author Andrea Valassi
   *  @date   2008-07-31 (for COOL)
   *  @date   2009-01-28 (for CORAL)
   */

  class InternalErrorException : public Exception
  {

  public:

    /// Constructor
    explicit InternalErrorException( const std::string& message,
                                     const std::string& methodName,
                                     const std::string& moduleName )
      : Exception( "*** CORAL INTERNAL ERROR *** " + message,
                   methodName, moduleName ) {}

    /// Destructor
    virtual ~InternalErrorException() throw() {}

  };

  //--------------------------------------------------------------------------

}
#endif // CORALSERVERBASE_INTERNALERROREXCEPTION_H
