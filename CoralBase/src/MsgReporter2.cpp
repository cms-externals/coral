// Include files
#include <cstdio>
#include <cstring> // fix bug #58581
#include <iostream> // fix bug #58581

// Local include files
#include "MsgReporter2.h"

/// Default constructor
coral::CoralBase::MsgReporter2::MsgReporter2()
  : m_level( coral::Error )
  , m_format( 0 )
  , m_mutex()
{
  // Use a non-default format?
  char* msgformat = getenv ( "CORAL_MSGFORMAT" );
  if( msgformat )
  {
    if ( strcmp( msgformat, "COOL" ) == 0 ||
         strcmp( msgformat, "cool" ) == 0 )
      m_format = 1;
    else if ( strcmp( msgformat, "ATLAS" ) == 0 ||
              strcmp( msgformat, "atlas" ) == 0 )
      m_format = 2;
  }
  if ( getenv ( "CORAL_MESSAGEREPORTER_FORMATTED" ) ) m_format = 1;

  // Use a non-default message level?
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

/// Access output level
coral::MsgLevel
coral::CoralBase::MsgReporter2::outputLevel() const
{
  return m_level;
}

/// Modify output level
void
coral::CoralBase::MsgReporter2::setOutputLevel(coral::MsgLevel lvl)
{
  m_level = lvl;
}

/// Report message to stdout
void
coral::CoralBase::MsgReporter2::report( int lvl,
                                        const std::string& src,
                                        const std::string& msg )
{
  if ( lvl < m_level ) return;
  boost::mutex::scoped_lock lock( m_mutex );

  if ( m_format == 1 ) // COOL format
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
  else if( m_format == 2 ) // ATLAS format
  {
    std::ostream& out = std::cout;

#ifdef __APPLE__
    time_t t01 = time(0);
    tm* l01 = localtime( &t01 ); //transform the time to human readable
    char b01[25];
    sprintf(b01, "%u-%02u-%02u %02u:%02u:%02u.000", (l01->tm_year + 1900), (l01->tm_mon + 1), l01->tm_mday, l01->tm_hour, l01->tm_min, l01->tm_sec);
#elif _WIN32
    time_t t01 = time(0);
    tm* l01 = localtime( &t01 ); //transform the time to human readable
    char b01[25];
    sprintf(b01, "%u-%02u-%02u %02u:%02u:%02u.000", (l01->tm_year + 1900), (l01->tm_mon + 1), l01->tm_mday, l01->tm_hour, l01->tm_min, l01->tm_sec);
#else
    timeval tv;
    gettimeofday (&tv, 0); //clock_gettime is only available on linux platform
    time_t t01 = time(0);
    tm* l01 = localtime( &t01 ); //transform the time to human readable
    char b02[7];
    sprintf(b02, "%6.6li", tv.tv_usec);
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

    out << "{" << src << "} " << msg << std::endl;

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
    std::cout << src << " " << level << " " << msg << std::endl;
  }
}
