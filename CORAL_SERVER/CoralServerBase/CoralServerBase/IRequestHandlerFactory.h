// $Id: IRequestHandlerFactory.h,v 1.1.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_IREQUESTHANDLERFACTORY_H
#define CORALSERVERBASE_IREQUESTHANDLERFACTORY_H 1

// Include files
#include "CoralServerBase/IRequestHandler.h"

namespace coral
{

  /** @class IRequestHandlerFactory
   *
   *  Abstract interface to a factory of IRequestHandler's.
   *
   *  The coralServer's SocketServer uses an IRequestHandlerFactory.
   *  For each new connection a separate IRequestHandler is created,
   *  which is deleted when the connection is closed.
   *
   *  @author Andrea Valassi
   *  @date   2007-12-26
   */

  class IRequestHandlerFactory
  {

  public:

    /// Destructor.
    virtual ~IRequestHandlerFactory(){}

    /// Return a new dedicated request handler.
    /// The returned handler is non-const (it may need to create mutex locks).
    /// This method is non-const (it may need to perform some bookkeeping).
    virtual IRequestHandler* newRequestHandler() = 0;

  };

}
#endif // CORALSERVERBASE_IREQUESTHANDLERFACTORY_H
