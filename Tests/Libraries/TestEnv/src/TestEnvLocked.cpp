#include "TestEnv/TestEnvLocked.h"

#ifndef _WIN32

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#endif

#include "CoralBase/MessageStream.h"

#define DEBUG( msg ){ coral::MessageStream myMsg("TestEnvLocked"); myMsg << coral::Debug << msg << coral::MessageStream::endmsg; }

namespace {

  int
  tryToLockFile( const std::string& filename )
  {
#ifndef _WIN32
    // open and lock the file
    DEBUG( "Open file" );
    int lfd = open( filename.c_str(), O_WRONLY|O_CREAT, 0600 );
    DEBUG( "Try to lock file" );
    int res = flock( lfd, LOCK_EX );
    if( res == -1 ) return -1;
    return lfd;
#else
    return -1;
#endif
  }

  void
  tryToUnlockFile( int lfd )
  {
#ifndef _WIN32
    if( lfd != -1 )
    {
      DEBUG( "Unlock and close file" );
      flock( lfd, LOCK_UN );
      close( lfd );
    }
#endif
  }

}

//-----------------------------------------------------------------------------

TestEnvLocked::TestEnvLocked(std::string testName)
  : TestEnv( testName )
  , m_lockfile( "/tmp/" ) // directory for the file
  , m_lfd( -1 )
{
  // try to lock a file as long the test is active
}

//-----------------------------------------------------------------------------

TestEnvLocked::~TestEnvLocked()
{
  // unlock the file
  tryToUnlockFile( m_lfd );
}

//-----------------------------------------------------------------------------

void
TestEnvLocked::setupUserLock()
{
  // avoid locking two times
  if( m_lfd != -1 ) return;
  // construct the filename
  m_lockfile.append( BuildUniqueTableName( m_testName + "_tempfile" ) );
  // try to lock the file
  m_lfd = tryToLockFile( m_lockfile );
}

//-----------------------------------------------------------------------------
