#ifndef CORALSERVERPROXY_TPCMANAGER_H
#define CORALSERVERPROXY_TPCMANAGER_H

// --------------------------------------------------------------------------
// File and Version Information:
	// $Id: TPCManager.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $

// Description:
	// Class TPCManager.

// ------------------------------------------------------------------------

// -----------------
// C/C++ Headers --
// -----------------
#include <string>

// ----------------------
// Base Class Headers --
// ----------------------

// -------------------------------
// Collaborating Class Headers --
// -------------------------------
#include "NetEndpointAddress.h"
#include "NetSocket.h"

// ------------------------------------
// Collaborating Class Declarations --
// ------------------------------------

		// ---------------------
		// -- Class Interface --
		// ---------------------

/**
 *  "Runnable" class (in terms of Boost.thread) which handles the
 *  connections from clients and instantiates all the threads.
 *  This implements thread-per-connection (TPC) model, instantiating
 *  new SingleClientReader thread for every new incoming connection.
 *  It also creates all communication/sinchronization queues and
 *  reader/writer/dispatcher which communicate through these queues.
 *
 *  @version $Id: TPCManager.h,v 1.1.2.1 2010/05/26 08:12:41 avalassi Exp $
 *
 *  @author Andy Salnikov
 */

namespace coral {
namespace CoralServerProxy {

class IPacketCache ;
class StatCollector ;

class TPCManager  {
public:

  // Default constructor
  TPCManager ( const NetEndpointAddress& listenAddress,
               const NetEndpointAddress& serverAddress,
               size_t maxQueueSize,
               unsigned timeout,
               IPacketCache& cache,
               StatCollector* statCollector ) ;

  // Destructor
  ~TPCManager () ;

  // this is the "run" method used by the Boost.thread
  void operator() () ;

protected:

  // start listening
  NetSocket setupListener() ;

  // set common socket options
  int setSocketOptions( NetSocket& sock ) ;

private:

  // Data members
  NetEndpointAddress m_listenAddress ;   // listenning iface address
  NetEndpointAddress m_serverAddress ;  // upstream server address
  size_t m_maxQueueSize ;
  unsigned m_timeout ;
  IPacketCache& m_cache;
  StatCollector* m_statCollector ;

};

} // namespace CoralServerProxy
} // namespace coral

#endif // CORALSERVERPROXY_TPCMANAGER_H
