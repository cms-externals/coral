// $Id: IRequestHandler.h,v 1.9.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_IREQUESTHANDLER_H
#define CORALSERVERBASE_IREQUESTHANDLER_H 1

// Include files
#include <memory>
#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/IByteBufferIterator.h"

namespace coral
{
  // forward declaration
  class ICertificateData;

  /// Buffer iterator pointer.
  typedef std::auto_ptr<IByteBufferIterator> IByteBufferIteratorPtr;

  /** @class IRequestHandler
   *
   *  Interface to a request handler for the CORAL server project.
   *
   *  A request handler delivers one logical reply to each logical request.
   *  One logical reply can be segmented in several physical buffers.
   *  One logical request is presently described by only one physical buffer.
   *
   *  @author Andrea Valassi, Martin Wache and Alexander Kalkhof
   *  @date   2007-12-04
   */

  class IRequestHandler
  {

  public:

    // Destructor.
    virtual ~IRequestHandler(){}

    /// Set the certificate data for this request handler.
    /// The caller has to ensure that the ICertificateData pointer
    /// remains valid over the lifetime of the IRequestHandler.
    virtual void setCertificateData( const coral::ICertificateData* cert ) = 0;

    // Handle a request message and return an iterator over reply messages.
    // This method cannot be const because we need to lock mutexes in it!
    // The iterator is positioned before the first reply in the loop:
    // the next() method must be called to retrieve the first reply.
    // The request handler must be kept alive while the iterator is used:
    // the iterator uses a byte buffer pool owned by the request handler.
    virtual IByteBufferIteratorPtr replyToRequest( const ByteBuffer& requestBuffer ) = 0;

  };

}
#endif // CORALSERVERBASE_IREQUESTHANDLER_H
