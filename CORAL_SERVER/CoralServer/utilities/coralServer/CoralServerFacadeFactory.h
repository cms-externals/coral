// $Id: CoralServerFacadeFactory.h,v 1.1.2.2 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_CORALSERVERFACADEFACTORY_H
#define CORALSERVER_CORALSERVERFACADEFACTORY_H 1

// Include files
#include "CoralServer/CoralServerFacade.h"
#include "CoralServer/ICoralFacadeFactory.h"
#include "CoralServer/TokenGenerator.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class CoralServerFacadeFactory
     *
     *  Factory of CoralServerFacade's.
     *
     *  The coralServer's ServerStubFactory uses an CoralServerFacadeFactory.
     *  Each ServerStub instantiates its own CoralServerFacade,
     *  which is deleted when the ServerStub is deleted.
     *
     *  @author Andrea Valassi
     *  @date   2009-03-02
     */

    class CoralServerFacadeFactory : virtual public ICoralFacadeFactory
    {

    public:

      /// Constructor from an IToke.
      CoralServerFacadeFactory(){}

      /// Destructor.
      virtual ~CoralServerFacadeFactory(){}

      /// Return a new dedicated coral facade.
      /// The returned facade is non-const (may need to create mutex locks).
      /// This method is non-const (it may need to perform some bookkeeping).
      ICoralFacade* newCoralFacade()
      {
        return new CoralServerFacade( &m_tokenPool );
      }

    private:

      /// Copy constructor is private
      CoralServerFacadeFactory( const CoralServerFacadeFactory& rhs );

      /// Assignment operator is private
      CoralServerFacadeFactory& operator=( const CoralServerFacadeFactory& rhs );

    private:

      /// The Token generator
      TokenGenerator m_tokenPool;

    };

  }

}
#endif // CORALSERVER_ICORALFACADEFACTORY_H
