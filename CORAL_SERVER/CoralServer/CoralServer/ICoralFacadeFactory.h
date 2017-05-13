// $Id: ICoralFacadeFactory.h,v 1.1.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_ICORALFACADEFACTORY_H
#define CORALSERVER_ICORALFACADEFACTORY_H 1

// Include files
#include "CoralServerBase/ICoralFacade.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class ICoralFacadeFactory
     *
     *  Abstract interface to a factory of ICoralFacade's.
     *
     *  The coralServer's ServerStubFactory uses an ICoralFacadeFactory.
     *  Each ServerStub instantiates its own ICoralFacade,
     *  which is deleted when the ServerStub is deleted.
     *
     *  @author Andrea Valassi
     *  @date   2007-12-26
     */

    class ICoralFacadeFactory
    {

    public:

      /// Destructor.
      virtual ~ICoralFacadeFactory(){}

      /// Return a new dedicated coral facade.
      /// The returned facade is non-const (may need to create mutex locks).
      /// This method is non-const (it may need to perform some bookkeeping).
      virtual ICoralFacade* newCoralFacade() = 0;

    };

  }

}
#endif // CORALSERVER_ICORALFACADEFACTORY_H
