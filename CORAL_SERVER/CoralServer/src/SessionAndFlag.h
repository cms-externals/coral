#ifndef CORALSERVER_SESSIONANDFLAG_H
#define CORALSERVER_SESSIONANDFLAG_H 1

// Include files
#include "RelationalAccess/ISessionProxy.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class SessionAndFlag
     *
     *  This is just a pair { ISessionProxy*, bool }.
     *  It is needed to register both in the IObjectStoreMgr.
     *
     *  @author Andrea Valassi
     *  @date   2009-02-02
     */

    class SessionAndFlag
    {

    public:

      /// Constructor.
      SessionAndFlag( ISessionProxy* session,
                      bool fromProxy )
        : m_session( session ), m_fromProxy( fromProxy ) {}

      /// Destructor.
      virtual ~SessionAndFlag()
      {
        delete m_session;
      }

      /// The session
      ISessionProxy* session() const
      {
        return m_session;
      }

      /// The flag
      bool fromProxy() const
      {
        return m_fromProxy;
      }

    private:

      /// The session.
      ISessionProxy* m_session;

      /// Is there a CORAL server proxy between client and server?
      bool m_fromProxy;

    };

  }

}
#endif // CORALSERVER_SESSIONANDFLAG_H
