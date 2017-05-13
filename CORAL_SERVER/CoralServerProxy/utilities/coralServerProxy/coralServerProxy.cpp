//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: coralServerProxy.cpp,v 1.1.2.4 2010/12/20 13:23:32 avalassi Exp $
//
// Description:
//	CoralProxy application
//
// Environment:
//	This software was developed for the ATLAS collaboration.
//
// Author List:
//  Andy salnikov
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include "CoralBase/boost_thread_headers.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "src/PacketCacheMemory.h"
#include "src/TPCManager.h"
#include "src/NetEndpointAddress.h"
#include "src/MsgLogger.h"
#include "src/StatCollector.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using std::cout ;
using std::cerr ;
using std::endl ;

using namespace coral::CoralServerProxy ;

namespace {

  int listen_port = 40007 ;
  int server_port = 40007 ;
  size_t maxCacheSizeMB = 1024 ;
  size_t maxCachePackets = 100000 ;
  size_t queue_size = 100000 ;
  unsigned timeout = 180 ;
  unsigned statSize = 10000 ;
  unsigned statAutoResetSec = 600 ;

  void
  usage( const char* argv0, std::ostream& out = cout )
  {
    out << "Usage: " << argv0 << " [options] server-host\n"
        << "  Available options:\n"
        << "    -h           - print help message and exit\n"
        << "    -i address   - interface to listen on [def: all]\n"
        << "    -p number    - listening port number [def: " << listen_port << "]\n"
        << "    -s number    - server port number [def: " << server_port << "]\n"
        << "    -z number    - max packet queue size [def: " << queue_size << "]\n"
        << "    -c number    - max cache size in MB [def: " << maxCacheSizeMB << "]\n"
        << "    -k number    - max packet number in cache [def: " << maxCachePackets << "]\n"
        << "    -t seconds   - max timeout value [def: " << timeout << "]\n"
        << "    -x           - collect data processing statistics\n"
        << "    -y number    - statistics sample size [def: " << statSize << "]\n"
        << "    -a seconds   - statistics auto-reset interval [def: " << statAutoResetSec << "]\n"
        << "    -d           - start as a daemon\n"
        << "    -l path      - log file name\n"
        << "    -m           - allocate thread for connection manager\n"
        << "    -v/q         - increase/decrease verbosity (can use many times)\n"
    ;
  }

  // condition used to wake sleeping threads
  boost::condition *_sleep_cond;

  // signal handler for terminating events
  void
  sig_handler_term ( int sigval )
  {
    if ( sigval == SIGINT || sigval == SIGTERM ) {
      // raise the condition
      if ( _sleep_cond ) _sleep_cond->notify_all() ;
    }
  }

  // close and open standard file descriptors
  int
  reOpenStd( const char* log )
  {
    int fdlog = -1 ;
    if ( log ) {
      // try to open log file first, if can't then return failure
      fdlog = open ( log, O_WRONLY | O_CREAT | O_TRUNC, 0666 ) ;
      if ( fdlog < 0 ) {
        cerr << "Failed to open the log file \"" << log << "\"" << endl ;
        return errno ;
      }
    }

    // close std files
    close ( 0 );
    if ( fdlog > 0 ) {
      close ( 1 );
      close ( 2 );
    }

    // open the files
    open ( "/dev/null", O_RDONLY ) ;   // stdin
    if ( fdlog > 0 ) {
      dup ( fdlog ) ;                  // stdout
      dup ( fdlog ) ;                  // stderr
    }

    if ( fdlog >= 0 ) close ( fdlog ) ;

    return 0 ;
  }

  // daemonize process
  int
  daemonize()
  {
    // first fork
    pid_t pid = ::fork() ;
    if ( pid < 0 ) {
      // fork failed
      return errno ;
    } else if ( pid > 0 ) {
      // parent exits
      exit ( 0 ) ;
    }

    // Become the leader of a new session
    pid = ::setsid();
    if ( pid < 0 ) {
      // setsid failed
      return errno ;
    }

    // second fork
    pid = ::fork() ;
    if ( pid < 0 ) {
      // fork failed
      return errno ;
    } else if ( pid > 0 ) {
      // parent exits
      exit ( 0 ) ;
    }

    // do not lock current directory
    chdir ( "/" ) ;

    return 0 ;
  }

}

int
main( int argc, char** argv )
{
  // all command options
  bool daemon = false ;
  unsigned int verbosity = 2 ;
  const char* iface = 0 ;
  const char* log = 0 ;
  bool useCmThread = false ;
  bool statCollect = false ;

  // parse the options
  int c ;
  while ( ( c = getopt ( argc, argv, "hi:p:s:z:c:k:t:dxy:a:l:mvq" ) ) != -1 ) {
    switch ( c ) {
    case 'h':
      usage ( argv[0], cout ) ;
      return 0 ;
    case 'i':
      iface = optarg ;
      break ;
    case 'p':
      listen_port = atoi ( optarg ) ;
      break ;
    case 's':
      server_port = atoi ( optarg ) ;
      break ;
    case 'z':
      queue_size = atoi ( optarg ) ;
      break ;
    case 'c':
      maxCacheSizeMB = atoi ( optarg ) ;
      break ;
    case 'k':
      maxCachePackets = atoi ( optarg ) ;
      break ;
    case 't':
      timeout = atoi ( optarg ) ;
      break ;
    case 'x':
      statCollect = true ;
      break ;
    case 'y':
      statSize = atoi ( optarg ) ;
      break ;
    case 'a':
      statAutoResetSec = atoi ( optarg ) ;
      break ;
    case 'd':
      daemon = true ;
      break ;
    case 'l':
      log = optarg ;
      break ;
    case 'v':
      verbosity ++ ;
      break ;
    case 'q':
      if ( verbosity > 0 ) verbosity -- ;
      break ;
    case 'm':
      useCmThread = true ;
      break ;
    case '?':
    default:
      usage ( argv[0], cerr ) ;
      return 1 ;
    }
  }

  // should have exactly one positional argument
  if ( argc - optind != 1 ) {
    cerr << "expect exactly one argument\n" ;
    usage ( argv[0], cerr ) ;
    return 1 ;
  }

  MsgLogger::setLevel ( verbosity ) ;

  // get server host name
  const char* server_host = argv[optind] ;

  // if hostname starts with "env:" then get it from the envvar that follows ':'
  if ( strncmp ( server_host, "env:", 4 ) == 0 ) {
    const char* envvar = server_host + 4 ;
    server_host = getenv ( envvar ) ;
    if ( ! server_host ) {
      cerr << "Environment variable \"" << envvar << "\" is not defined" ;
      return 1 ;
    }
  }

  // resolve the addresses
  NetEndpointAddress server_address ;
  NetEndpointAddress iface_address ( listen_port ) ;
  try {
    server_address = NetEndpointAddress ( server_host, server_port ) ;
    if ( iface ) iface_address = NetEndpointAddress ( iface, listen_port ) ;
  } catch ( const std::exception& e ) {
    cerr << e.what() << endl ;
    return 1 ;
  }

  // force log to /dev/null in case I'm daemon
  // and no log file was defined by user
  if ( daemon && ! log ) log = "/dev/null" ;

  // re-open standard file descriptors
  umask ( 022 ) ;
  int s = reOpenStd( log ) ;
  if ( s != 0 ) return s ;

  // daemonize if needed
  if ( daemon ) {
    int st = ::daemonize() ;
    if ( st != 0 ) {
      cerr << "failed to daemonize the process" << endl ;
      return st ;
    }
  }

  // instantiate cache instance
  PacketCacheMemory cache ( maxCacheSizeMB, maxCachePackets ) ;

  // statistics collection
  StatCollector* stat = 0 ;
  if ( statCollect ) {
    stat = new StatCollector ( statSize, statAutoResetSec ) ;
  }

  // instantiate connection manager
  TPCManager manager ( iface_address, server_address, queue_size, timeout, cache, stat ) ;

  if ( useCmThread ) {

    PXY_INFO ( "Spawning the connection manager thread" ) ;

    // this will spawn the new thread which will run forever
    boost::thread cmThread( manager ) ;

    // setup infinite waiting on a condition variable
    // until signal handler notifys us on this condition
    boost::mutex sleep_mutex ;
    boost::mutex::scoped_lock sleep_lock ( sleep_mutex ) ;
    boost::condition sleep_cond ;
    ::_sleep_cond = &sleep_cond ;

    PXY_INFO ( "Setting up signal handlers" ) ;
    signal ( SIGINT, ::sig_handler_term );
    signal ( SIGTERM, ::sig_handler_term );

    PXY_INFO ( "Waiting for interrupts" ) ;
    sleep_cond.wait ( sleep_lock ) ;
    PXY_INFO ( "Received interrupt, exiting" ) ;

  } else {

    PXY_INFO ( "Give control to the connection manager" ) ;
    // just run connection manager in the main thread
    manager() ;

  }

  return 0 ;
}
