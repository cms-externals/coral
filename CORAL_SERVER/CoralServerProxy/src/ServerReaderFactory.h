#ifndef CORALSERVERPROXY_SERVERREADERFACTORY_H
#define CORALSERVERPROXY_SERVERREADERFACTORY_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ServerReaderFactory.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ServerReaderFactory.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

//----------------------
// Base Class Headers --
//----------------------


//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

/**
 *  Class which is responsible for the connection to the upstream server and
 *  creation of the server reader thread.
 *
 *  @see AdditionalClass
 *
 *  @version $Id: ServerReaderFactory.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andrei Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class ClientConnManager ;
class NetEndpointAddress ;
class NetSocket ;
class PacketQueue ;

class ServerReaderFactory  {
public:

  // Default constructor
  ServerReaderFactory ( const NetEndpointAddress& serverAddress,
                        PacketQueue& rcvQueue,
                        unsigned timeoutSec ) ;

  // Destructor
  ~ServerReaderFactory () ;

  // Connect to a server, return socket. Socket will be in a closed
  // state if there is a problem in connection.
  NetSocket serverConnect( ClientConnManager& connManager ) const ;

protected:

  // set appropriate socket options for a server socket
  int setSocketOptions( NetSocket& sock ) const ;

private:

  // Data members

  const NetEndpointAddress& m_serverAddress ;
  PacketQueue& m_rcvQueue ;
  unsigned m_timeoutSec ;

  // Copy constructor and assignment are disabled by default
  ServerReaderFactory operator = ( const ServerReaderFactory& ) ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_SERVERREADERFACTORY_H
