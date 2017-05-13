#include <cstdio>
#include <cstring> // fix bug #58581
#include <iostream> // fix bug #58581
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/MessageStream.h"

// WARNING! THIS CLASS IS NO LONGER SUPPORTED AND WILL BE REMOVED!
// [See https://savannah.cern.ch/bugs/index.php?53040]
// WARNING! THIS FILE CAN NO LONGER BE CHANGED IN CVS!
// [See https://savannah.cern.ch/bugs/index.php?59445]

/// Default constructor
coral::MsgReporter::MsgReporter()
  : m_level( coral::Error )
{
  static bool first = true;
  if ( first )
  {
    first = false;
    if ( getenv( "CORAL_MSGLEVEL" ) )
    {
      // Check only the first char of the environment variable
      switch ( *getenv( "CORAL_MSGLEVEL" ) )
      {
      case '0':
      case 'n':
      case 'N': m_level = coral::Nil; break;

      case '1':
      case 'v':
      case 'V': m_level = coral::Verbose; break;

      case '2':
      case 'd':
      case 'D': m_level = coral::Debug; break;

      case '3':
      case 'i':
      case 'I': m_level = coral::Info; break;

      case '4':
      case 'w':
      case 'W': m_level = coral::Warning; break;

      case '5':
      case 'e':
      case 'E': m_level = coral::Error; break;

      case '6':
      case 'f':
      case 'F': m_level = coral::Fatal; break;

      case '7':
      case 'a':
      case 'A': m_level = coral::Always; break;

      default: break; // keep the default
      }
    }
  }
}

/// Access output level
coral::MsgLevel
coral::MsgReporter::outputLevel() const
{
  return m_level;
}

/// Modify output level
void
coral::MsgReporter::setOutputLevel(coral::MsgLevel lvl)
{
  m_level = lvl;
}

/// report mesage to stdout
void
coral::MsgReporter::report(int lvl, const std::string& src, const std::string& msg)
{
  if ( lvl < m_level ) return;
  static boost::mutex mutex;
  boost::mutex::scoped_lock lock(mutex);

  static const char* newFormat = getenv ( "CORAL_MESSAGEREPORTER_FORMATTED" );

  size_t format = 0;

  char* msgformat = getenv ( "CORAL_MSGFORMAT" );
  if(msgformat)
  {
    if(strcmp( msgformat, "COOL" ) == 0 || strcmp( msgformat, "cool" ) == 0)
      format = 1;
    else if(strcmp( msgformat, "ATLAS" ) == 0 || strcmp( msgformat, "atlas" ) == 0)
      format = 2;
  }

  if ( newFormat || format == 1 )
  {
    // Formatted CORAL reporter (as in COOL)
    std::ostream& out = std::cout;
    const std::string::size_type src_name_maxsize = 36;
    if ( src.size() <= src_name_maxsize )
    {
      out << src << std::string( src_name_maxsize-src.size(), ' ' );
    }
    else
    {
      out << src.substr( 0, src_name_maxsize-3 ) << "...";
    }
    switch ( lvl )
    {
    case 0:  out << " Nil      "; break;
    case 1:  out << " Verbose  "; break;
    case 2:  out << " Debug    "; break;
    case 3:  out << " Info     "; break;
    case 4:  out << " Warning  "; break;
    case 5:  out << " Error    "; break;
    case 6:  out << " Fatal    "; break;
    case 7:  out << " Always   "; break;
    default: out << " Unknown  "; break;
    }
    out << msg << std::endl;
  }
  else if( format == 2 /*atlas format*/ )
  {
    std::ostream& out = std::cout;
    //clock_gettime is only available on linux platform
#ifdef __APPLE__

    time_t t01 = time(0);
    //transform the time to human readable
    tm* l01 = localtime( &t01 );

    char b01[25];

    sprintf(b01, "%u-%02u-%02u %02u:%02u:%02u.000", (l01->tm_year + 1900), (l01->tm_mon + 1), l01->tm_mday, l01->tm_hour, l01->tm_min, l01->tm_sec);

#elif _WIN32

    time_t t01 = time(0);
    //transform the time to human readable
    tm* l01 = localtime( &t01 );

    char b01[25];

    sprintf(b01, "%u-%02u-%02u %02u:%02u:%02u.000", (l01->tm_year + 1900), (l01->tm_mon + 1), l01->tm_mday, l01->tm_hour, l01->tm_min, l01->tm_sec);

#else
    timespec tspec;

    clock_gettime(CLOCK_MONOTONIC, &tspec);

    time_t t01 = time(0);
    //transform the time to human readable
    tm* l01 = localtime( &t01 );

    char b02[10];

    sprintf(b02, "%li", tspec.tv_nsec);

    char b01[25];

    sprintf(b01, "%i-%02i-%02i %02i:%02i:%02i.%.3s", (l01->tm_year + 1900), (l01->tm_mon + 1), l01->tm_mday, l01->tm_hour, l01->tm_min, l01->tm_sec, b02);
#endif

    out << b01;

    switch ( lvl )
    {
    case 0:  out << " [   ] "; break;
    case 1:  out << " [VRB] "; break;
    case 2:  out << " [CDB] "; break;
    case 3:  out << " [INF] "; break;
    case 4:  out << " [WRN] "; break;
    case 5:  out << " [ERR] "; break;
    case 6:  out << " [!!!] "; break;
    case 7:  out << " [ALL] "; break;
    default: out << " [???] "; break;
    }

    out << msg << std::endl;

  }
  else
  {
    // Default CORAL reporter
    const char* level = "Unknown";
    switch ( lvl )
    {
#define SET(x)  case coral::x:  level=# x;  break
      SET( Nil );
      SET( Verbose );
      SET( Debug );
      SET( Info );
      SET( Warning );
      SET( Error );
      SET( Fatal );
    case coral::Always:
      level="Success";
      break;
    default:
      level="Unknown";
      break;
#undef SET
    }
    std::cout << "HALLO " << src << " " << level << " " << msg << std::endl;
  }
}
