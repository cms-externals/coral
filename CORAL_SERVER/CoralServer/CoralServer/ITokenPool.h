// $Id: ITokenPool.h,v 1.1.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_ITOKENPOOL_H
#define CORALSERVER_ITOKENPOOL_H 1

// Include files
#include "CoralServerBase/ICoralFacade.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class ITokenPool
     *
     *  Abstract interface to a pool of Token's.
     *  The generated Token's are globally unique within the pool.
     *
     *  @author Andrea Valassi
     *  @date   2009-03-02
     */

    class ITokenPool
    {

    public:

      /// Destructor.
      virtual ~ITokenPool(){}

      /// Allocate (or generate) a Token from the pool.
      /// The implementation of this method must be thread-safe.
      virtual Token allocateToken() = 0;

      /// Release a Token previously allocated from the pool.
      /// The implementation of this method must be thread-safe.
      /// In some implementations the Token may then be reused (no guarantee).
      virtual void releaseToken( Token token ) = 0;

    };

  }

}
#endif // CORALSERVER_ITOKENPOOL_H
