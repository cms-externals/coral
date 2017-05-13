// $Id: coralDummySocketServer.cpp,v 1.1.2.4 2011/09/15 15:23:42 avalassi Exp $

// Include files
#include <cstdlib> // For setenv and _putenv
#include <iostream>
#include "CoralMonitor/StopTimer.h"
#include "CoralSockets/SocketServer.h"

// Local include files
#include "../../src/DummyRequestHandlerFactory.h"

// Namespace
using namespace coral::CoralSockets;

//-----------------------------------------------------------------------------

/** @file socketServerMT.cpp
 *
 *  @author Andrea Valassi and Martin Wache
 *  @date   2007-12-26
 */

int main( int argc, char** argv )
{

  try
  {

    // Get the server parameters
    std::string host;
    int port;
    //int nHThreads;
    //bool debug = false;
    if ( argc == 1 )
    {
      host = "localhost";
      port = 50017;
      //nHThreads = 10;
      //debug = true;
    }
    /*
    else if ( argc == 7 || argc == 8 )
    {
      host = argv[1];
      port = atoi( argv[2] );
      bufferSize = atoi( argv[3] );
      timeout = atoi( argv[4] );
      nHThreads = atoi( argv[5] );
      if ( atoi( argv[6] ) == 1 ) debug = true;
      if ( argc == 8 && std::string( argv[7] ) == std::string( "SLAC" ) ) SLAC = true;
    }
    else
    {
      LOG << "Usage:   " << argv[0]
          << " [host port bufferSize timeout #handlerThreadsPerSocket debug [SLAC]]" << std::endl;
      LOG << "Example: " << argv[0] << " localhost 50017 8192 10 10 1" << std::endl;
      LOG << "Example: " << argv[0] << " localhost 60017 8192 10 10 1 SLAC" << std::endl;
      return 1;
    }
    */
    else
    {
      std::cout << "Usage:   " << argv[0] << std::endl;
      return 1;
    }
    // Start the server
    std::cout << "Entering main" << std::endl;
    std::cout << "Create a dummy request handler factory" << std::endl;
    //coral::CoralServerBase::DummyRequestHandlerFactory handlerFactory;
    std::cout << "Start the server on " << host << ":" << port << std::endl;
    coral::CoralSockets::DummyRequestHandlerFactory dummyHandlerFactory;
    SocketServer server( dummyHandlerFactory, host, port );
    server.run( 30 );
    std::cout << "Exiting main" << std::endl;
  }

  catch( std::exception& e )
  {
    std::cout << "ERROR! Standard C++ exception: '" << e.what() << "'" << std::endl;
    return 1;
  }

  catch( ... )
  {
    std::cout << "ERROR! Unknown exception caught" << std::endl;
    return 1;
  }

  //coral::printTimers();
  // Successful program termination
  return 0;

}

//-----------------------------------------------------------------------------
