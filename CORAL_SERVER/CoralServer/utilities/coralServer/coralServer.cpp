// $Id: coralServer.cpp,v 1.1.2.5 2011/09/10 16:51:18 avalassi Exp $

// Include files
#include <cerrno>
#include <csignal>
#include <cstdlib> // For setenv and _putenv
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "CoralKernel/Context.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/../src/MsgReporter2.h"
#include "CoralMonitor/StatsStorage.h"
#include "CoralMonitor/StatsSimpleBuffer.h"
#include "CoralMonitor/StatsTypeCPUsage.h"
#include "CoralMonitor/StatsTypeMEMUsage.h"
#include "CoralMonitor/StatsCSVPlotter.h"
#include "CoralSockets/SocketServer.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#define LOGGER_NAME CoralServer::logger // dummy (avoid compilation #error)
#include "CoralServerBase/logger.h"

// Local include files
#include "CoralServerFacadeFactory.h"
#include "ServerStubFactory.h"
//#include "ThreadMonitorReporter.h"

// Force the instantiation of global statistics singletons
static coral::StatsTypeCPUsage myCPUsage;
static coral::StatsTypeMEMUsage myMEMUsage;

// Message output
#define SERVER_LOG( msg ){ MessageStream myMsg("CoralServer"); myMsg << Always << msg << coral::MessageStream::endmsg; }
#define SERVER_WARNING( msg ){ MessageStream myMsg("CoralServer"); myMsg << Always << msg << coral::MessageStream::endmsg; }
//#define SERVER_LOG( msg ){ MessageStream myMsg("CoralServer"); myMsg << Info << msg << coral::MessageStream::endmsg; }
//#define SERVER_WARNING( msg ){ MessageStream myMsg("CoralServer"); myMsg << Warning << msg << coral::MessageStream::endmsg; }

// Namespace
using namespace coral;

//-----------------------------------------------------------------------------

// Global variables
int listen_port = 40007; //default port
int timeout = -1;
int threadsPerSocket = 5;
int monitorPeriod = 10;
const char* lrotate = 0;
const char* monitorPath = 0;
const char* cauth = getenv( "CORAL_AUTH_PATH" );
const char* cdblookup = getenv( "CORAL_DBLOOKUP_PATH" );
const char* outputformat = getenv( "CORAL_MSGFORMAT" );
const char* verboselevel = getenv( "CORAL_MSGLEVEL" );

//-----------------------------------------------------------------------------

void
usage( const char* argv0, std::ostream& out = std::cout )
{
  out << "Found current environment variables:\n";

  if( cauth )
    out << "  CORAL_AUTH_PATH=" << cauth << "\n";
  if( cdblookup )
    out << "  CORAL_DBLOOKUP_PATH=" << cdblookup << "\n";
  if( verboselevel )
    out << "  CORAL_MSGLEVEL=" << verboselevel << "\n";
  if( outputformat )
    out << "  CORAL_MSGFORMAT=" << outputformat << "\n";

  char* costro = getenv( "CORAL_ORA_SKIP_TRANS_READONLY" );
  if( costro )
    out << "  CORAL_ORA_SKIP_TRANS_READONLY=" << costro << "\n";

  out << "\nUsage: " << argv0 << " [options]\n"
      << "  Available options:\n"
      << "    -h           - print help message and exit\n"
      << "    -i address   - interface to listen on [def: all]\n"
      << "    -p number    - listening port number [def: " << listen_port << "]\n"
      << "    -t number    - server timeout in seconds {-1: infinity} [def: " << timeout << "] \n"
      << "    -m number    - monitor period in seconds [def: " << monitorPeriod << "]\n"
      << "    -e path      - monitor file for export csv [def: no monitoring]\n"
      << "    -r number    - log rotate {0 = rename once, n = rotate the log after n seconds} [def: deactivate]\n"
      << "    -a text      - auth service [def: CORAL/Services/XMLAuthenticationService]\n"
      << "    -l text      - lookup service [def: CORAL/Services/XMLLookupService]\n"
      << "    -A path      - set the CORAL_AUTH_PATH\n"
      << "    -L path      - set the CORAL_DBLOOKUP_PATH\n"
      << "    -o format    - set the CORAL_MSGFORMAT {ATLAS,COOL,CORAL} [def: ATLAS]\n"
      << "    -y text      - read-only transactions {ON,OFF} [def: ON]\n"
      << "    -n number    - threads per socket [def: " << threadsPerSocket << "]\n"
      << "    -d logfile   - start as a daemon with the given log file\n"
      << "    -v/q         - increase/decrease verbosity (can use many times)\n"
    ;
}

//-----------------------------------------------------------------------------

StatsSimpleBuffer*
startPlotter()
{
  if( getenv( "CORALSERVER_SPECIAL_OPTION01" ) )
  {
    //start the monitor thread
    SERVER_LOG( "Start the Monitor thread" );
    StatsSimpleBuffer* sbuffer = new StatsSimpleBuffer(2, 200);

    StatsSimpleBufferPlot& plot00 = sbuffer->addPlot("CPU usage");
    plot00.stats.push_back("CPU[system] usage in %");
    plot00.stats.push_back("CPU[user] usage in %");
    plot00.stats.push_back("CPU[nice] usage in %");
    plot00.stats.push_back("CPU[total] usage in %");

    StatsSimpleBufferPlot& plot0a = sbuffer->addPlot("Memory usage");
    plot0a.stats.push_back("Memory total");
    plot0a.stats.push_back("Memory resident set size");
    plot0a.stats.push_back("Memory shared pages");
    plot0a.stats.push_back("Memory text (code)");
    plot0a.stats.push_back("Memory lib");
    plot0a.stats.push_back("Memory data");

    StatsSimpleBufferPlot& plot01 = sbuffer->addPlot("bandwidth");
    plot01.stats.push_back("bandwidth in in bytes/s");
    plot01.stats.push_back("bandwidth out in bytes/s");

    StatsSimpleBufferPlot& plot02 = sbuffer->addPlot("packets");
    plot02.stats.push_back("packets in in amount/s");
    plot02.stats.push_back("packets out in amount/s");

    StatsSimpleBufferPlot& plot03 = sbuffer->addPlot("packets in data");
    plot03.stats.push_back("packets in header data in bytes/s");
    plot03.stats.push_back("packets in payload data in bytes/s");

    StatsSimpleBufferPlot& plot04 = sbuffer->addPlot("packets out data");
    plot04.stats.push_back("packets out header data in bytes/s");
    plot04.stats.push_back("packets out payload data in bytes/s");

    StatsSimpleBufferPlot& plot05 = sbuffer->addPlot("threads");
    plot05.stats.push_back("current idle threads");
    plot05.stats.push_back("current threads active");

    StatsSimpleBufferPlot& plot06 = sbuffer->addPlot("connections");
    plot06.stats.push_back("connection counter");

    StatsSimpleBufferPlot& plot07 = sbuffer->addPlot("compute checksum");
    plot07.stats.push_back("compute checksum current user time");
    plot07.stats.push_back("compute checksum current system time");
    plot07.stats.push_back("compute checksum current real time");

    StatsSimpleBufferPlot& plot08 = sbuffer->addPlot("reply time");
    plot08.stats.push_back("reply current user time");
    plot08.stats.push_back("reply current system time");
    plot08.stats.push_back("reply current real time");

    StatsSimpleBufferPlot& plot09 = sbuffer->addPlot("ConnectRO");
    plot09.stats.push_back("ConnectRO current user time");
    plot09.stats.push_back("ConnectRO current system time");
    plot09.stats.push_back("ConnectRO current real time");

    StatsSimpleBufferPlot& plot10 = sbuffer->addPlot("FetchAllRows");
    plot10.stats.push_back("FetchAllRows current user time");
    plot10.stats.push_back("FetchAllRows current system time");
    plot10.stats.push_back("FetchAllRows current real time");

    StatsSimpleBufferPlot& plot11 = sbuffer->addPlot("ListTables");
    plot11.stats.push_back("ListTables current user time");
    plot11.stats.push_back("ListTables current system time");
    plot11.stats.push_back("ListTables current real time");

    StatsSimpleBufferPlot& plot12 = sbuffer->addPlot("FetchTableDescription");
    plot12.stats.push_back("FetchTableDescription current user time");
    plot12.stats.push_back("FetchTableDescription current system time");
    plot12.stats.push_back("FetchTableDescription current real time");

    //start the plotter in an extra thread
    //plots with gnuplot each 10 seconds
    sbuffer->startDatPlotter("monitor.dat", "/var/www/lighttpd/", 10);

    return sbuffer;
  }
  return 0;
}

//-----------------------------------------------------------------------------

unsigned int
transformVerbosity(char c, unsigned int def)
{
  //transform the verboselevel from env to a number
  //customized copy from CoralBase
  switch ( c )
  {
  case '0':
  case 'n':
  case 'N': return 0; //nil

  case '1':
  case 'v':
  case 'V': return 6; //verbose

  case '2':
  case 'd':
  case 'D': return 5; //debug

  case '3':
  case 'i':
  case 'I': return 4; //info

  case '4':
  case 'w':
  case 'W': return 3; //warning

  case '5':
  case 'e':
  case 'E': return 2; //error

  case '6':
  case 'f':
  case 'F': return 1; //fatal

  case '7':
  case 'a':
  case 'A': return 7; //allways
  }
  return def;
}

//-----------------------------------------------------------------------------

CoralSockets::SocketServer* pServer;

// See http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=125
void signalhandler( int sig )
{
  if ( sig == SIGTERM )
  {
    SERVER_WARNING( "Signal SIGTERM (#" << sig << ") caught" );
  }
  else if ( sig == SIGINT )
  {
    SERVER_WARNING( "Signal SIGINT (#" << sig << ") caught" );
  }
  else
  {
    SERVER_WARNING( "PANIC! Unknown signal #" << sig
                    << " caught: this should not happen!" );
  }
  if ( pServer )
  {
    SERVER_WARNING( "Stop the server" );
    pServer->stopServer();
  }
  else
  {
    SERVER_WARNING( "The server is not running - do nothing" );
  }
}

//-----------------------------------------------------------------------------

// Close and open standard file descriptors
// [copied from Andy's coralServerProxy.cpp]
int
reOpenStd( const char* log )
{
  int fdlog = -1;
  if ( log )
  {
    // try to open log file first, if can't then return failure
    fdlog = open ( log, O_WRONLY | O_CREAT | O_TRUNC, 0666 );
    if ( fdlog < 0 )
    {
      std::cerr << "Failed to open the log file '" << log << "'" << std::endl;
      return errno;
    }
  }

  // close std files
  close ( 0 );
  if ( fdlog > 0 )
  {
    close ( 1 );
    close ( 2 );
  }

  // open the files
  open ( "/dev/null", O_RDONLY ); // stdin
  if ( fdlog > 0 )
  {
    dup ( fdlog ); // stdout
    dup ( fdlog ); // stderr
  }
  if ( fdlog >= 0 ) close ( fdlog );
  return 0;
}

//-----------------------------------------------------------------------------

// Daemonize process
// [copied from Andy's coralServerProxy.cpp]
// [see also http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html]
int
daemonize()
{
  // first fork
  pid_t pid = ::fork();
  if ( pid < 0 )
  {
    // fork failed
    return errno;
  }
  else if ( pid > 0 )
  {
    // parent exits
    exit ( 0 );
  }

  // Become the leader of a new session
  pid = ::setsid();
  if ( pid < 0 )
  {
    // setsid failed
    return errno;
  }

  // second fork
  pid = ::fork();
  if ( pid < 0 )
  {
    // fork failed
    return errno;
  }
  else if ( pid > 0 )
  {
    // parent exits
    exit ( 0 );
  }

  // do not lock current directory
  chdir ( "/" );

  // print the PID
  std::cout << "Server will run as a daemon: PID=" << ::getpid() << std::endl;

  return 0;
}

//-----------------------------------------------------------------------------

/** @file coralServer.cpp
 *
 *  @author Andrea Valassi & Alexander Kalkhof
 *  @date   2009-09-30
 */

int
main( int argc, char** argv )
{
  signal( SIGTERM, signalhandler ); // register a SIGTERM handler
  signal( SIGINT, signalhandler ); // register a SIGINT handler (task #13097)
  unsigned int verbosity = 2;
  bool verbchanged = false;

  bool authchanged = false;
  bool lookupchanged = false;

  const char* iface = "0.0.0.0";
  const char* authsvc = 0;
  const char* lookupsvc = 0;
  const char* orastro = 0;
  const char* daemonlog = 0;
  //check if we have a verboselevel set
  //parse the options
  char c;
  while ( ( c = getopt ( argc, argv, "hi:p:t:m:e:r:a:l:A:L:o:y:n:d:vq" ) ) != -1 )
  {
    switch ( c )
    {
    case 'h':
      usage( argv[0], std::cout );
      return 0;
    case 'i':
      iface = optarg;
      break;
    case 't':
      timeout = atoi( optarg );
      break;
    case 'm':
      monitorPeriod = atoi( optarg );
      break;
    case 'e':
      monitorPath = optarg;
      break;
    case 'r':
      lrotate = optarg;
      break;
    case 'p':
      listen_port = atoi( optarg );
      break;
    case 'a':
      authsvc = optarg;
      break;
    case 'l':
      lookupsvc = optarg;
      break;
    case 'A':
      cauth = optarg;
      authchanged = true;
      break;
    case 'L':
      cdblookup = optarg;
      lookupchanged = true;
      break;
    case 'o':
      outputformat = optarg;
      break;
    case 'y':
      orastro = optarg;
      break;
    case 'n':
      threadsPerSocket = atoi( optarg );
      break;
    case 'd':
      daemonlog = optarg;
      break;
    case 'v':
      verbosity++;
      verbchanged = true;
      break;
    case 'q':
      if( verbosity > 0) verbosity--;
      verbchanged = true;
      break;
    default:
      usage( argv[0], std::cerr );
      return 1;
    }
  }

  // should have exactly zero positional arguments
  if ( argc - optind != 0 )
  {
    std::cerr << "Expect exactly zero arguments\n";
    usage ( argv[0], std::cerr ) ;
    return 1 ;
  }

  // Daemonize and write to the given log
  // [Andy writes to /dev/null in daemon mode if no log is defined]
  if ( daemonlog )
  {
    std::cout << "Run as a daemon with output on " << daemonlog << std::endl;
    // Re-open standard file descriptors in daemon mode
    // [Andy does it even if not in daemon mode]
    umask ( 022 );
    int s = reOpenStd( daemonlog );
    if ( s != 0 ) return s;
    // Daemonize if needed
    // [copied from Andy's coralServerProxy.cpp]
    s = ::daemonize();
    if ( s != 0 )
    {
      std::cerr << "Failed to daemonize the process" << std::endl;
      return s;
    }
  }

  try
  {
    if( !outputformat )
    {
      outputformat = "ATLAS";
    }
    setenv( "CORAL_MSGFORMAT", outputformat, 1 );
    //create a new message reporter
    coral::IMsgReporter* reporter = new coral::CoralBase::MsgReporter2;
    //install and replace the old message reporter
    coral::MessageStream::installMsgReporter( reporter );
    //set the verbosity level
    if( verboselevel && !verbchanged )
    {
      //transform verbosity level from the string input
      verbosity = transformVerbosity( *verboselevel, verbosity );
    }
    switch(verbosity)
    {
    case 0:
      coral::MessageStream::setMsgVerbosity( coral::Always );
      break;
    case 1:
      coral::MessageStream::setMsgVerbosity( coral::Fatal );
      break;
    case 2:
      coral::MessageStream::setMsgVerbosity( coral::Error );
      break;
    case 3:
      coral::MessageStream::setMsgVerbosity( coral::Warning );
      break;
    case 4:
      coral::MessageStream::setMsgVerbosity( coral::Info );
      break;
    case 5:
      coral::MessageStream::setMsgVerbosity( coral::Debug );
      break;
    case 6:
      coral::MessageStream::setMsgVerbosity( coral::Verbose );
      break;
    default:
      coral::MessageStream::setMsgVerbosity( coral::Nil );
      break;
    }
    unsetenv( "CORAL_MSGLEVEL" );

    SERVER_LOG( "Starting CoralServer built on " << __DATE__ );
    SERVER_LOG( "Using CORAL_MSGFORMAT = '" << getenv( "CORAL_MSGFORMAT" ) << "'" );
    SERVER_LOG( "Using CORAL_MSGLEVEL = '" << verbosity <<  "'" );

    //configure all environment variables
    if ( cauth )
    {
      if( authchanged )
      {
        setenv( "CORAL_AUTH_PATH", cauth, 1 );
        SERVER_WARNING( "env variable CORAL_AUTH_PATH was overriden" );
      }
      SERVER_LOG( "Using CORAL_AUTH_PATH = '" << cauth <<  "'" );
    }
    else
    {
      SERVER_LOG( "ERROR! Env CORAL_AUTH_PATH is not defined" );
      return 1;
    }
    if( cdblookup )
    {
      if( lookupchanged )
      {
        setenv( "CORAL_DBLOOKUP_PATH", cdblookup, 1 );
        SERVER_WARNING( "env variable CORAL_DBLOOKUP_PATH was overriden" );
      }
      SERVER_LOG( "Using CORAL_DBLOOKUP_PATH = '" << cdblookup <<  "'" );
    }
    else
    {
      SERVER_LOG( "ERROR! Env CORAL_DBLOOKUP_PATH is not defined" );
      return 1;
    }
    if( orastro )
    {
      if( ( strcmp( orastro , "ON" ) == 0 ) || ( strcmp( orastro , "on" ) == 0 ) )
      {
        unsetenv( "CORAL_ORA_SKIP_TRANS_READONLY" );
      }
      else if( ( strcmp( orastro , "OFF" ) == 0 ) || ( strcmp( orastro , "off" ) == 0 ) )
      {
        setenv( "CORAL_ORA_SKIP_TRANS_READONLY", "yes", 1 );
        SERVER_LOG( "Using CORAL_ORA_SKIP_TRANS_READONLY" );
      }
    }
    else
    {
      if( getenv("CORAL_ORA_SKIP_TRANS_READONLY") )
      {
        SERVER_LOG( "Using CORAL_ORA_SKIP_TRANS_READONLY" );
      }
      else
      {
        SERVER_LOG( "CORAL_ORA_SKIP_TRANS_READONLY was not set" );
      }
    }
    // Load ConnectionService2
    std::string connSvc2Name = "CORAL/Services/ConnectionService2";
    //SERVER_LOG( "Load " << connSvc2Name );
    Context::instance().loadComponent( connSvc2Name );
    IHandle<IConnectionService> connSvc2 = Context::instance().query<IConnectionService>( connSvc2Name );
    if ( !connSvc2.isValid() )
    {
      SERVER_LOG( "ERROR! Could not load " << connSvc2Name );
      return 1;
    }
    // Configure ConnectionService2
    //SERVER_LOG( "Configure " << connSvc2Name );
    connSvc2->configuration().disablePoolAutomaticCleanUp();
    connSvc2->configuration().setConnectionTimeOut( 0 );
    if ( authsvc )
    {
      connSvc2->configuration().setAuthenticationService( authsvc );
    }
    if ( lookupsvc )
    {
      connSvc2->configuration().setLookupService( lookupsvc );
    }
    // Start the coralServer
    {
      //SERVER_LOG( "Create a CoralServerFacadeFactory" );
      CoralServer::CoralServerFacadeFactory facadeFactory;
      //SERVER_LOG( "Create a ServerStubFactory" );
      CoralServer::ServerStubFactory handlerFactory( facadeFactory );
      SERVER_LOG( "Create the server on " << iface << ":" << listen_port );
      pServer = new CoralSockets::SocketServer( handlerFactory, iface, listen_port, threadsPerSocket );

      StatsCSVPlotter* scsvplotter = 0;
      if( (monitorPeriod > 0) && monitorPath )
        //activate monitoring using an export file
      {
        if(lrotate)
        {
          scsvplotter = new StatsCSVPlotter(monitorPeriod, monitorPath, atoi(lrotate));
          SERVER_LOG( "Activate Monitoring ( period: " << monitorPeriod
                      << "s, log rotate: "
                      << lrotate << "s, filename: '"
                      << monitorPath << "' )" );
        }
        else
        {
          scsvplotter = new StatsCSVPlotter(monitorPeriod, monitorPath, -1);
          SERVER_LOG( "Activate Monitoring ( period: " << monitorPeriod
                      << "s, log rotate: deactivated, filename: '"
                      << monitorPath << "' )" );
        }
        //needed for monitoring
        setenv( "CORALSERVER_TIMING", "yes", 1 );
      }
      else
      {
        unsetenv( "CORALSERVER_TIMING" );
      }
      StatsSimpleBuffer* sbuffer = startPlotter();

      // *** Start the server loop
      SERVER_LOG( "Start the server (timeout = " << timeout << "s)" );
      pServer->run( timeout );

      //---------------------------
      // ... SERVER LOOP ...
      //---------------------------

      // *** End of server loop
      SERVER_LOG( "The server has been stopped" );

      if(scsvplotter)
      {
        SERVER_LOG( "Delete the StatsCSVPlotter" );
        delete scsvplotter;
      }

      if(sbuffer)
      {
        SERVER_LOG( "Delete the StatsSimpleBuffer" );
        delete sbuffer;
      }

      SERVER_LOG( "Delete the server" );
      if(pServer)
      {
        CoralSockets::SocketServer* pServer2 = pServer;
        pServer = 0; // Make sure SIGTERM has no effect while deleting it
        delete pServer2;
      }
    }
  }

  catch( std::exception& e )
  {
    SERVER_LOG( "ERROR! Standard C++ exception: '" << e.what() << "'" );
    return 1;
  }

  catch( ... )
  {
    SERVER_LOG( "ERROR! Unknown exception caught" );
    return 1;
  }

  // Successful program termination
  SERVER_LOG( "Exiting main" );
  return 0;
}

//-----------------------------------------------------------------------------
