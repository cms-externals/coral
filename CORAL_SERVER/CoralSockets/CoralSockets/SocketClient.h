// $Id: SocketClient.h,v 1.2.2.1 2010/12/20 09:10:11 avalassi Exp $
#ifndef CORALSOCKETS_SOCKETCLIENT_H
#define CORALSOCKETS_SOCKETCLIENT_H 1

// Include files
#include <string>
// Local include files
#include "CoralServerBase/IRequestHandler.h"

namespace coral {

  namespace CoralSockets {
    // forward definition
    class SocketRequestHandler;

    /** @class SocketClient SocketClient.h
     *
     *  @author Andrea Valassi and Martin Wache
     *  @date   2007-12-26
     */

    class SocketClient   {

    public:

      virtual ~SocketClient();

      SocketClient( const std::string& host, int port );

      IRequestHandler *getRequestHandler();

    private:

      /// Standard constructor is private
      SocketClient();

      SocketRequestHandler* m_handler;
    };

  }

}
#endif
