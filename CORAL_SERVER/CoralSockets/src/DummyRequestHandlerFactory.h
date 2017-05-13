// $Id: DummyRequestHandlerFactory.h,v 1.1.2.1 2010/12/20 09:10:11 avalassi Exp $
#ifndef CORALSOCKETS_IDUMMYREQUESTHANDLERFACTORY_H
#define CORALSOCKETS_IDUMMYREQUESTHANDLERFACTORY_H 1

// Include files
#include "CoralServerBase/IRequestHandlerFactory.h"
#include "DummyRequestHandler.h"

namespace coral
{
  namespace CoralSockets
  {

    /** @class DummyRequestHandlerFactory
     *
     *  @author Martin Wache
     *  @date   2009-02-27
     */

    class DummyRequestHandlerFactory : public IRequestHandlerFactory
    {

    public:

      /// Destructor.
      virtual ~DummyRequestHandlerFactory(){}

      /// Return a new dedicated request handler.
      /// For each new connection a seperate request handler is created,
      /// which is deleted when the connection is closed.
      /// The returned handler is non-const (it may need to create mutex locks).
      /// This method is non-const (it may need to perform some bookkeeping).
      virtual IRequestHandler* newRequestHandler()
      {
        return new DummyRequestHandler();
      };

    };
  }
}
#endif // CORALSOCKETS_IDUMMYREQUESTHANDLERFACTORY_H
