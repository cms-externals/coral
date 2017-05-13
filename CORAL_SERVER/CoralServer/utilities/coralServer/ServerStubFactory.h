// $Id: ServerStubFactory.h,v 1.1.2.2 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_SERVERSTUBFACTORY_H
#define CORALSERVER_SERVERSTUBFACTORY_H 1

// Include files
#include "CoralServerBase/IRequestHandlerFactory.h"
#include "CoralServer/ICoralFacadeFactory.h"

// Local include files
#include "ServerStubWrapper.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class IRequestHandlerFactory
     *
     *  Factory of ServerStub's.
     *
     *  The coralServer's SocketServer uses a ServerStubFactory.
     *  For each new connection a separate ServerStub is created,
     *  which is deleted when the connection is closed.
     *  Each ServerStub uses a different CoralServerFacade (each using
     *  its own ObjectStoreMgr - but ObjectID's are globally unique).
     *
     *  @author Andrea Valassi
     *  @date   2009-03-02
     */

    class ServerStubFactory : virtual public IRequestHandlerFactory
    {

    public:

      /// Constructor from an ICoralFacadeFactory.
      ServerStubFactory( ICoralFacadeFactory& facadeFactory )
        : m_facadeFactory( facadeFactory )
      {
      }

      /// Destructor.
      virtual ~ServerStubFactory()
      {
      }

      /// Return a new dedicated request handler.
      /// The returned handler is non-const (may need to create mutex locks).
      /// This method is non-const (may need to perform some bookkeeping).
      IRequestHandler* newRequestHandler()
      {
        return new ServerStubWrapper( *(m_facadeFactory.newCoralFacade()) );
      }

    private:

      /// Copy constructor is private
      ServerStubFactory( const ServerStubFactory& rhs );

      /// Assignment operator is private
      ServerStubFactory& operator=( const ServerStubFactory& rhs );

    private:

      /// The ICoralFacadeFactory.
      ICoralFacadeFactory& m_facadeFactory;

    };

  }
}
#endif // CORALSERVER_SERVERSTUBFACTORY_H
