#ifndef CORAL_CORALSTUBS_REQUESTHANDLER_H
#define CORAL_CORALSTUBS_REQUESTHANDLER_H

#include "CoralServerBase/IRequestHandler.h"
#include "CoralServerBase/ICoralFacade.h"

namespace coral {

  class ByteBuffer;

  namespace CoralStubs
  {

    // forward declarations
    struct rowIteratorMap;

    /** @class ServerStub
     *
     *  An inheritance of the IRequestHandler used for the server side
     *
     *  @author Alexander Kalkhof
     *  @date   2009-01-28
     */

    class ServerStub : public IRequestHandler
    {

    public:

      // Constructor from an ICoralFacade.
      ServerStub( ICoralFacade& coralFacade );

      // Virtual destructor.
      virtual ~ServerStub();

      void setCertificateData( const coral::ICertificateData* cert );

      IByteBufferIteratorPtr replyToRequest( const ByteBuffer& requestBuffer );

    private:

      ICoralFacade& m_facade;

      rowIteratorMap* m_rowimap;

    };

  }

}
#endif
