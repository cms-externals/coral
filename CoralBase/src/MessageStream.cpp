// $Id: MessageStream.cpp,v 1.22 2011/03/22 11:31:07 avalassi Exp $
//====================================================================
//  Implementation file of the MessageStream object
//--------------------------------------------------------------------
//
//  Package: coral/CoralBase (The CORAL project)
//
//  Description: Generic data persistency for LHC
//
//  @author      M.Frank
//====================================================================
#include <cstring> // fix bug #58581
#include <iostream>
#include <memory>
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/MessageStream.h"
#include "MsgReporter2.h"

// Preset default message reporter to None
static std::unique_ptr<coral::IMsgReporter> s_msgReporter( new coral::CoralBase::MsgReporter2 );

/// Install new message reporter object
void coral::MessageStream::installMsgReporter( IMsgReporter* reporter )
{
  if ( reporter )
  {
    if ( s_msgReporter.get() )
    {
      s_msgReporter->release();
      s_msgReporter.release();
    }
    s_msgReporter = std::unique_ptr<coral::IMsgReporter>(reporter);
    return;
  }

  s_msgReporter = std::unique_ptr<coral::IMsgReporter>(new coral::MsgReporter);
}

/// Set the verbosity of the default message reporter.
void coral::MessageStream::setMsgVerbosity( coral::MsgLevel lvl )
{
  if ( s_msgReporter.get() ) s_msgReporter->setOutputLevel( lvl );
}

/// Get the verbosity of the default message reporter.
coral::MsgLevel coral::MessageStream::msgVerbosity()
{
  return s_msgReporter->outputLevel();
}

/// Standard initializing constructor
coral::MessageStream::MessageStream( const std::string& source )
  : m_act( false ), m_source( source ), m_currLevel( Nil )
{
  m_level = coral::Verbose; // Accept all messages (fix for bug #36831)
}

/// Propagate message to reporter
void coral::MessageStream::doOutput()
{
  s_msgReporter->report( m_currLevel, m_source, m_stream.str() );
  // This is the valid mechanism to clear a string stream (fix bug #56900)
  m_stream.str( "" );
}

/// Print data of type long long int
void coral::MessageStream::prtLongLong( const long long int arg )
{
#ifdef _WIN32
  char buf[128];
  ::sprintf( buf, m_stream.flags()&std::ios::hex ? "%I64x" : "%I64d", arg );
  m_stream << buf;
#else
  m_stream << arg;
#endif
}

/// Print data of type unsigned long long int
void coral::MessageStream::prtLongLong( const unsigned long long int arg )
{
#ifdef _WIN32
  char buf[128];
  ::sprintf( buf, m_stream.flags()&std::ios::hex ? "%I64x" : "%I64d", arg );
  m_stream << buf;
#else
  m_stream << arg;
#endif
}
